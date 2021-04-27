#pragma once

#include "RaftDefs.h"
#include <stdio.h>
#include <string>
#include "ClientRequest.h"
#include "RPC_API_Client.h"


#define FOLLOWER_TEXT							"FOLLOWER"
#define	LEADER_TEXT								"LEADER"
#define CANDIDATE_TEXT							"CANDIDATE"
#define SERVER_TEXT								"SERVER"


class RaftUtils
{
public:
	static std::string parse_state_to_string(StateEnum state);
	static std::string parse_from_socket_enum_to_text(ClientRequesTypeEnum type);
	static std::string parse_from_rpc_enum_to_text(RPCTypeEnum type);	
};



