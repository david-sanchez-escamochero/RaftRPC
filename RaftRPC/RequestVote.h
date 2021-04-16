#pragma once

#include <stdint.h>


class RequestVote 
{
public:
	// Arguments:
	int argument_term_;				// Candidate's term
	int argument_candidate_id_;		// Candidate requesting vote
	int argument_last_log_index_;		// Index of candidate's last log entry (§5.4)
	int argument_last_log_term_;		// Term of candidate's last log entry (§5.4)
	//Results :
	int result_term_;				// CurrentTerm, for candidate to update itself
	int result_vote_granted_;		// True means candidate received vote
};

