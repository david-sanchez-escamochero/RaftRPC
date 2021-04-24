#include "Leader.h"
#include "Tracer.h"
#include "ClientDefs.h"

Leader::Leader(void* server)
{	
	server_ = server;
	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") I am a LEADER\r\n", SeverityTrace::action_trace);

	have_to_die_		= false;	
	term_is_not_timeout_= false;
}

Leader::~Leader()
{
	have_to_die_ = true;

	cv_send_heart_beat_all_servers_.notify_all();

	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroying...\r\n");
	if (thread_send_heart_beat_all_servers_.joinable())
		thread_send_heart_beat_all_servers_.join();

	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroyed...\r\n");
}

void Leader::start()
{
	// I'm a leader.
	((Server*)server_)->set_current_leader_id(((Server*)server_)->get_server_id());											

	thread_send_heart_beat_all_servers_ = std::thread(&Leader::send_heart_beat_all_servers, this);
		
}

void Leader::send_heart_beat_all_servers() 
{
	while ((!term_is_not_timeout_) && (!have_to_die_))
	{
		// Send RPC's(Heart beat)in parallel to each of the other servers in the cluster. 
		for (int count = 0; count < NUM_SERVERS; count++)
		{
			{
				std::lock_guard<std::mutex> locker_leader(mu_leader_);


				if ((!term_is_not_timeout_) && (!have_to_die_)) {

					// If the receiver is not equal to sender...
					if (count != ((Server*)server_)->get_server_id()) {

						int result_term;
						int result_success;
						int	argument_entries[1000];
						argument_entries[0] = NONE;

						int status = ((Server*)server_)->send_append_entry_rpc(
							RPCTypeEnum::rpc_append_heart_beat,
							RPCDirection::rpc_in_invoke,
							((Server*)server_)->get_server_id(),
							count,
							RPC_BASE_PORT + RPC_RECEIVER_PORT + count,
							((Server*)server_)->get_current_term(),														// Leader's term
							((Server*)server_)->get_server_id(),														// So follower can redirect clients
							((Server*)server_)->get_log_index() - 1,													// Index of log entry immediately preceding	new ones
							((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index() - 1),		// Term of argument_prev_log_index entry
							argument_entries,																						// Log entries to store(empty for heartbeat; may send more than one for efficiency)
							((Server*)server_)->get_commit_index(),														// Leader’s commitIndex
							&result_term,
							&result_success
						);
						if (status) {
							Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Failed to send append entry(Heart-beat): " + std::to_string(status) + "\r\n", SeverityTrace::error_trace);
						}
						else {
							Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent append entry(Heart-beat) to Server." + std::to_string(count) + "\r\n", SeverityTrace::action_trace);
						}
					}
				}
			}
		}

		{			
			std::mutex mtx;
			std::unique_lock<std::mutex> lck(mtx);
			cv_send_heart_beat_all_servers_.wait_for(lck, std::chrono::milliseconds(TIME_OUT_HEART_BEAT));
		}
	}
	

}

void Leader::send(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	((Server*)server_)->send(client_request, port, sender, action, receiver);
}

void Leader::receive(ClientRequest* client_request)
{
	//dispatch(rpc);
}

//void Leader::dispatch_client_request_value(RaftSockets* rpc)
//{
//	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
//
//		// Write to log.
//
//		
//		int client_value = rpc->client_request.client_value_;
//		int ret = ((Server*)server_)->write_log(client_value);
//		if (ret == MANAGER_NO_ERROR) {
//			for (int num_server = 0; num_server < NUM_SERVERS; num_server++) {
//				RaftSockets rpc;
//				rpc.rpc_type = RPCTypeEnum_sockets::rpc_append_entry;
//				rpc.rpc_direction = RPCDirection_sockets::rpc_in_invoke;
//				rpc.append_entry.argument_term_				= ((Server*)server_)->get_current_term();													// Leader's term
//				rpc.append_entry.argument_leader_id_		= ((Server*)server_)->get_server_id();														// So follower can redirect clients
//				rpc.append_entry.argument_prev_log_index_	= ((Server*)server_)->get_log_index() - 1;													// Index of log entry immediately preceding	new ones
//				rpc.append_entry.argument_prev_log_term_	= ((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index() - 1);	// Term of argument_prev_log_index entry
//				// TODO:
//				rpc.append_entry.argument_entries_[0]		= 0;																				// Log entries to store(empty for heartbeat; may send more than one for efficiency)
//				rpc.append_entry.argument_leader_commit_	= ((Server*)server_)->get_commit_index();													// Leader’s commitIndex
//
//				send(&rpc,
//					BASE_PORT + RECEIVER_PORT + num_server,
//					std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
//					std::string(APPEND_ENTRY_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
//					std::string(SERVER_TEXT) + "(ALL)." + std::to_string(num_server)
//				);
//			}
//		}
//		else {
//			Tracer::trace("Leader::dispatch_client_request_value - FAILED to write log, error " + std::to_string(ret) + "\r\n");
//		}
//	}
//	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
//		// N/A
//	}
//}




void Leader::append_entry_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_leader_id,
	/* [in] */ int argument_prev_log_index,
	/* [in] */ int argument_prev_log_term,
	/* [in] */ int argument_entries[1000],
	/* [in] */ int argument_leader_commit,
	/* [out] */ int* result_term,
	/* [out] */ int* result_success) {

	std::lock_guard<std::mutex> locker_candidate(mu_leader_);
	
	// Heart beat...(argument entries is empty.) 
	if (argument_entries[0] == 0)
	{
		// If term is out of date
		if (argument_term < ((Server*)server_)->get_current_term()) {
			*result_term = ((Server*)server_)->get_current_term();
			*result_success = false;
			Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") [HeartBeat::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
		}
		// If I discover current leader or new term
		else if (argument_term >= ((Server*)server_)->get_current_term()) {
			Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") [HeartBeat::Accepted] Received an append_entry claiming to be leader[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			*result_success = true;
			have_to_die_ = true;			

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
			((Server*)server_)->set_current_term(argument_term);
		}
		else {
			Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
		}
	}
	// Append entry
	else
	{
		// If term is out of date
		if (argument_term < ((Server*)server_)->get_current_term()) {
			*result_term = ((Server*)server_)->get_current_term();
			*result_success = false;
			Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
		}
		// And its terms is equal or highest than mine... 
		else if (argument_term >= ((Server*)server_)->get_current_term()) {
			Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Accepted] Received an append_entry claiming to be leader[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			*result_success = true;
			have_to_die_ = true;			

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
		}
		else {
			Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
		}
	}	
}


void Leader::request_vote_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_candidate_id,
	/* [in] */ int argument_last_log_index,
	/* [in] */ int argument_last_log_term,
	/* [out] */ int* result_term,
	/* [out] */ int* result_vote_granted) {

	std::lock_guard<std::mutex> locker_candidate(mu_leader_);


	// If term is out of date
	if (argument_term < ((Server*)server_)->get_current_term()) {
		*result_term = ((Server*)server_)->get_current_term();
		*result_vote_granted = false;
		Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
	}
	// And its terms is equal or highest than mine... 
	else if (argument_term >= ((Server*)server_)->get_current_term()) {
		Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Accepted] received a request_vote[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

		*result_vote_granted = false;
		*result_term = ((Server*)server_)->get_current_term();

		// Inform server that state has changed to follower.  
		((Server*)server_)->set_new_state(StateEnum::follower_state);		
	}
	else {
		Tracer::trace(">>>>>[RECEVIVED](Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n");
	}
}


