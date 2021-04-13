#pragma once

enum class RPCTypeEnum_sockets { rpc_append_entry, rpc_append_request_vote, rpc_append_heart_beat, rpc_client_request_value, rpc_client_request_leader};
enum class RPCDirection_sockets { rpc_in_invoke, rpc_out_result};

#include "AppendEntry.h"
#include "RequestVote.h"
#include "ClientRequest.h"
#include "ClientDefs.h"




class RPC_sockets
{
public:
	RPC_sockets();

	RPCTypeEnum_sockets	 rpc_type; 
	RPCDirection_sockets rpc_direction;
	uint32_t	 server_id_origin;
	uint32_t	 server_id_target;

	//union {
		AppendEntry append_entry;
		RequestVote request_vote;
		ClientRequest client_request;
	//};	
};


