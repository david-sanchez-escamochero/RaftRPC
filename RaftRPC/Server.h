#pragma once

#include <stdint.h>
#include "RaftDefs.h"
#include "Command.h"
#include "Candidate.h"
#include "Follower.h"
#include "Leader.h"
#include "IRole.h"
#include "Semaphore.h"
#include <queue>
#include "ManagerLog.h"
#include "RPC_API_Server.h"
#include "RPC_API_Client.h"




typedef struct {
	int log_index_;
	Command	 command_[MAX_LOG_ENTRIES];
}Log;

class Server
{

public: 
	Server(int server_id);
	~Server();
	void		send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver);
	void		start();
	IRole*		get_current_shape_sever(StateEnum state);
	int			get_server_id();
	void		set_new_state(StateEnum state);
	
	std::mutex	mu_server_;
	void		increment_current_term();

	// Get/set current term. 
	int			get_current_term();
	void		set_current_term(int term);

	int			get_commit_index();
	void		set_commit_index(int commit_index);
	int			get_last_applied();

	// Get/set vote for. 
	int32_t		get_voted_for();	 
	void		set_voted_for(int32_t vote_for);

	// Manager logs. 
	int			write_log(int state_machine_command);
	int			get_log_index();
	int			get_term_of_entry_in_log(int log_index);
	int			get_state_machime_command(int log_index);

	// Leader 
	int			current_leader_id_;
	int			get_current_leader_id();
	void		set_current_leader_id(int leader_id);

	void		panic();


	// RPC
	void receive_rpc();
	int send_append_entry_rpc(
		// Only Debug
		RPCTypeEnum	 rpc_type,
		RPCDirection rpc_direction,
		int	server_id_origin,
		int	server_id_target,
		int port_target,
		//Arguments :
		int	argument_term,					// Leader's term
		int	argument_leader_id,				// So follower can redirect clients
		int	argument_prev_log_index,		// Index of log entry immediately preceding	new ones
		int	argument_prev_log_term,			// Term of argument_prev_log_index entry
		int	argument_entries[],	            // Log entries to store(empty for heartbeat; may send more than one for efficiency)
		int	argument_leader_commit,			// Leader’s commitIndex
		// Results :
		int* result_term,					// CurrentTerm, for leader to update itself
		int* result_success					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
	);
	int send_request_vote_rpc(
		// Only Debug
		RPCTypeEnum	 rpc_type,
		RPCDirection rpc_direction,
		int	server_id_origin,
		int	server_id_target,
		int port_target,
		// Arguments:
		int argument_term,					// Candidate's term
		int argument_candidate_id,			// Candidate requesting vote
		int argument_last_log_index,		// Index of candidate's last log entry (§5.4)
		int argument_last_log_term,			// Term of candidate's last log entry (§5.4)
		//Results :
		int* result_term,				    // CurrentTerm, for candidate to update itself
		int* result_vote_granted		    // True means candidate received vote    
	);

	void append_entry_server(
		/* [in] */ int argument_term,
		/* [in] */ int argument_leader_id,
		/* [in] */ int argument_prev_log_index,
		/* [in] */ int argument_prev_log_term,
		/* [in] */ int argument_entries[],
		/* [in] */ int argument_leader_commit,
		/* [out] */ int* result_term,
		/* [out] */ int* result_success);


	void request_vote_server(
		/* [in] */ int argument_term,
		/* [in] */ int argument_candidate_id,
		/* [in] */ int argument_last_log_index,
		/* [in] */ int argument_last_log_term,
		/* [out] */ int* result_term,
		/* [out] */ int* result_vote_granted);

protected:

	StateEnum					current_state_;
	StateEnum					new_state_;
	std::thread					thread_dispatch_msg_socket_;
	std::thread					thread_receive_msg_socket_;
	std::thread					thread_check_new_state_;

	// Persisten state on all servers. 
	int							current_term_;			// Latest term server has seen (initialized to 0 on first boot, increases monotonically)
	int32_t						voted_for_;				// CandidateId that received vote in current term(or null if none)
	Log							log_;					// Log entries; each entry contains command for state machine, and term when entry was received by leader(first index is 1)

	// Volatile state on all servers. 
	int							commit_index_;			// Index of highest log entry known to be committed(initialized to 0, increases	monotonically)
	int							last_applied_;			// Index of highest log entry applied to state	machine(initialized to 0, increases	monotonically)	

	
	
private: 
	
	IRole*						connector_;
	int							server_id_;
	Communication				communication_;
	bool						have_to_die_;	
	void						dispatch_msg_socket();
	void						receive_msg_socket();
	Semaphore					semaphore_dispatch_;
	Semaphore					semaphore_new_state_;
	queue<ClientRequest>		queue_;
	void						check_new_state();
	std::mutex					mu_new_state_;
	ManagerLog					manager_log_;
	std::string					file_log_name_;	
	RPC_API_Server				rpc_api_server_;
	RPC_API_Client				rpc_api_client_;
};

