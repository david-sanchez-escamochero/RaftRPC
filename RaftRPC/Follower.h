#pragma once


#include "IRole.h"
#include "ClientRequest.h"
#include "Communication.h"
#include "Server.h"
#include <queue>
#include <chrono>
#include <mutex>
#include "RPC_API_Client.h"

using namespace std::chrono;

class Follower : public IRole
{
public:
	Follower(void* server);
	~Follower();
	void send(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver);
	void receive(ClientRequest* client_request);
	void start();

protected:
	Communication	communication_;
	ClientRequest*  client_request_;
	void*			server_;	
	bool			have_to_die_;
	std::queue<ClientRequest> queue_;
	
	
	void			check_if_there_is_candidate_or_leader();	
	milliseconds	last_time_stam_taken_miliseconds_;
	std::mutex		mu_follower_;
	int				count_check_if_there_is_candidate_or_leader_;
	std::thread		thread_check_candidate_;


	// RPC 	
	void append_entry_role(
		/* [in] */ int argument_term,
		/* [in] */ int argument_leader_id,
		/* [in] */ int argument_prev_log_index,
		/* [in] */ int argument_prev_log_term,
		/* [in] */ int argument_entries[1000],
		/* [in] */ int argument_leader_commit,
		/* [out] */ int* result_term,
		/* [out] */ int* result_success);


	void request_vote_role(
		/* [in] */ int argument_term,
		/* [in] */ int argument_candidate_id,
		/* [in] */ int argument_last_log_index,
		/* [in] */ int argument_last_log_term,
		/* [out] */ int* result_term,
		/* [out] */ int* result_vote_granted);


};

