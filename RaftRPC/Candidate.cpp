#include "Candidate.h"
#include "Tracer.h"


Candidate::Candidate(void* server)
{	
	server_ = server;	
	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") I am a CANDIDATE\r\n", SeverityTrace::action_trace);
	there_is_leader_					= false;
	have_to_die_						= false;
	count_received_votes_				= 1; // Starts in 1, because we dont send messages to myself. 	
	((Server*)server_)->increment_current_term();
	last_time_stam_taken_miliseconds_	= duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	term_finished_						= false;
	
	// At the beginning nobody has sent a vote. 
	memset(received_votes_, false, sizeof(received_votes_));
}

Candidate::~Candidate()
{
	have_to_die_ = true;	

	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroying...\r\n");
	if (thread_send_request_vote_to_all_servers_.joinable())
		thread_send_request_vote_to_all_servers_.join();
	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Destroyed...\r\n");
}

void Candidate::start()
{
	thread_send_request_vote_to_all_servers_ = std::thread(&Candidate::send_request_vote_to_all_servers, this);	
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));	
}


void Candidate::reset_receive_votes() 
{
	count_received_votes_ = 0; 
}

void Candidate::send_request_vote_to_all_servers() 
{
	while ((!there_is_leader_) && (!have_to_die_)) {

		term_finished_ = false;

		while ((!there_is_leader_) && (!have_to_die_) && (!term_finished_))
		{			
			// Send RPC's(Request vote) in parallel to each of the other servers in the cluster. 
			for (int count = 0; ( (count < NUM_SERVERS) && (!received_votes_[count]) && (!there_is_leader_) && (!have_to_die_) && (!term_finished_) ); count++)
			{
				milliseconds current_time_stam_taken_miliseconds = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
				{
					std::lock_guard<std::mutex> locker_candidate(mu_candidate_);

					if ((!there_is_leader_) && (!have_to_die_)) {

						// If the receiver is not equal to sender...
						if (count != ((Server*)server_)->get_server_id()) {

							int result_term;										// CurrentTerm, for candidate to update itself
							int result_vote_granted;								// True means candidate received vote    


							int status = ((Server*)server_)->send_request_vote_rpc(
								RPCTypeEnum::rpc_append_request_vote,
								RPCDirection::rpc_in_invoke,
								((Server*)server_)->get_server_id(),
								count,
								BASE_PORT + RECEIVER_PORT + count,
								((Server*)server_)->get_current_term(),				// Candidate's term
								((Server*)server_)->get_server_id(),				// Candidate requesting vote
								((Server*)server_)->get_last_applied(),				// Index of candidate's last log entry (§5.4)
								0,													// Term of candidate's last log entry (§5.4)
								&result_term,										// CurrentTerm, for candidate to update itself
								&result_vote_granted								// True means candidate received vote    
							);

							if (status) {
								Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Failed to send request vote error: " + std::to_string(status) + "\r\n", SeverityTrace::error_trace);
							}
							else {
								// And its terms is equal or highest than mine... 
								if (result_term >= ((Server*)server_)->get_current_term()) {
									have_to_die_ = true;
									there_is_leader_ = true;
									Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Accepted]received an request_vote[term:" + std::to_string(result_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");
									// Inform server that state has changed to follower.  
									((Server*)server_)->set_new_state(StateEnum::follower_state);
								}
								else
								{
									if (result_vote_granted == (int)true) {
										count_received_votes_++;
										// If I wins election. 	
										if (count_received_votes_ >= MAJORITY) {
											Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") I have received just mayority of request vote: " + std::to_string(count_received_votes_) + "\r\n");
											((Server*)server_)->set_new_state(StateEnum::leader_state);
											there_is_leader_ = true;
										}
									}
									// If I was rejected. 
									else {
										Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Rejected request voted\r\n");
									}
								}
							}
						}
					}
				}

				if ((!there_is_leader_) && (!have_to_die_)) {
					if ((abs(last_time_stam_taken_miliseconds_.count() - current_time_stam_taken_miliseconds.count())) > TIME_OUT_TERM)
					{
						Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Time out without being Leader\r\n");
						term_finished_ = true;
					}
					else {
						Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Time out term: " + std::to_string((abs(last_time_stam_taken_miliseconds_.count() - current_time_stam_taken_miliseconds.count()))) + "ms > " + std::to_string(TIME_OUT_TERM) + "\r\n", SeverityTrace::warning_trace);
					}
				}
			}
		}

		{
			//std::lock_guard<std::mutex> locker_candidate(mu_candidate_);
			if ((!there_is_leader_) && (!have_to_die_)) {
				// Increments term. 
				((Server*)server_)->increment_current_term();
				// Reset received votes. 
				count_received_votes_ = 0;
				memset(received_votes_, false, sizeof(received_votes_));
				// wait ramdomly. 
				/* initialize random seed: */
				srand((unsigned int)time(NULL));
				// 150-300(ms). 
				int ramdom_timeout = (rand() % MINIMUM_VALUE_RAMDOM_TIME_OUT) + MINIMUM_VALUE_RAMDOM_TIME_OUT;
				std::this_thread::sleep_for(std::chrono::milliseconds(ramdom_timeout));
				last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			}
		}
	}
	
	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") send_request_vote_to_all_servers FINISHED.\r\n");
}

void Candidate::send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	((Server*)server_)->send(rpc, port, sender, action, receiver);
}

void Candidate::dispatch_append_entry(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// And its terms is equal or highest than mine... 
		if (rpc->append_entry.argument_term_ >= ((Server*)server_)->get_current_term()) {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Accepted]received an append_entry claiming to be leader[term:" + std::to_string(rpc->append_entry.argument_term_) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
			rpc->append_entry.result_success_ = true;
			rpc->server_id_origin = ((Server*)server_)->get_server_id();
			rpc->server_id_target = rpc->append_entry.argument_leader_id_;

			send(rpc,
				BASE_PORT + RECEIVER_PORT + rpc->append_entry.argument_leader_id_,
				std::string(SERVER_TEXT) + "(C)." + std::to_string(((Server*)server_)->get_server_id()),
				std::string(APPEND_ENTRY_TEXT) + std::string("(") + std::string(RESULT_TEXT) + std::string(")"),
				std::string(SERVER_TEXT) + "(C)." + std::to_string(rpc->append_entry.argument_leader_id_)
			);
			have_to_die_ = true;
			there_is_leader_ = true;

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
		}
		// Reject...
		else {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Rejected] received an append_entry claiming to be leader[term:" + std::to_string(rpc->append_entry.argument_term_) + " < current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");
			rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
			rpc->append_entry.result_success_ = false;

			send(rpc,
				BASE_PORT + RECEIVER_PORT + rpc->append_entry.argument_leader_id_,
				std::string(SERVER_TEXT) + "(C)." + std::to_string(((Server*)server_)->get_server_id()),
				std::string(APPEND_ENTRY_TEXT) + std::string("(") + std::string(RESULT_TEXT) + std::string(")"),
				std::string(SERVER_TEXT) + "(C)." + std::to_string(rpc->append_entry.argument_leader_id_)
			);
		}
	}
	else if ((rpc->rpc_direction == RPCDirection_sockets::rpc_out_result)) 
	{
	}
}

void Candidate::dispatch_request_vote(RPC_sockets* rpc) 
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// And its terms is equal or highest than mine... 
		if (rpc->request_vote.argument_term_ >= ((Server*)server_)->get_current_term()) {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Accepted]received an request_vote[term:" + std::to_string(rpc->request_vote.argument_term_) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
			rpc->request_vote.result_vote_granted_ = true;
			rpc->server_id_origin = ((Server*)server_)->get_server_id();
			rpc->server_id_target = rpc->append_entry.argument_leader_id_;

			send(rpc,
				BASE_PORT + RECEIVER_PORT + rpc->request_vote.argument_candidate_id_,
				std::string(SERVER_TEXT) + "(C)." + std::to_string(((Server*)server_)->get_server_id()),
				std::string(REQUEST_VOTE_TEXT) + std::string("(") + std::string(RESULT_TEXT) + std::string(")"),
				std::string(SERVER_TEXT) + "(C)." + std::to_string(rpc->request_vote.argument_candidate_id_)
			);

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
		}
		// Reject...
		else {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Rejected]received an request_vote[term:" + std::to_string(rpc->request_vote.argument_term_) + " < current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");
			rpc->rpc_direction = RPCDirection_sockets::rpc_out_result;
			rpc->request_vote.result_vote_granted_ = false;
			rpc->server_id_origin = ((Server*)server_)->get_server_id();
			rpc->server_id_target = rpc->request_vote.argument_candidate_id_;


			send(rpc,
				BASE_PORT + RECEIVER_PORT + rpc->request_vote.argument_candidate_id_,
				std::string(SERVER_TEXT) + "(C)." + std::to_string(((Server*)server_)->get_server_id()),
				std::string(REQUEST_VOTE_TEXT) + std::string("(") + std::string(RESULT_TEXT) + std::string(")"),
				std::string(SERVER_TEXT) + "(C)." + std::to_string(rpc->request_vote.argument_candidate_id_)
			);
		}

	}
	// rpc_out_result
	else if ((rpc->rpc_direction == RPCDirection_sockets::rpc_out_result)) {
		// If someone voted for me. 
		if ((bool)rpc->request_vote.result_term_ == true) {
			count_received_votes_++;
			// If I wins election. 	
			if (count_received_votes_ >= MAJORITY) {
				Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") I have received just mayority of request vote: " + std::to_string(count_received_votes_) + "\r\n");
				((Server*)server_)->set_new_state(StateEnum::leader_state);
				there_is_leader_ = true;
			}			
		}
		// If I was rejected. 
		else {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Rejected request voted\r\n");
		}
	}
}


void Candidate::dispatch_client_request_leader(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// N/A
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
		// N/A
	}
}


void Candidate::dispatch_client_request_value(RPC_sockets* rpc)
{
	if (rpc->rpc_direction == RPCDirection_sockets::rpc_in_invoke) {
		// N/A
	}
	else if (rpc->rpc_direction == RPCDirection_sockets::rpc_out_result) {
		// N/A
	}
}


void Candidate::dispatch_append_heart_beat(RPC_sockets* rpc) 
{
	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") received heart_beat from another Leader...\r\n");

	// save current leader's id.
	((Server*)server_)->set_current_leader_id(rpc->append_entry.argument_leader_id_);

	((Server*)server_)->set_new_state(StateEnum::follower_state);
	there_is_leader_ = true;

}

void Candidate::receive(RPC_sockets* rpc)
{
	dispatch(rpc);
}

void Candidate::dispatch(RPC_sockets* rpc) 
{
	std::lock_guard<std::mutex> locker_candidate(mu_candidate_);

	if ((!there_is_leader_) && (!have_to_die_)) {
		// If I receive an append_entry(// Another server establishes itself as a leader. )
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
			Tracer::trace("Candidate::dispatch - Wrong!!! type " + std::to_string(static_cast<int>(rpc->rpc_type)) + "\r\n");

	}
}


void Candidate::append_entry_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_leader_id,
	/* [in] */ int argument_prev_log_index,
	/* [in] */ int argument_prev_log_term,
	/* [in] */ int argument_entries[1000],
	/* [in] */ int argument_leader_commit,
	/* [out] */ int* result_term,
	/* [out] */ int* result_success) 
{

	// Heart beat...(argument entries is empty.) 
	if (argument_entries[0] == 0) 
	{
		Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") received heart_beat from another Leader...\r\n");

		// save current leader's id.
		((Server*)server_)->set_current_leader_id(argument_leader_id);

		((Server*)server_)->set_new_state(StateEnum::follower_state);
		there_is_leader_ = true;
	}
	else
	{		
		// And its terms is equal or highest than mine... 
		if (argument_term >= ((Server*)server_)->get_current_term()) {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Accepted]received an append_entry claiming to be leader[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			*result_success		= true;
			have_to_die_		= true;
			there_is_leader_	= true;

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
		}
		// Reject...
		else {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Rejected] received an append_entry claiming to be leader[term:" + std::to_string(argument_term) + " < current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");
			*result_success = false;
		}
	}
}


void Candidate::request_vote_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_candidate_id,
	/* [in] */ int argument_last_log_index,
	/* [in] */ int argument_last_log_term,
	/* [out] */ int* result_term,
	/* [out] */ int* result_vote_granted) {

	// If there is not leader yet. 
	if (there_is_leader_) {

		// And its terms is equal or highest than mine... 
		if (argument_term >= ((Server*)server_)->get_current_term()) {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Accepted]received an request_vote[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			*result_vote_granted = true;

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
		}
		// Reject...
		else {
			Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [Rejected]received an request_vote[term:" + std::to_string(argument_term) + " < current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			*result_vote_granted = false;
		}
	}
}