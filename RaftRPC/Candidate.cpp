#include "Candidate.h"
#include "Tracer.h"
#include "ClientDefs.h"

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
							RPC_BASE_PORT + RPC_RECEIVER_PORT + count,
							((Server*)server_)->get_current_term(),												// Candidate's term
							((Server*)server_)->get_server_id(),												// Candidate requesting vote
							((Server*)server_)->get_log_index(),												// Index of candidate's last log entry (?5.4)
							((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index()),	// Term of candidate's last log entry (?5.4)
							&result_term,																		// CurrentTerm, for candidate to update itself
							&result_vote_granted																// True means candidate received vote    
						);

						{
							std::lock_guard<std::mutex> locker_candidate(mu_candidate_);
							if (!there_is_leader_) 
							{
								if (status) {
									Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [SendAllServer::FAILED] Failed to send request vote error: " + std::to_string(status) + "\r\n", SeverityTrace::error_trace);
								}
								else {
									// And its terms is equal or highest than mine... 
									if (result_term >= ((Server*)server_)->get_current_term()) {
										have_to_die_ = true;
										there_is_leader_ = true;
										Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [SendAllServer::Accepted] Received an request_vote[term:" + std::to_string(result_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");
										// Inform server that state has changed to follower.  
										((Server*)server_)->set_new_state(StateEnum::follower_state);
									}
									else
									{
										if (result_vote_granted == (int)true) {
											count_received_votes_++;
											// If I wins election. 	
											if (count_received_votes_ >= MAJORITY) {
												Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [SendAllServer] I have received just mayority of request vote: " + std::to_string(count_received_votes_) + "\r\n");
												((Server*)server_)->set_new_state(StateEnum::leader_state);
												there_is_leader_ = true;
											}
										}
										// If I was rejected. 
										else {
											Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [SendAllServer::Rejected] Rejected request voted\r\n");
										}
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
			/* initialize random seed: */
			srand((unsigned int)time(NULL));
			int ramdom_timeout = (rand() % MINIMUM_VALUE_RAMDOM_TIME_OUT) + MINIMUM_VALUE_RAMDOM_TIME_OUT;
			std::this_thread::sleep_for(std::chrono::milliseconds(ramdom_timeout));
			Tracer::trace("ramdom_timeout(1): " + std::to_string(ramdom_timeout) + "\r\n");
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
				Tracer::trace("ramdom_timeout(2): " + std::to_string(ramdom_timeout)+"\r\n");
				std::this_thread::sleep_for(std::chrono::milliseconds(ramdom_timeout));
				last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			}
		}
	}
	
	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") send_request_vote_to_all_servers FINISHED.\r\n");
}

void Candidate::send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	((Server*)server_)->send_msg_socket(client_request, port, sender, action, receiver);
}


void Candidate::dispatch_client_request_leader(ClientRequest* client_request)
{
	// We are not Leader, so we reply with leader's id.( If I known it... )  	
	client_request->client_result_ = true;
	client_request->client_leader_ = ((Server*)server_)->get_current_leader_id();


	send_msg_socket(client_request,
		SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + client_request->client_id_,
		std::string(SERVER_TEXT) + "(L)." + std::to_string(((Server*)server_)->get_server_id()),
		std::string(HEART_BEAT_TEXT) + std::string("(") + std::string(INVOKE_TEXT) + std::string(")"),
		std::string(CLIENT_TEXT) + "(Unique)." + std::to_string(client_request->client_id_)
	);
	Tracer::trace("(Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") sending who is leader("+ std::to_string(((Server*)server_)->get_current_leader_id()) +") to client." + std::to_string(client_request->client_id_) +" .\r\n");
}

void Candidate::dispatch_client_request_value(ClientRequest* client_request)
{
	// N/A	
}

void Candidate::receive_msg_socket(ClientRequest* client_request)
{
	// A client request a leader
	if (client_request->client_request_type == ClientRequesTypeEnum::client_request_leader) {
		dispatch_client_request_leader(client_request);
	}
	// A client request value
	else if (client_request->client_request_type == ClientRequesTypeEnum::client_write_master) {
		dispatch_client_request_value(client_request);
	}
	else
		Tracer::trace("Follower::dispatch - Wrong!!! type " + std::to_string(static_cast<int>(client_request->client_request_type)) + "\r\n");
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

	std::lock_guard<std::mutex> locker_candidate(mu_candidate_);

	if (!there_is_leader_) {

		// Heart beat...(argument entries is empty.) 
		if (argument_entries[0] == NONE)
		{
			// If term is out of date
			if (argument_term < ((Server*)server_)->get_current_term()) {
				*result_term = ((Server*)server_)->get_current_term();
				*result_success = false;
				Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [HeartBeat::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
			}
			// If I discover current leader or new term
			else if (argument_term >= ((Server*)server_)->get_current_term()) {
				Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [HeartBeat::Accepted] Received an append_entry claiming to be leader[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

				*result_success = true;
				have_to_die_ = true;
				there_is_leader_ = true;

				// Inform server that state has changed to follower.  
				((Server*)server_)->set_new_state(StateEnum::follower_state);
				((Server*)server_)->set_current_term(argument_term);
				((Server*)server_)->set_current_leader_id(argument_leader_id);
			}
			else {
				Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
			}
		}
		// Append entry
		else
		{
			// If term is out of date
			if (argument_term < ((Server*)server_)->get_current_term()) {
				*result_term = ((Server*)server_)->get_current_term();
				*result_success = false;
				Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
			}
			// And its terms is equal or highest than mine... 
			else if (argument_term >= ((Server*)server_)->get_current_term()) {
				Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Accepted] Received an append_entry claiming to be leader[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

				*result_success = true;
				have_to_die_ = true;
				there_is_leader_ = true;

				// Inform server that state has changed to follower.  
				((Server*)server_)->set_new_state(StateEnum::follower_state);

				((Server*)server_)->set_current_term(argument_term);
				((Server*)server_)->set_current_leader_id(argument_leader_id);
			}
			else {
				Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
			}
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

	std::lock_guard<std::mutex> locker_candidate(mu_candidate_);

	// If there is not leader yet. 
	if (!there_is_leader_) {

		// If term is out of date
		if (argument_term < ((Server*)server_)->get_current_term()) {
			*result_term = ((Server*)server_)->get_current_term();
			*result_vote_granted = false;
			Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
		}
		// And its terms is equal or highest than mine... 
		else if (argument_term >= ((Server*)server_)->get_current_term()) {
			Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Accepted] received a request_vote[term:" + std::to_string(argument_term) + " >= current_term:" + std::to_string(((Server*)server_)->get_current_term()) + "]\r\n");

			*result_vote_granted = true;
			*result_term = ((Server*)server_)->get_current_term();

			// Inform server that state has changed to follower.  
			((Server*)server_)->set_new_state(StateEnum::follower_state);
			there_is_leader_ = true;
		}
		else {
			Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n");
		}
	}
	else
	{
		Tracer::trace(">>>>>[RECEVIVED](Candidate." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Rejected]received a request_vote but it already exists a Leader\r\n", SeverityTrace::error_trace);
	}
}