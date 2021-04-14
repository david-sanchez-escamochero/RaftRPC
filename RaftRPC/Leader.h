#pragma once

#include "RaftDefs.h"
#include <stdint.h>
#include "IRole.h"
#include "Server.h"
#include <mutex>


class Leader : public IRole
{


public: 
	Leader(void* server);
	~Leader();

	void send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver);
	void receive(RPC_sockets* rpc);
	void start();
	

protected: 
	//  Volatile state on leaders 
	// (Reinitialized after election)
	uint32_t				next_index_[NUM_SERVERS];	// For each server, index of the next log entry	to send to that server(initialized to leader last log index + 1)
	uint32_t				match_index_[NUM_SERVERS];	// For each server, index of highest log entry known to be replicated on server	(initialized to 0, increases monotonically)
	void*					server_;
	std::mutex				mu_leader_;
	bool					have_to_die_;
	bool					term_is_not_timeout_;

	// Dispatchers
	void					dispatch(RPC_sockets* rpc);
	void					dispatch_append_entry(RPC_sockets* rpc);
	void					dispatch_request_vote(RPC_sockets* rpc);
	void					dispatch_append_heart_beat(RPC_sockets* rpc);
	void					dispatch_client_request_leader(RPC_sockets* rpc);
	void					dispatch_client_request_value(RPC_sockets* rpc);


	std::thread				thread_send_heart_beat_all_servers_;
	std::thread				thread_check_leader_time_out_to_change_term_;
	void					send_heart_beat_all_servers();
	void					check_leader_time_out_to_change_term();
	std::condition_variable cv_send_heart_beat_all_servers_;
	

	void append_entry_role(
		/* [in] */ int argument_term_,
		/* [in] */ int argument_leader_id_,
		/* [in] */ int argument_prev_log_index_,
		/* [in] */ int argument_prev_log_term_,
		/* [in] */ int argument_entries_[1000],
		/* [in] */ int argument_leader_commit_,
		/* [out] */ int* result_term_,
		/* [out] */ int* result_success_);


	void request_vote_role(
		/* [in] */ int argument_term_,
		/* [in] */ int argument_candidate_id_,
		/* [in] */ int argument_last_log_index_,
		/* [in] */ int argument_last_log_term_,
		/* [out] */ int* result_term_,
		/* [out] */ int* result_vote_granted_);
};

