#include "Follower.h"
#include <thread>
#include "Tracer.h"




Follower::Follower(void* server)
{	
	server_											= server;
	Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") I am a Follower\r\n", SeverityTrace::action_trace);
	have_to_die_									= false;
	receiving_heartbeats_							= ELECTION_TIME_OUT; // Tries alllowed without receiving heartbeats 
	last_time_stam_taken_miliseconds_				= duration_cast<milliseconds>(system_clock::now().time_since_epoch());	
	count_check_if_there_is_candidate_or_leader_	= 0;
	((Server*)server_)->set_voted_for(NONE);
}

Follower::~Follower()
{
	have_to_die_ = true;
	
	Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroying...\r\n");
	if (thread_check_candidate_.joinable()) {
		thread_check_candidate_.join();
	}
	Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroyed...\r\n");
}


void Follower::start()
{
	thread_check_candidate_ = std::thread(&Follower::check_if_there_is_candidate_or_leader, this);		
}


void Follower::check_if_there_is_candidate_or_leader() 
{
	count_check_if_there_is_candidate_or_leader_ = 0;
	while (!have_to_die_) {
		{
			std::lock_guard<std::mutex> locker(mu_follower_); 

			if (!have_to_die_) {
				milliseconds current_time_stam_taken_miliseconds = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

				Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Waiting if there is candidate or leader " + std::to_string(count_check_if_there_is_candidate_or_leader_++) + "...\r\n", SeverityTrace::warning_trace);

				if ((abs(last_time_stam_taken_miliseconds_.count() - current_time_stam_taken_miliseconds.count())) > TIME_OUT_CHECK_IF_THERE_IS_CANDIDATE_OR_LEADER)
				{
					Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Time out without receiving messages from Candidate or Leader\r\n");
					((Server*)server_)->set_new_state(StateEnum::candidate_state);
					have_to_die_ = true;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_OUT_IF_THERE_IS_CANDIDATE_OR_LEADER));		
	}
}


void Follower::send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	(((Server*)server_)->send(rpc, port, sender, action, receiver));
}

void Follower::receive(RPC_sockets* rpc)
{	
	dispatch(rpc);
}

void Follower::dispatch_append_entry(RPC_sockets* rpc) 
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		if (
			( rpc->append_entry.argument_term_ == ((Server*)server_)->get_current_term() ) && 
			( rpc->append_entry.argument_prev_log_index_ ==  ((Server*)server_)->get_log_index() - 1 ) &&
			( rpc->append_entry.argument_prev_log_term_ == ((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index() - 1) ) && 
			( rpc->append_entry.argument_leader_commit_ == ((Server*)server_)->get_commit_index() )
			) {

		}
		else {
		}
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
	}
}
		
//void Follower::dispatch_request_vote(RPC_sockets* rpc) {
//
//	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
//		// Set as result. 
//		rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
//
//		// If term is out of date
//		if (rpc->request_vote.argument_term_ < ((Server*)server_)->get_current_term()) {
//			rpc->request_vote.result_term_ = false;
//			rpc->request_vote.result_term_ = ((Server*)server_)->get_current_term();
//			Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Term is out of date " + std::to_string(rpc->request_vote.argument_term_) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n");
//		}
//		// I have already voted.
//		else if (((Server*)server_)->get_voted_for() != NONE) {
//			rpc->request_vote.result_term_ = false;
//			rpc->request_vote.result_term_ = ((Server*)server_)->get_current_term();
//			Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") I have already voted:" + std::to_string(((Server*)server_)->get_voted_for()) + "\r\n");
//		}
//		// Candidate's term is updated...
//		// CandidateID is not null. 
//		// Candidate's log is at least as up-to-date receivers's log, grant vote.
//		else if (
//			(rpc->request_vote.argument_term_ >= ((Server*)server_)->get_current_term()) &&
//			(rpc->request_vote.argument_candidate_id_ != NONE)							 //&& 
//			//(rpc->request_vote.argument_last_log_index_ == 0)							 && TODO: ?¿?¿?¿?¿?¿?¿?¿?¿?¿?
//			//(rpc->request_vote.argument_last_log_term_ == 0)								TODO: ?¿?¿?¿?¿?¿?¿?¿?¿?¿?¿
//			)
//		{
//			((Server*)server_)->set_current_term(rpc->request_vote.argument_term_);
//			((Server*)server_)->set_voted_for(rpc->request_vote.argument_candidate_id_);
//			Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Vote granted to :" + std::to_string(rpc->request_vote.argument_candidate_id_) + "\r\n");
//		}
//
//
//		send(rpc,
//			BASE_PORT + RECEIVER_PORT + rpc->request_vote.argument_candidate_id_,
//			std::string(SERVER_TEXT) + "(F)." + std::to_string(((Server*)server_)->get_server_id()),
//			std::string(REQUEST_VOTE_TEXT) + std::string("(") + std::string(RESULT_TEXT) + std::string(")"),
//			std::string(SERVER_TEXT) + "(C)." + std::to_string(rpc->request_vote.argument_candidate_id_)
//		);
//	}
//	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
//	}
//}
		
void Follower::dispatch_append_heart_beat(RPC_sockets* rpc) 
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {

		// save current leader's id.
		((Server*)server_)->set_current_leader_id(rpc->append_entry.argument_leader_id_);

		Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Heart beat from Leader." + std::to_string(rpc->append_entry.argument_leader_id_) + "\r\n");

		rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
		rpc->append_entry.result_success_ = true;

		send(rpc,
			BASE_PORT + RECEIVER_PORT + rpc->append_entry.argument_leader_id_,
			std::string(SERVER_TEXT) + "(F)." + std::to_string(((Server*)server_)->get_server_id()),
			std::string(HEART_BEAT_TEXT) + std::string("(") + std::string(RESULT_TEXT) + std::string(")"),
			std::string(SERVER_TEXT) + "(L)." + std::to_string(rpc->append_entry.argument_leader_id_)
		);
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
	}
}

void Follower::dispatch_client_request_leader(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// We are not Leader, so we reply with leader's id.( If I known it... )  
		rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
		rpc->client_request.client_result_ = true;
		rpc->client_request.client_leader_ = ((Server*)server_)->get_current_leader_id();

		send(rpc,
			BASE_PORT + RECEIVER_PORT + rpc->client_request.client_id_,
			std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
			std::string(HEART_BEAT_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
			std::string(CLIENT_TEXT) + "(Unique)." + std::to_string(rpc->client_request.client_id_)
		);
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
		// N/A
	}
}

void Follower::dispatch_client_request_value(RPC_sockets* rpc)
{
	// N/A	
}



void Follower::dispatch(RPC_sockets* rpc) 
{
	std::lock_guard<std::mutex> locker(mu_follower_);
	
	if (!have_to_die_) {
		last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		count_check_if_there_is_candidate_or_leader_ = 0;

		if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_append_entry)
		{
			dispatch_append_entry(rpc);
		}
		// If I receive a request vote(// Another server is faster than I am. )
		//else if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_append_request_vote)
		//{
		//	dispatch_request_vote(rpc);
		//}
		// Another server establishes itself as a leader. 
		else if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_append_heart_beat) {
			dispatch_append_heart_beat(rpc);
		}
		// A client request a leader
		else if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_client_request_leader) {
			dispatch_client_request_leader(rpc);
		}
		// A client request value
		else if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_client_request_value) {
			dispatch_client_request_value(rpc);
		}
		else 
			Tracer::trace("Follower::dispatch - Wrong!!! type " + std::to_string(static_cast<int>(rpc->rpc_type)) + "\r\n");
	}
}

void Follower::append_entry_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_leader_id,
	/* [in] */ int argument_prev_log_index,
	/* [in] */ int argument_prev_log_term,
	/* [in] */ int argument_entries[1000],
	/* [in] */ int argument_leader_commit,
	/* [out] */ int* result_term,
	/* [out] */ int* result_success) {
	printf("FOLLOWER - append_entry_role\r\n");

	// Heart beat...(argument entries is empty.) 
	if (argument_entries == NULL) {
		last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		count_check_if_there_is_candidate_or_leader_ = 0;
	}
	// Append entry...
	else {	
	}
}


void Follower::request_vote_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_candidate_id,
	/* [in] */ int argument_last_log_index,
	/* [in] */ int argument_last_log_term,
	/* [out] */ int* result_term,
	/* [out] */ int* result_vote_granted) {
	printf("FOLLOWER - request_vote_role\r\n");
	
	// If term is out of date
	if (argument_term < ((Server*)server_)->get_current_term()) {
		*result_vote_granted = false;
		*result_term = ((Server*)server_)->get_current_term();
		Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
	}
	// I have already voted.
	else if (((Server*)server_)->get_voted_for() != NONE) {
		*result_vote_granted = false;
		*result_term = ((Server*)server_)->get_current_term();
		Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") I have already voted:" + std::to_string(((Server*)server_)->get_voted_for()) + "\r\n", SeverityTrace::error_trace);
	}
	// Candidate's term is updated...
	// CandidateID is not null. 
	// Candidate's log is at least as up-to-date receivers's log, grant vote.
	else if (
		(argument_term >= ((Server*)server_)->get_current_term()) &&
		(argument_candidate_id != NONE)							 //&& 
		//(rpc->request_vote.argument_last_log_index_ == 0)							 && TODO: ?¿?¿?¿?¿?¿?¿?¿?¿?¿?
		//(rpc->request_vote.argument_last_log_term_ == 0)								TODO: ?¿?¿?¿?¿?¿?¿?¿?¿?¿?¿
		)
	{
		((Server*)server_)->set_current_term(argument_term);
		((Server*)server_)->set_voted_for(argument_candidate_id);
		*result_vote_granted = true;
		Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Vote granted to :" + std::to_string(argument_candidate_id) + "\r\n", SeverityTrace::action_trace);
	}
}