#pragma once

enum class RPCTypeEnum { rpc_append_entry, rpc_append_request_vote, rpc_append_heart_beat, rpc_client_request_value, rpc_client_request_leader };
enum class RPCDirection { rpc_in_invoke, rpc_out_result };


class RPC_API_Client
{
public:
	void send_append_entry(
        // Only Debug
        RPCTypeEnum	 rpc_type,
        RPCDirection rpc_direction,
        int	server_id_origin,
        int	server_id_target,
        //Arguments :
        int	argument_term,						// Leader's term
        int	argument_leader_id,				// So follower can redirect clients
        int	argument_prev_log_index,			// Index of log entry immediately preceding	new ones
        int	argument_prev_log_term,			// Term of argument_prev_log_index entry
        int	argument_entries[],	            // Log entries to store(empty for heartbeat; may send more than one for efficiency)
        int	argument_leader_commit,			// Leader’s commitIndex
        // Results :
        int* result_term,						// CurrentTerm, for leader to update itself
        int* result_success					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
    );
	void send_request_vote(
        // Only Debug
        RPCTypeEnum	 rpc_type,
        RPCDirection rpc_direction,
        int	server_id_origin,
        int	server_id_target,
        // Arguments:
        int argument_term,				// Candidate's term
        int argument_candidate_id,		// Candidate requesting vote
        int argument_last_log_index,		// Index of candidate's last log entry (§5.4)
        int argument_last_log_term,		// Term of candidate's last log entry (§5.4)
        //Results :
        int *result_term,				    // CurrentTerm, for candidate to update itself
        int *result_vote_granted		    // True means candidate received vote    
    );
};

