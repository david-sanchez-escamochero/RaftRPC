#pragma once

#include "IRole.h"
#include "Server.h"
#include <string>
#include <mutex>
#include "RPC_API_Server.h"
#include "RPC_API_Client.h"
#include <chrono>

using namespace std::chrono;

class Candidate : public IRole
{
public:
	Candidate(void *server);
	~Candidate();
	void send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver);
	void receive(RPC_sockets* rpc);
	void start();

	
protected:
	void*			server_;
	void			send_request_vote_to_all_servers();
	bool			there_is_leader_;
	bool			have_to_die_;
	void			reset_receive_votes();
	int		count_received_votes_;
	std::mutex		mu_candidate_;
	bool			received_votes_[NUM_SERVERS];
	milliseconds	last_time_stam_taken_miliseconds_;
	bool			term_finished_;


	std::thread thread_send_request_vote_to_all_servers_;

	// Dispatchers.
	void dispatch(RPC_sockets *rpc);
	void dispatch_append_entry(RPC_sockets* rpc);
	void dispatch_request_vote(RPC_sockets* rpc);
	void dispatch_append_heart_beat(RPC_sockets* rpc);
	void dispatch_client_request_leader(RPC_sockets* rpc);
	void dispatch_client_request_value(RPC_sockets* rpc);



	


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

