#pragma once

#include "RaftDefs.h"
#include <stdint.h>
#include "IRole.h"
#include "Server.h"
#include <mutex>
#include "RPC_API_Server.h"
#include "RPC_API_Client.h"



class Leader : public IRole
{


public: 
	Leader(void* server);
	~Leader();

	void send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver);
	void receive_msg_socket(ClientRequest* client_request);
	void start();
	

protected: 
	//  Volatile state on leaders 
	// (Reinitialized after election)
	int						next_index_[NUM_SERVERS];	// For each server, index of the next log entry	to send to that server(initialized to leader last log index + 1)
	int						match_index_[NUM_SERVERS];	// For each server, index of highest log entry known to be replicated on server	(initialized to 0, increases monotonically)
	void*					server_;
	std::mutex				mu_leader_;
	bool					have_to_die_;	
	ClientRequest		    client_request_;
	

	std::thread				thread_send_heart_beat_all_servers_;	
	std::thread				thread_send_append_entry_1th_phase_;
	std::thread				thread_send_append_entry_2nd_phase_;
	bool					threads_have_to_die_;
	void					send_heart_beat_all_servers();	
	void					send_append_entry_1th_phase();
	//void					send_append_entry_2nd_phase();
	void					send_append_entry_2nd_phase(int index, int server_to_apply_state_machine);
	std::condition_variable cv_send_heart_beat_all_servers_;


	// Dispatch
	void					dispatch_client_request_leader(ClientRequest* client_request);
	void					dispatch_client_request_value(ClientRequest* client_request);
	void					dispatch_client_ping_master(ClientRequest* client_request);
	
	// Master
	int						master_;
	long					deadline_master_;
	


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

