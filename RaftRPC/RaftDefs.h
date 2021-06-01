#pragma once


enum class StateEnum { follower_state, leader_state, candidate_state, unknown_state };


#define NUM_SERVERS												3
#define MAX_LOG_ENTRIES											10000
#define RPC_BASE_PORT											6000
#define RPC_SENDER_PORT											100
#define RPC_RECEIVER_PORT										200
#define SOCKET_BASE_PORT										7000
#define SOCKET_SENDER_PORT										100
#define SOCKET_RECEIVER_PORT									200

#define MAJORITY												( ( NUM_SERVERS / 2 ) + 1 ) - 1  // -1 Because I do not have to count on myself
#define ELECTION_TIME_OUT										15000//(ms) 	A follower receives no communication over a period of time. 	
#define	MINIMUM_VALUE_RAMDOM_TIME_OUT							2000//(150-300)//(ms)

#define	TIME_OUT_IF_THERE_IS_CANDIDATE_OR_LEADER				1000//(ms)
#define	TIME_OUT_TERM											300000//(ms)
#define TIME_OUT_HEART_BEAT										5000//(ms)
//#define TIME_OUT_LEADER_TERM									60000//(ms)
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


#define NONE													-1


#define TIME_OUT_MASTER											40//(s)


