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





typedef struct {
	int log_index_;
	Command	 log_[MAX_LOG_ENTRIES];
}Log;

class Server
{

public: 
	Server(int server_id);
	~Server();
	void		send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver);
	void*		receive();
	void		start();
	IRole* get_current_shape_sever(StateEnum state);
	int    get_server_id();
	void		set_new_state(StateEnum state);
	std::mutex	mu_server_;
	void		increment_current_term();

	// Get/set current term. 
	int	get_current_term();
	void		set_current_term(int term);

	int	get_commit_index();
	int	get_last_applied();

	// Get/set vote for. 
	int32_t		get_voted_for();	 
	void		set_voted_for(int32_t vote_for);

	// Manager logs. 
	int	write_log(int state_machine_command);
	int	get_log_index();
	int	get_term_of_entry_in_log(int log_index);

	// Leader 
	int	current_leader_id_;
	int	get_current_leader_id();
	void		set_current_leader_id(int leader_id);


protected:

	StateEnum	current_state_;
	StateEnum	new_state_;
	std::thread thread_server_dispatch_;
	std::thread thread_check_new_state_;


	// Persisten state on all servers. 
	int	current_term_;			// Latest term server has seen (initialized to 0 on first boot, increases monotonically)
	int32_t		voted_for_;				// CandidateId that received vote in current term(or null if none)
	Log			log_;					// Log entries; each entry contains command for state machine, and term when entry was received by leader(first index is 1)

	// Volatile state on all servers. 
	int	commit_index_;			// Index of highest log entry known to be committed(initialized to 0, increases	monotonically)
	int	last_applied_;			// Index of highest log entry applied to state	machine(initialized to 0, increases	monotonically)
	
	
private: 
	
	IRole*			connector_;
	int		server_id_;
	Communication	communication_;
	bool			have_to_die_;	
	void			dispatch();
	Semaphore		semaphore_dispatch_;
	Semaphore		semaphore_new_state_;
	queue<RPC_sockets>		queue_;
	void			check_new_state();
	std::mutex		mu_new_state_;
	ManagerLog		manager_log_;
	std::string		file_log_name_;

	void			keep_alive();
};

