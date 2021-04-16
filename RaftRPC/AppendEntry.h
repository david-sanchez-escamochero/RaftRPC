#pragma once

#include <stdint.h>
#include "RaftDefs.h"
#include <string>


class AppendEntry 
{
public:
	//Arguments :
	int	argument_term_;						// Leader's term
	int	argument_leader_id_;				// So follower can redirect clients
	int	argument_prev_log_index_;			// Index of log entry immediately preceding	new ones
	int	argument_prev_log_term_;			// Term of argument_prev_log_index entry
	int	argument_entries_[MAX_LOG_ENTRIES];	// Log entries to store(empty for heartbeat; may send more than one for efficiency)
	int	argument_leader_commit_;			// Leader’s commitIndex
	// Results :
	int	result_term_;						// CurrentTerm, for leader to update itself
	int	result_success_;					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
};

