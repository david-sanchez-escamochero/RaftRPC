#pragma once


enum class StateEnum { follower_state, leader_state, candidate_state, unknown_state };


#define NUM_SERVERS												3
#define MAX_LOG_ENTRIES											50
#define BASE_PORT												6000
#define SENDER_PORT												100
#define RECEIVER_PORT											200
#define MAJORITY												( ( NUM_SERVERS % 2 ) + 1 ) - 1  // -1 Because I do not have to send message to myself.
#define ELECTION_TIME_OUT										3//(s) 	A follower receives no communication over a period of time. 	
#define	MINIMUM_VALUE_RAMDOM_TIME_OUT							150//(ms)
#define	TIME_OUT_CHECK_IF_THERE_IS_CANDIDATE_OR_LEADER			15000//(ms)
#define	TIME_OUT_IF_THERE_IS_CANDIDATE_OR_LEADER				1000//(ms)
#define	TIME_OUT_TERM											10000//(ms)
#define TIME_OUT_HEART_BEAT										2000//(ms)
#define TIME_OUT_LEADER_TERM									60000//(ms)
#define TIME_OUT_WAIT											2000//(ms)


#define FOLLOWER_TEXT											"FOLLOWER"
#define	LEADER_TEXT												"LEADER"
#define CANDIDATE_TEXT											"CANDIDATE"
#define SERVER_TEXT												"SERVER"
#define UNKNOWN_TEXT											"UNKNOWN"

#define REQUEST_VOTE_TEXT										"REQUEST_VOTE"
#define APPEND_ENTRY_TEXT										"APPEND_ENTRY"
#define HEART_BEAT_TEXT											"HEART_BEAT"
#define INVOKE_TEXT												"INVOKE"
#define RESULT_TEXT												"RESULT"


