#include "Server.h"
#include <string>
#include "Tracer.h"
#include <string>
#include "RaftUtils.h"




#define SEMAPHORE_SERVER_DISPATCH		1
#define SEMAPHORE_SERVER_NEW_STATE		2	

Server::Server(int server_id)
{

	// Persisten state on all servers. 
	current_term_			= 0;							// Latest term server has seen (initialized to 0 on first boot, increases monotonically)
	voted_for_				= NONE;							// CandidateId that received vote in current term(or null if none)
	
	memset(&log_, 0, sizeof(log_));							// Log entries; each entry contains command for state machine, and term when entry was received by leader(first index is 1)
	log_.command_[0].set_state_machime_command(NONE);
	log_.command_[0].set_term_when_entry_was_received_by_leader(NONE);
	log_.log_index_			= 0;							// First index is 1, but when log writes increments + 1(So, first index is 1) 

	current_leader_id_		= NONE;							// Current leader's id 

	// Volatile state on all servers. 
	commit_index_			= 0;							// Index of highest log entry known to be committed(initialized to 0, increases	monotonically)
	last_applied_			= 0	;							// Index of highest log entry applied to state	machine(initialized to 0, increases	monotonically)	
	current_state_			= StateEnum::unknown_state;		// Starts as unknown. 
	new_state_				= StateEnum::unknown_state;		// Starts as unknown. 
	
	server_id_				= server_id;
	have_to_die_			= false;
	file_log_name_			= ".\\..\\Debug\\" + std::string(SERVER_TEXT) + std::to_string(server_id_) + std::string(".txt");

		
	Tracer::trace("Started server ID:" + std::to_string(server_id_) + "\r\n");
}

Server::~Server()
{
	have_to_die_ = true;
	semaphore_dispatch_.notify(SEMAPHORE_SERVER_DISPATCH);
	semaphore_dispatch_.notify(SEMAPHORE_SERVER_NEW_STATE);

	if (thread_dispatch_msg_socket_.joinable()) {
		thread_dispatch_msg_socket_.join();
	}

	if (thread_check_new_state_.joinable()) {
		thread_check_new_state_.join();
	}
}

void Server::send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	communication_.sendMessage(client_request, port, sender, action, receiver);
}

void Server::start() 
{		
	int ret = manager_log_.read_log(file_log_name_, &log_, sizeof(log_));
	if (ret != MANAGER_NO_ERROR) {
		Tracer::trace("Server::start - FAILED!!! to read " + file_log_name_ +" log, stopping server...\r\n");
		return;
	}
	
	// Update commit_index_
	set_commit_index(get_log_index());


	// TEST.
	//std::this_thread::sleep_for(std::chrono::milliseconds(30000));

	// Start dispatch msg socket
	thread_dispatch_msg_socket_ = std::thread(&Server::dispatch_msg_socket, this);	
	// Start receive msg socket
	thread_receive_msg_socket_ = std::thread(&Server::receive_msg_socket, this);


	current_state_ = StateEnum::follower_state;
	connector_ = get_current_shape_sever(current_state_);
	if (connector_) {
		connector_->start();
		rpc_api_server_.start(this, RPC_BASE_PORT + RPC_RECEIVER_PORT + get_server_id());
	}

	// Start server check new state
	thread_check_new_state_ = std::thread(&Server::check_new_state, this);
	

	receive_rpc();	
}


void Server::dispatch_msg_socket()
{
	while(!have_to_die_)
	{
		semaphore_dispatch_.wait(SEMAPHORE_SERVER_DISPATCH);		
		// Get (FIFO)
		ClientRequest client_request = queue_.front();
		// Delete rcp from queue. 
		queue_.pop();
		{
			std::lock_guard<std::mutex> locker(mu_server_);

			if (connector_ != nullptr) {	
				connector_->receive_msg_socket(&client_request);
			}
		}
	}
}

void Server::receive_msg_socket()
{
	while (!have_to_die_) {
		ClientRequest client_request;
		int error = communication_.receiveMessage(&client_request, SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + server_id_, std::string(SERVER_TEXT) + std::string(".") + std::to_string(get_server_id()));

		if (error) {
			Tracer::trace("Follower::receive - FAILED!!!  - error" + std::to_string(error) + "\r\n");
		}
		else {			
			queue_.push(client_request);
			semaphore_dispatch_.notify(SEMAPHORE_SERVER_DISPATCH);
		}
	}	
}

void Server::receive_rpc()
{	
	while (!have_to_die_)
	{
		int error = rpc_api_server_.receive();
		if (error != NONE) {
			Tracer::trace("RPC_API_Server::receive_rpc error:" + std::to_string(error) + "\r\n");
			if (!have_to_die_) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		}
	}
}

IRole* Server::get_current_shape_sever(StateEnum state)
{
	// Before creating a new role, we have to delete previous one. 
	if (connector_ != nullptr)
		delete(connector_);

	IRole* role; 
	if (state == StateEnum::follower_state) {
		role = new Follower(this);		
		Tracer::trace("Created Follower." + std::to_string(get_server_id()) + "\r\n");
	}
	else if (state == StateEnum::candidate_state) {
		role = new Candidate(this);
		Tracer::trace("Created Candidate." + std::to_string(get_server_id()) + "\r\n");
	}
	else if (state == StateEnum::leader_state) {
		role = new Leader(this);	
		Tracer::trace("Created Leader." + std::to_string( get_server_id() ) +"\r\n");
	}
	else {
		std::string color_name{ "GREEN" };
		Tracer::trace("Server::get_current_shape_sever -  FAILED!!! Unknow state:" + std::to_string(static_cast<int>(state)) + "%d\r\n");
		role = NULL;
	}
	
	return role;
}

int Server::get_server_id()
{
	return server_id_;
}

void Server::check_new_state() 
{
	while (!have_to_die_) {
		
		semaphore_new_state_.wait(SEMAPHORE_SERVER_NEW_STATE);
		{
			printf("Signal\r\n");
			{
				std::lock_guard<std::mutex> locker(mu_server_);
				printf("locker\r\n");
				if (current_state_ != new_state_) {
					Tracer::trace("Server(" + std::to_string(server_id_) + ") State changes from " + RaftUtils::parse_state_to_string(current_state_) + " to " + RaftUtils::parse_state_to_string(new_state_) + "\r\n", SeverityTrace::change_status_trace);
					current_state_ = new_state_;
					connector_ = get_current_shape_sever(current_state_);
					if (connector_) {
						connector_->start();						
					}
				}
			}
		}
	}
}


void Server::set_new_state(StateEnum state)
{			
	{
		//std::lock_guard<std::mutex> locker_new_state(mu_new_state_);
		Tracer::trace("Server(" + std::to_string(server_id_) + ") New state has been requested\r\n", SeverityTrace::action_trace);
		new_state_ = state;
		semaphore_new_state_.notify(SEMAPHORE_SERVER_NEW_STATE);
	}
}

void Server::increment_current_term()
{	
	current_term_++;
	Tracer::trace("Server(" + std::to_string(server_id_) + ") Increment term from " + std::to_string(current_term_ - 1) + " to " + std::to_string(current_term_) + "\r\n", SeverityTrace::action_trace);
}

int Server::get_current_term()
{
	return current_term_;
}

void Server::set_current_term(int term)
{
	current_term_ = term;
}

void Server::set_commit_index(int commit_index)
{
	commit_index_ = commit_index;
}


int Server::get_commit_index()
{
	return commit_index_;
}

int Server::get_last_applied()
{
	return last_applied_;
}

void Server::set_last_applied(int last_applied)
{
	last_applied_ = last_applied;
}


int32_t Server::get_voted_for()
{
	return voted_for_;
}

void Server::set_voted_for(int32_t vote_for)
{
	voted_for_ = vote_for;
}

int  Server::write_log(int state_machine_command)
{
	// Increment log index; 
	log_.log_index_++;
	// Update term for the entry.
	log_.command_[log_.log_index_].set_term_when_entry_was_received_by_leader(current_term_);
	// Update state machine command. 
	log_.command_[log_.log_index_].set_state_machime_command(state_machine_command);
	// Write log. 
	int ret = manager_log_.write_log(file_log_name_, &log_, sizeof(log_));

	if (ret) {
		Tracer::trace("Server::write_log - FAILED!!! to write in log, error: " + std::to_string(ret) + "\r\n");
	}
	 
	// TEST ONLY
	string str_index;
	string str_term;
	string str_value;
	for (int count = 0; count < log_.log_index_; count++) {

		str_index += "\t|index:" + std::to_string(count)  + "|";
		str_term +=  "\t|term :" + std::to_string(log_.command_[count].get_term_when_entry_was_received_by_leader()) + "|";
		str_value += "\t|value:" + std::to_string(log_.command_[count].get_state_machime_command()) + "|";

	}
	str_index += "\r\n";
	str_term += +"\r\n";
	str_value += +"\r\n";

	Tracer::trace(str_index.c_str());
	Tracer::trace(str_term.c_str());
	Tracer::trace(str_value.c_str());

	return ret;
}

int Server::get_log_index()
{
	return log_.log_index_;
}

int Server::get_term_of_entry_in_log(int log_index)
{
	return log_.command_[log_index].get_term_when_entry_was_received_by_leader();
}

int Server::get_current_leader_id()
{
	return current_leader_id_;
}

int Server::get_state_machime_command(int log_index)
{
	return log_.command_[log_index].get_state_machime_command();
}

void Server::set_current_leader_id(int leader_id)
{
	current_leader_id_ = leader_id;
}

int Server::send_append_entry_rpc(	RPCTypeEnum rpc_type, 
									RPCDirection rpc_direction,		
									int server_id_origin, 
									int server_id_target, 
									int port_target, 
									int argument_term, 
									int argument_leader_id, 
									int argument_prev_log_index, 
									int argument_prev_log_term, 
									int argument_entries[], 
									int argument_leader_commit, 
									int* result_term, 
									int* result_success)
{
	return rpc_api_client_.send_append_entry_rpc(
		rpc_type,
		rpc_direction,
		server_id_origin,
		server_id_target,
		port_target,
		argument_term,
		argument_leader_id,
		argument_prev_log_index,
		argument_prev_log_term,
		argument_entries,
		argument_leader_commit,
		result_term,
		result_success);
}
int Server::send_request_vote_rpc(	RPCTypeEnum rpc_type, 
									RPCDirection rpc_direction, 
									int server_id_origin, 
									int server_id_target, 
									int port_target, 
									int argument_term, 
									int argument_candidate_id, 
									int argument_last_log_index, 
									int argument_last_log_term, 
									int* result_term, 
									int* result_vote_granted) 
{
	return rpc_api_client_.send_request_vote_rpc(
		rpc_type,
		rpc_direction,
		server_id_origin,
		server_id_target,
		port_target,
		argument_term,
		argument_candidate_id,
		argument_last_log_index,
		argument_last_log_term,
		result_term,
		result_vote_granted);
}

void Server::append_entry_server(
	/* [in] */ int argument_term,
	/* [in] */ int argument_leader_id,
	/* [in] */ int argument_prev_log_index,
	/* [in] */ int argument_prev_log_term,
	/* [in] */ int argument_entries[],
	/* [in] */ int argument_leader_commit,
	/* [out] */ int* result_term,
	/* [out] */ int* result_success) {
		{
			std::lock_guard<std::mutex> locker(mu_server_);
			if(connector_)
				connector_->append_entry_role(argument_term,
				argument_leader_id,
				argument_prev_log_index,
				argument_prev_log_term,
				argument_entries,
				argument_leader_commit,
				result_term,
				result_success);
		}
}


void Server::request_vote_server(
	/* [in] */ int argument_term,
	/* [in] */ int argument_candidate_id,
	/* [in] */ int argument_last_log_index,
	/* [in] */ int argument_last_log_term,
	/* [out] */ int* result_term,
	/* [out] */ int* result_vote_granted) {
		{
			std::lock_guard<std::mutex> locker(mu_server_);
			if (connector_)
				connector_->request_vote_role(argument_term,
				argument_candidate_id,
				argument_last_log_index,
				argument_last_log_term,
				result_term,
				result_vote_granted);
		}
}


void Server::panic() {
	while (true) {
		Tracer::trace("Server(" + std::to_string(server_id_) + ") Panic\r\n", SeverityTrace::action_trace);
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	}
}