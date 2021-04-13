#pragma once

#include <stdint.h>
#include "RaftDefs.h"
#include <string>


class AppendEntry 
{
public:
	//Arguments :
	uint32_t	argument_term_;						// Leader's term
	uint32_t	argument_leader_id_;				// So follower can redirect clients
	uint32_t	argument_prev_log_index_;			// Index of log entry immediately preceding	new ones
	uint32_t	argument_prev_log_term_;			// Term of argument_prev_log_index entry
	uint32_t	argument_entries_[MAX_LOG_ENTRIES];	// Log entries to store(empty for heartbeat; may send more than one for efficiency)
	uint32_t	argument_leader_commit_;			// Leader’s commitIndex
	// Results :
	uint32_t	result_term_;						// CurrentTerm, for leader to update itself
	uint32_t	result_success_;					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
};

