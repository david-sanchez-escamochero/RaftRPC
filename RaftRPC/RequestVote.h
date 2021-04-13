#pragma once

#include <stdint.h>


class RequestVote 
{
public:
	// Arguments:
	uint32_t argument_term_;				// Candidate's term
	uint32_t argument_candidate_id_;		// Candidate requesting vote
	uint32_t argument_last_log_index_;		// Index of candidate's last log entry (§5.4)
	uint32_t argument_last_log_term_;		// Term of candidate's last log entry (§5.4)
	//Results :
	uint32_t result_term_;				// CurrentTerm, for candidate to update itself
	uint32_t result_vote_granted_;		// True means candidate received vote
};

