#include "Leader.h"
#include "Tracer.h"
#include "ClientDefs.h"

Leader::Leader(void* server)
{	
	server_ = server;
	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") I am a LEADER\r\n", SeverityTrace::action_trace);
	threads_have_to_die_	= false;
	master_					= NONE;
	have_to_die_			= false;	
	deadline_master_		= 0;
	
	
	// Initialize next_index_ & match_index arrays. 	
	for (int count = 0; count < NUM_SERVERS; count++) {
		next_index_[count] = ((Server*)server_)->get_log_index() + 1;
		match_index_[count] = 0;
	}	
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
	while ( !have_to_die_ )
	{
		// Send RPC's(Heart beat)in parallel to each of the other servers in the cluster. 
		for (int count = 0; count < NUM_SERVERS; count++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			{
				std::lock_guard<std::mutex> locker_leader(mu_leader_);


				if ( !have_to_die_ ) {

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

void Leader::send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	((Server*)server_)->send_msg_socket(client_request, port, sender, action, receiver);
}


void Leader::dispatch_client_request_leader(ClientRequest* client_request)
{
	//I am a leader, so I replay with my id. 
	client_request->client_result_ = true;
	client_request->client_leader_ = ((Server*)server_)->get_current_leader_id();

	send_msg_socket(client_request,
		SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + client_request->client_id_,
		std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
		std::string(CLIENT_REQUEST_LEADER_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
		std::string(CLIENT_TEXT) + "(Unique)." + std::to_string(client_request->client_id_)
	);
	Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") sending who is leader(" + std::to_string(((Server*)server_)->get_current_leader_id()) + ") to client." + std::to_string(client_request->client_id_) + " .\r\n");
}

void Leader::dispatch_client_request_value(ClientRequest* client_request)
{
	if ( (master_ == NONE) || (deadline_master_ <= client_request->client_seconds_january_1_1970 ) ){

		Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") New value: " + std::to_string(client_request->client_value_) + " \r\n", SeverityTrace::action_trace);
		Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") New value: "  + std::to_string(client_request->client_value_) + " \r\n", SeverityTrace::action_trace);
		Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") New value: " + std::to_string(client_request->client_value_) + " \r\n", SeverityTrace::action_trace);

		master_ = client_request->client_value_;
		deadline_master_ = client_request->client_seconds_january_1_1970 + TIME_OUT_MASTER;
		client_request->client_result_ = 1;
		client_request->client_master_ = master_;


		if (((Server*)server_)->write_log(client_request->client_value_) != MANAGER_NO_ERROR) {
			Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") FAILED to write log\r\n", SeverityTrace::error_trace);
			((Server*)server_)->panic();
		}

		// Update commit_index_
		((Server*)server_)->set_commit_index(((Server*)server_)->get_log_index());

		if (thread_send_append_entry_1th_phase_.joinable()) {
			threads_have_to_die_ = true;
			thread_send_append_entry_1th_phase_.join();
		}

		threads_have_to_die_ = false;
		thread_send_append_entry_1th_phase_ = std::thread(&Leader::send_append_entry_1th_phase, this);

		client_request_ = *client_request;
	}
	else {
		client_request->client_result_ = 1; 
		client_request->client_master_ = master_;
		//I am a leader, so I replay with my id. 
		client_request->client_result_ = true;
		client_request->client_leader_ = ((Server*)server_)->get_current_leader_id();


		send_msg_socket(client_request,
			SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + client_request->client_id_,
			std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
			std::string(CLIENT_REQUEST_LEADER_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
			std::string(CLIENT_TEXT) + "(Unique)." + std::to_string(client_request->client_id_)
		);

	}
}


void Leader::dispatch_client_ping_master(ClientRequest* client_request) 
{
	if ( (client_request->client_value_ == master_) && (deadline_master_ <= client_request->client_seconds_january_1_1970) ) {
		deadline_master_ = client_request->client_seconds_january_1_1970 + TIME_OUT_MASTER;
		Tracer::trace("Leaader:: client ping master: " + std::to_string(client_request->client_value_) + "\r\n");
		Tracer::trace("Leaader:: client ping master: " + std::to_string(client_request->client_value_) +"\r\n");
		Tracer::trace("Leaader:: client ping master: " + std::to_string(client_request->client_value_) + "\r\n");
	}
	else {
		master_ = NONE;
	}
}

void Leader::receive_msg_socket(ClientRequest* client_request)
{
	// A client request a leader
	if (client_request->client_request_type == ClientRequesTypeEnum::client_request_leader) {
		dispatch_client_request_leader(client_request);
	}
	// A client request value
	else if (client_request->client_request_type == ClientRequesTypeEnum::client_write_master) {
		dispatch_client_request_value(client_request);
	}
	else if (client_request->client_request_type == ClientRequesTypeEnum::client_ping_master) {
		dispatch_client_ping_master(client_request);
	}
	else
		Tracer::trace("Leaader::dispatch - Wrong!!! type " + std::to_string(static_cast<int>(client_request->client_request_type)) + "\r\n");
}


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
	if (argument_entries[0] == NONE)
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
			((Server*)server_)->set_current_leader_id(argument_leader_id);
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

			((Server*)server_)->set_current_term(argument_term);
			((Server*)server_)->set_current_leader_id(argument_leader_id);
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


void Leader::send_append_entry_1th_phase() 
{		
	int	 count_followers_ack_to_value_sent					= 0;	

	while( ( !threads_have_to_die_ ) && ( count_followers_ack_to_value_sent < ( NUM_SERVERS - 1/*myself*/ ) ) )
	{
		printf("while send_append_entry_1th_phase\r\n");
		for (int server = 0;( ( server < NUM_SERVERS ) && ( !threads_have_to_die_ )  ); server++)
		{			
			// If server is updated. 
			if (match_index_[server] == ((Server*)server_)->get_commit_index()) {								
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			{
				std::lock_guard<std::mutex> locker_leader(mu_leader_);

				if (!threads_have_to_die_) {

					// If the receiver is not equal to sender...
					if (server != ((Server*)server_)->get_server_id()) {

						printf("SEND send_append_entry_1th_phase: %d\r\n", server);

						int result_term;
						int result_success;
						int	argument_entries[1000];
						argument_entries[0] = ((Server*)server_)->get_state_machime_command(((Server*)server_)->get_log_index());

						int status = ((Server*)server_)->send_append_entry_rpc(
							RPCTypeEnum::rpc_append_entry,
							RPCDirection::rpc_in_invoke,
							((Server*)server_)->get_server_id(),
							server,
							RPC_BASE_PORT + RPC_RECEIVER_PORT + server,
							((Server*)server_)->get_current_term(),														// Leader's term
							((Server*)server_)->get_server_id(),														// So follower can redirect clients
							next_index_[server] - 1,																		// Index of log entry immediately preceding	new ones
							((Server*)server_)->get_term_of_entry_in_log(next_index_[server] -1),						// Term of argument_prev_log_index entry
							argument_entries,																			// Log entries to store(empty for heartbeat; may send more than one for efficiency)
							((Server*)server_)->get_commit_index(),														// Leader’s commitIndex
							&result_term,
							&result_success
						);
						if (status) {

							Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Failed to send append entry(AppendEntry): " + std::to_string(status) + "\r\n", SeverityTrace::error_trace);
						}
						else {
							if (result_success) {
								if (next_index_[server] < ((Server*)server_)->get_commit_index()) {
									match_index_[server] = next_index_[server];
									printf("(next_index_[count]:%d < ((Server*)server_)->get_commit_index():%d\r\n", next_index_[server], ((Server*)server_)->get_commit_index());									
									next_index_[server] = next_index_[server] + 1;
									printf("next_index_[count] + 1 : %d\r\n", next_index_[server]);
									Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent append entry(AppendEntry) to Server." + std::to_string(server) + " successfully(next_index_[" + std::to_string(server) + "] + 1 = " + std::to_string(next_index_[server]) + ").\r\n", SeverityTrace::action_trace);																		

									send_append_entry_2nd_phase(match_index_[server], server);
								}
								else {
									match_index_[server] = ((Server*)server_)->get_commit_index();
									next_index_[server] = next_index_[server] + 1;
									printf("match_index_[count]: %d = ((Server*)server_)->get_commit_index();\r\n", match_index_[server]);
									Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent append entry(AppendEntry) to Server." + std::to_string(server) + " successfully( match_index_[" + std::to_string(server) + "]" + std::to_string(match_index_[server]) + " ).\r\n", SeverityTrace::action_trace);
									
									send_append_entry_2nd_phase(match_index_[server], server);
								}								
							}
							else {
								next_index_[server] = next_index_[server] - 1;
								printf("next_index_[count] : %d = next_index_[count] - 1;\r\n", next_index_[server]);
								Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent append entry(AppendEntry) to Server." + std::to_string(server) + " Consisency check, next_index_[" + std::to_string(server) + "]:" + std::to_string(next_index_[server]) +".\r\n", SeverityTrace::warning_trace);
							}
						}
					}
				}
			}
		}		
	}
}

void Leader::send_append_entry_2nd_phase(int index, int server_to_apply_state_machine)
{
	int count_followers_ack_to_value_sent = 0;
	for (int server = 0; server < NUM_SERVERS; server++) {
		if (match_index_[server] == index)
			count_followers_ack_to_value_sent++;
	}

	if (count_followers_ack_to_value_sent < MAJORITY) {
		Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") There is not enought ACK " + std::to_string(((Server*)server_)->get_state_machime_command(((Server*)server_)->get_log_index())) + "\r\n", SeverityTrace::warning_trace);

	}
	else if (count_followers_ack_to_value_sent == MAJORITY) {
		
		((Server*)server_)->set_last_applied(((Server*)server_)->get_last_applied() + 1);

		Tracer::trace("¡¡¡¡¡¡¡¡(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Applied " + std::to_string(((Server*)server_)->get_state_machime_command(((Server*)server_)->get_log_index())) + " to STATE MACHINE!!!!!!!!\r\n", SeverityTrace::action_trace);

		ClientRequest client_request;
		client_request.client_id_ = client_request_.client_id_;
		strcpy_s(client_request.client_ip_, sizeof(client_request.client_ip_), client_request_.client_ip_);
		client_request.client_leader_ = client_request_.client_leader_;
		client_request.client_master_ = client_request_.client_master_;		
		client_request.client_request_type = ClientRequesTypeEnum::client_write_master;		
		client_request.client_result_ = true;
		

		// Replay Client...
		send_msg_socket(&client_request,
			SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + client_request_.client_id_,
			std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
			std::string(CLIENT_REQUEST_VALUE_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
			std::string(CLIENT_TEXT) + "(Unique)." + std::to_string(client_request.client_id_)
		);
		Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") sending who is leader(" + std::to_string(((Server*)server_)->get_current_leader_id()) + ") to client." + std::to_string(client_request_.client_id_) + " .\r\n");

		for (int server = 0; server < NUM_SERVERS; server++) {

			// If the receiver is not equal to sender...
			if (server != ((Server*)server_)->get_server_id()) {

				if (match_index_[server] == index) {

					int result_term;
					int result_success;
					int	argument_entries[1000];
					argument_entries[0] = ((Server*)server_)->get_state_machime_command(((Server*)server_)->get_log_index());

					int status = ((Server*)server_)->send_append_entry_rpc(
						RPCTypeEnum::rpc_append_entry,
						RPCDirection::rpc_in_invoke,
						((Server*)server_)->get_server_id(),
						server,
						RPC_BASE_PORT + RPC_RECEIVER_PORT + server,
						((Server*)server_)->get_current_term(),														// Leader's term
						((Server*)server_)->get_server_id(),														// So follower can redirect clients
						next_index_[server] - 1,																		// Index of log entry immediately preceding	new ones
						((Server*)server_)->get_term_of_entry_in_log(next_index_[server] - 1),						// Term of argument_prev_log_index entry
						argument_entries,																			// Log entries to store(empty for heartbeat; may send more than one for efficiency)
						index,														// Leader’s commitIndex
						&result_term,
						&result_success
					);
					if (status) {
						Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Failed to send append entry(AppendEntry): " + std::to_string(status) + "\r\n", SeverityTrace::error_trace);
					}
					else {
						if (result_success) {
							Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent append entry(AppendEntry) to apply State machine Follower." + std::to_string(server) + " successfully.\r\n", SeverityTrace::action_trace);
						}
					}
				}
			}
		}
	}
	else {

		int result_term;
		int result_success;
		int	argument_entries[1000];
		argument_entries[0] = ((Server*)server_)->get_state_machime_command(((Server*)server_)->get_log_index());

		int status = ((Server*)server_)->send_append_entry_rpc(
			RPCTypeEnum::rpc_append_entry,
			RPCDirection::rpc_in_invoke,
			((Server*)server_)->get_server_id(),
			server_to_apply_state_machine,
			RPC_BASE_PORT + RPC_RECEIVER_PORT + server_to_apply_state_machine,
			((Server*)server_)->get_current_term(),														// Leader's term
			((Server*)server_)->get_server_id(),														// So follower can redirect clients
			next_index_[server_to_apply_state_machine] - 1,																		// Index of log entry immediately preceding	new ones
			((Server*)server_)->get_term_of_entry_in_log(next_index_[server_to_apply_state_machine] - 1),						// Term of argument_prev_log_index entry
			argument_entries,																			// Log entries to store(empty for heartbeat; may send more than one for efficiency)
			index,														// Leader’s commitIndex
			&result_term,
			&result_success
		);
		if (status) {
			Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Failed to send append entry(AppendEntry): " + std::to_string(status) + "\r\n", SeverityTrace::error_trace);
		}
		else {
			if (result_success) {
				Tracer::trace("(Leader." + std::to_string(((Server*)server_)->get_server_id()) + ") Sent append entry(AppendEntry) to apply State machine Follower." + std::to_string(server_to_apply_state_machine) + " successfully.\r\n", SeverityTrace::action_trace);
			}
		}

	}
}
