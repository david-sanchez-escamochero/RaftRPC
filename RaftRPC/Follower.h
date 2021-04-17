#pragma once


#include "IRole.h"
#include "RPC_sockets.h"
#include "Communication.h"
#include "Server.h"
#include <queue>
#include <chrono>
#include <mutex>
#include "RPC_API_Server.h"
#include "RPC_API_Client.h"

using namespace std::chrono;

class Follower : public IRole
{
public:
	Follower(void* server);
	~Follower();
	void send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver);
	void receive(RPC_sockets *rpc);
	void start();

protected:
	Communication	communication_;
	RPC_sockets				rpc_;
	void*			server_;	
	bool			have_to_die_;
	std::queue<RPC_sockets> queue_;
	
	
	void			check_if_there_is_candidate_or_leader();
	int		receiving_heartbeats_;		
	milliseconds	last_time_stam_taken_miliseconds_;
	std::mutex		mu_follower_;
	int		count_check_if_there_is_candidate_or_leader_;
	std::thread		thread_check_candidate_;


	// Dispatchers 
	void			dispatch(RPC_sockets* rpc);
	void			dispatch_append_entry(RPC_sockets* rpc);
	//void			dispatch_request_vote(RPC_sockets* rpc);
	void			dispatch_append_heart_beat(RPC_sockets* rpc);
	void			dispatch_client_request_leader(RPC_sockets* rpc);
	void			dispatch_client_request_value(RPC_sockets* rpc);



	// RPC 
	RPC_API_Client	rpc_api_client_;
	RPC_API_Server  rpc_api_server_;
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

