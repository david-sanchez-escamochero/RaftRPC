#pragma once

#include "RaftDefs.h"
#include <stdio.h>
#include <string>
#include "RPC_sockets.h"


#define FOLLOWER_TEXT							"FOLLOWER"
#define	LEADER_TEXT								"LEADER"
#define CANDIDATE_TEXT							"CANDIDATE"
#define SERVER_TEXT								"SERVER"


class RaftUtils
{
public:
	static std::string parse_state_to_string(StateEnum state);
	static std::string parse_from_rcp_enum_to_text(RPCTypeEnum_sockets type);
};



