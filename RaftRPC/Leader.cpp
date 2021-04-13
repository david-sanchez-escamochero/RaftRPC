#include "Leader.h"
#include "Tracer.h"
#include "ClientDefs.h"

Leader::Leader(void* server)
{	
	server_ = server;
	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") I am a LEADER\r\n");

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

	if (thread_check_leader_time_out_to_change_term_.joinable())
		thread_check_leader_time_out_to_change_term_.join();

	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroyed...\r\n");
}

void Leader::start()
{
	// I'm a leader.
	((Server*)server_)->set_current_leader_id(((Server*)server_)->get_server_id());											

	thread_send_heart_beat_all_servers_ = std::thread(&Leader::send_heart_beat_all_servers, this);

	thread_check_leader_time_out_to_change_term_ = std::thread(&Leader::check_leader_time_out_to_change_term, this);
}

void Leader::check_leader_time_out_to_change_term() 
{
	uint32_t count_term = (TIME_OUT_LEADER_TERM / 1000);
	milliseconds last_time_stamp_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	while (!have_to_die_) {
		{
			std::lock_guard<std::mutex> locker_leader(mu_leader_);
			milliseconds current_time_stam_taken_miliseconds = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

			count_term = count_term - (TIME_OUT_WAIT / 1000);

			Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Count term["+std::to_string(TIME_OUT_LEADER_TERM/1000)+"]:" + std::to_string(count_term) + " \r\n");

			if ((abs(last_time_stamp_taken_miliseconds_.count() - current_time_stam_taken_miliseconds.count())) > TIME_OUT_LEADER_TERM) {
				Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") TERM was finished\r\n");
				// Inform server that state has changed to follower.  
				((Server*)server_)->set_new_state(StateEnum::follower_state);
			}
			
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_OUT_WAIT));
	}
}



void Leader::send_heart_beat_all_servers() 
{
	while ((!term_is_not_timeout_) && (!have_to_die_))
	{
		// Send RPC's(Heart beat)in parallel to each of the other servers in the cluster. 
		for (uint32_t count = 0; count < NUM_SERVERS; count++)
		{
			{
				std::lock_guard<std::mutex> locker_leader(mu_leader_);


				if ((!term_is_not_timeout_) && (!have_to_die_)) {

					// If the receiver is not equal to sender...
					if (count != ((Server*)server_)->get_server_id()) {
						RPC_sockets rpc;
						rpc.server_id_origin = ((Server*)server_)->get_server_id();
						rpc.server_id_target = count;
						rpc.rpc_type = RPCTypeEnum_sockets::rpc_append_heart_beat;
						rpc.rpc_direction = RPCDirection_sockets::rpc_in_invoke;
						rpc.append_entry.argument_term_ = ((Server*)server_)->get_current_term();															// Leader's term
						rpc.append_entry.argument_leader_id_ = ((Server*)server_)->get_server_id();															// So follower can redirect clients
						rpc.append_entry.argument_prev_log_index_ = ((Server*)server_)->get_log_index() - 1;												// Index of log entry immediately preceding	new ones
						rpc.append_entry.argument_prev_log_term_ = ((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index() - 1);	// Term of argument_prev_log_index entry
						rpc.append_entry.argument_entries_[0] = NONE;																							// Log entries to store(empty for heartbeat; may send more than one for efficiency)
						rpc.append_entry.argument_leader_commit_ = ((Server*)server_)->get_commit_index();													// Leader’s commitIndex


						send(&rpc,
							BASE_PORT + RECEIVER_PORT + count,
							std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
							std::string(HEART_BEAT_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
							std::string(SERVER_TEXT) + "(ALL)." + std::to_string(count)
						);
						Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent Heart-beat to Server." + std::to_string(count)+ "\r\n");
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

void Leader::send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	((Server*)server_)->send(rpc, port, sender, action, receiver);
}

void Leader::receive(RPC_sockets* rpc)
{
	dispatch(rpc);
}


void Leader::dispatch_append_entry(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
	}
}

void Leader::dispatch_request_vote(RPC_sockets* rpc) {

	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
	}
}

void Leader::dispatch_append_heart_beat(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// TODO:
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
		if(rpc->append_entry.result_success_ == (uint32_t)true)
			Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") ACK heart beat Server\r\n");
		else 
			Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") FAILED!!! ACK heart beat Server\r\n");
	}
}

void Leader::dispatch_client_request_leader(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// Actually we are Leader, so we reply with our id.
		rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
		rpc->client_request.client_result_ = true;
		rpc->client_request.client_leader_ = ((Server*)server_)->get_server_id();	

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

void Leader::dispatch_client_request_value(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {

		// Write to log.

		
		uint32_t client_value = rpc->client_request.client_value_;
		uint32_t ret = ((Server*)server_)->write_log(client_value);
		if (ret == MANAGER_NO_ERROR) {
			for (uint32_t num_server = 0; num_server < NUM_SERVERS; num_server++) {
				RPC_sockets rpc;
				rpc.rpc_type = RPCTypeEnum_sockets::rpc_append_entry;
				rpc.rpc_direction = RPCDirection_sockets::rpc_in_invoke;
				rpc.append_entry.argument_term_				= ((Server*)server_)->get_current_term();													// Leader's term
				rpc.append_entry.argument_leader_id_		= ((Server*)server_)->get_server_id();														// So follower can redirect clients
				rpc.append_entry.argument_prev_log_index_	= ((Server*)server_)->get_log_index() - 1;													// Index of log entry immediately preceding	new ones
				rpc.append_entry.argument_prev_log_term_	= ((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index() - 1);	// Term of argument_prev_log_index entry
				// TODO:
				rpc.append_entry.argument_entries_[0]		= 0;																				// Log entries to store(empty for heartbeat; may send more than one for efficiency)
				rpc.append_entry.argument_leader_commit_	= ((Server*)server_)->get_commit_index();													// Leader’s commitIndex

				send(&rpc,
					BASE_PORT + RECEIVER_PORT + num_server,
					std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
					std::string(APPEND_ENTRY_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
					std::string(SERVER_TEXT) + "(ALL)." + std::to_string(num_server)
				);
			}
		}
		else {
			Tracer::trace("Leader::dispatch_client_request_value - FAILED to write log, error " + std::to_string(ret) + "\r\n");
		}
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
		// N/A
	}
}


void Leader::dispatch(RPC_sockets* rpc)
{
	std::lock_guard<std::mutex> locker(mu_leader_);

	if (!have_to_die_) {

		if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_append_entry)
		{
			dispatch_append_entry(rpc);
		}
		// If I receive a request vote(// Another server is faster than I am. )
		else if (rpc->rpc_type == RPCTypeEnum_sockets::rpc_append_request_vote)
		{
			dispatch_request_vote(rpc);
		}
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
			Tracer::trace("Leader::dispatch - Wrong!!! type " + std::to_string(static_cast<int>(rpc->rpc_type)) + "\r\n");
	}
}



