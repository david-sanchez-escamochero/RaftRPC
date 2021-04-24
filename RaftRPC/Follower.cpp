#include "Follower.h"
#include <thread>
#include "Tracer.h"




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

void Follower::receive_msg_socket(ClientRequest* client_request)
{	
	//dispatch(rpc);
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
	if (argument_entries[0] == NONE) {
		last_time_stam_taken_miliseconds_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		count_check_if_there_is_candidate_or_leader_ = 0;		
		Tracer::trace(">>>>>[RECEVIVED](Follower." + std::to_string(((Server*)server_)->get_server_id()) + ") Received append entry(Heart-beat) to Server. \r\n", SeverityTrace::action_trace);
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
		(argument_term >= ((Server*)server_)->get_current_term()) &&
		(argument_candidate_id != NONE)							 //&& 
		//(rpc->request_vote.argument_last_log_index_ == 0)							 && TODO: ?¿?¿?¿?¿?¿?¿?¿?¿?¿?
		//(rpc->request_vote.argument_last_log_term_ == 0)								TODO: ?¿?¿?¿?¿?¿?¿?¿?¿?¿?¿
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