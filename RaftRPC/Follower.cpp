#include "Follower.h"
#include <thread>
#include "Tracer.h"
#include "ClientDefs.h"




Follower::Follower(void* server)
{	
	server_											= server;
	Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") I am a Follower\r\n", SeverityTrace::action_trace);
	have_to_die_									= false;	
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

				if ((abs(last_time_stam_taken_miliseconds_.count() - current_time_stam_taken_miliseconds.count())) > ELECTION_TIME_OUT)
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


void Follower::send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver)
{
	(((Server*)server_)->send_msg_socket(client_request, port, sender, action, receiver));
}



void Follower::dispatch_client_request_leader(ClientRequest* client_request)
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
	Tracer::trace("(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") sending who is leader(" + std::to_string(((Server*)server_)->get_current_leader_id()) + ") to client." + std::to_string(client_request->client_id_) + " .\r\n");
}

void Follower::dispatch_client_request_value(ClientRequest* client_request)
{
	// N/A	
}


void Follower::receive_msg_socket(ClientRequest* client_request)
{	
	// A client request a leader
	if (client_request->client_request_type == ClientRequesTypeEnum::client_request_leader) {
		dispatch_client_request_leader(client_request);
	}
	// A client request value
	else if (client_request->client_request_type == ClientRequesTypeEnum::client_request_value) {
		dispatch_client_request_value(client_request);
	}
	else
		Tracer::trace("Follower::dispatch - Wrong!!! type " + std::to_string(static_cast<int>(client_request->client_request_type)) + "\r\n");
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
	

	// Heart beat...(argument entries is empty.) 
	if (argument_entries[0] == NONE)
	{
		// If term is out of date
		if (argument_term < ((Server*)server_)->get_current_term()) {
			*result_term = ((Server*)server_)->get_current_term();
			*result_success = false;
			Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [HeartBeat::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
		}
		// If I discover current leader or new term
		else if (argument_term >= ((Server*)server_)->get_current_term()) {
			last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			count_check_if_there_is_candidate_or_leader_ = 0;
			Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Received append entry(Heart-beat) to Server. \r\n", SeverityTrace::action_trace);

			*result_success = true;
			((Server*)server_)->set_current_term(argument_term);
			((Server*)server_)->set_current_leader_id(argument_leader_id);
		}
		else {
			Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
		}
	}
	// Append entry
	else
	{
		// If term is out of date
		if (argument_term < ((Server*)server_)->get_current_term()) {
			*result_term = ((Server*)server_)->get_current_term();
			*result_success = false;
			Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
		}
		// And its terms is equal or highest than mine... 
		else if (argument_term >= ((Server*)server_)->get_current_term()) {			

			// Replay false if log does not contain an entry at prevLogIndex whose term matches preLogTerm($5.3)
			if ( argument_prev_log_index > ((Server*)server_)->get_log_index() ) {
				*result_term = ((Server*)server_)->get_current_term();
				*result_success = false;
				Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Rejected] argument_prev_log_index: " + std::to_string(argument_prev_log_index) + " > ((Server*)server_)->get_log_index() " + std::to_string(((Server*)server_)->get_log_index()) + "\r\n", SeverityTrace::error_trace);
			}
			// If an existing entry conflict with a new one (same index but different terms), delete the existing entry and all that follow it ($5.3)
			else if (argument_prev_log_term != ((Server*)server_)->get_term_of_entry_in_log(argument_prev_log_index))
			{
				*result_term = ((Server*)server_)->get_current_term();
				*result_success = false;
				Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Rejected] argument_prev_log_term:" + std::to_string(argument_prev_log_term) + " != get_term_of_entry_in_log(argument_prev_log_index): " + std::to_string(((Server*)server_)->get_term_of_entry_in_log(argument_prev_log_index)) + "\r\n", SeverityTrace::error_trace);
			}
			else {
				if (argument_leader_commit == ((Server*)server_)->get_commit_index()) {
					*result_success = true;
					((Server*)server_)->set_last_applied(((Server*)server_)->get_last_applied() + 1);

					Tracer::trace("¡¡¡¡¡¡¡¡(Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Applied " + std::to_string(argument_entries[0]) + " to STATE MACHINE.!!!!!!!!\r\n", SeverityTrace::action_trace);
				}
				else {
					*result_success = true;					
					((Server*)server_)->write_log(argument_entries[0]);
					((Server*)server_)->set_current_term(argument_term);
					((Server*)server_)->set_current_leader_id(argument_leader_id);
					((Server*)server_)->set_commit_index(((Server*)server_)->get_commit_index() + 1);
					Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [AppendEntry::Accepted] Accepted value\r\n", SeverityTrace::action_trace);
				}
			}
		}
		else {
			Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
		}
	}	
}


void Follower::request_vote_role(
	/* [in] */ int argument_term,
	/* [in] */ int argument_candidate_id,
	/* [in] */ int argument_last_log_index,
	/* [in] */ int argument_last_log_term,
	/* [out] */ int* result_term,
	/* [out] */ int* result_vote_granted) {
	
	
	// If term is out of date
	if (argument_term < ((Server*)server_)->get_current_term()) {
		*result_vote_granted = false;
		*result_term = ((Server*)server_)->get_current_term();
		Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Rejected] Term is out of date " + std::to_string(argument_term) + " < " + std::to_string(((Server*)server_)->get_current_term()) + "\r\n", SeverityTrace::error_trace);
	}
	// I have already voted in this term.
	else if ( (((Server*)server_)->get_voted_for() != NONE) && (argument_term == ((Server*)server_)->get_current_term()) ) {
		*result_vote_granted = false;
		*result_term = ((Server*)server_)->get_current_term();
		Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Rejected]I have already voted:" + std::to_string(((Server*)server_)->get_voted_for()) + "\r\n", SeverityTrace::error_trace);
	}
	// Candidate's term is updated...
	// CandidateID is not null. 
	// Candidate's log is at least as up-to-date receivers's log, grant vote.
	else if (
				( argument_term >= ((Server*)server_)->get_current_term() ) &&
				( argument_candidate_id != NONE )	 && 
				( argument_last_log_index >= ((Server*)server_)->get_log_index() ) &&												// Index of candidate's last log entry (§5.4)
				( argument_last_log_term >= ((Server*)server_)->get_term_of_entry_in_log(((Server*)server_)->get_log_index()) )     // Term of candidate's last log entry (§5.4)
	        )
	{
		((Server*)server_)->set_current_term(argument_term);
		((Server*)server_)->set_voted_for(argument_candidate_id);
		*result_vote_granted = true;
		last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		count_check_if_there_is_candidate_or_leader_ = 0;
		Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") [RequestVote::Accepted]Vote granted to S.:" + std::to_string(argument_candidate_id) + "\r\n", SeverityTrace::action_trace);
	}
	else {
		Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Unknown \r\n", SeverityTrace::error_trace);
	}
}