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

	if (thread_server_dispatch_.joinable()) {
		thread_server_dispatch_.join();
	}

	if (thread_check_new_state_.joinable()) {
		thread_check_new_state_.join();
	}
}

void Server::send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	communication_.sendMessage(rpc, port, sender, action, receiver);
}

void Server::start() 
{		
	int ret = manager_log_.read_log(file_log_name_, &log_, sizeof(log_));
	if (ret != MANAGER_NO_ERROR) {
		Tracer::trace("Server::start - FAILED!!! to read " + file_log_name_ +" log, stopping server...\r\n");
		return;
	}
	
	// TEST.
	//std::this_thread::sleep_for(std::chrono::milliseconds(30000));

	// Start server thread. 
	//thread_server_dispatch_ = std::thread(&Server::dispatch, this);	

	// Start server check new state
	thread_check_new_state_ = std::thread(&Server::check_new_state, this);
	


	// Start new Rol(Follower at the beginning)
	set_new_state(StateEnum::follower_state);							


	//receive();
	keep_alive();
}


void Server::dispatch()
{
	while(!have_to_die_)
	{
		semaphore_dispatch_.wait(SEMAPHORE_SERVER_DISPATCH);		
		// Get rpc(FIFO)
		RPC_sockets rpc = queue_.front();
		// Delete rcp from queue. 
		queue_.pop();
		{
			std::lock_guard<std::mutex> locker(mu_server_);

			if (connector_ != nullptr) {	
				connector_->receive(&rpc);
			}
		}
	}
}

void Server::keep_alive()
{
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}	
}


void* Server::receive()
{
	while (!have_to_die_) {
		RPC_sockets rpc;		
		int error = communication_.receiveMessage( &rpc, BASE_PORT + RECEIVER_PORT + server_id_, std::string( SERVER_TEXT ) + std::string( "." ) + std::to_string( get_server_id() ) );

		if (error) {
			Tracer::trace("Follower::receive - FAILED!!!  - error" + std::to_string(error) + "\r\n");
		}
		else {
			int xx = sizeof(rpc);
			queue_.push(rpc);
			semaphore_dispatch_.notify(SEMAPHORE_SERVER_DISPATCH);
		}
	}
	return nullptr;
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
			{
				//std::lock_guard<std::mutex> locker_new_state(mu_new_state_);
				std::lock_guard<std::mutex> locker(mu_server_);
				if (current_state_ != new_state_) {
					Tracer::trace("Server(" + std::to_string(server_id_) + ") State changes from " + RaftUtils::parse_state_to_string(current_state_) + " to " + RaftUtils::parse_state_to_string(new_state_) + "\r\n", SeverityTrace::change_status_trace);
					current_state_ = new_state_;
					connector_ = get_current_shape_sever(current_state_);

					connector_->start();
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


int Server::get_commit_index()
{
	return commit_index_;
}

int Server::get_last_applied()
{
	return last_applied_;
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
	log_.log_[log_.log_index_].set_term_when_entry_was_received_by_leader(current_term_);
	// Update state machine command. 
	log_.log_[log_.log_index_].set_state_machime_command(state_machine_command);
	// Wirte log. 
	int ret = manager_log_.write_log(file_log_name_, &log_, sizeof(log_));

	if (ret) {
		Tracer::trace("Server::write_log - FAILED!!! to write in log, error: " + std::to_string(ret) + "\r\n");
	}
	return ret;
}

int Server::get_log_index()
{
	return log_.log_index_;
}

int Server::get_term_of_entry_in_log(int log_index)
{
	return log_.log_[log_index].get_term_when_entry_was_received_by_leader();
}

int Server::get_current_leader_id()
{
	return current_leader_id_;
}

void Server::set_current_leader_id(int leader_id)
{
	current_leader_id_ = leader_id;
}
