#pragma once

#include "IRole.h"
#include "RaftTFM_rpc_server.h"          
#include <thread>



class RPC_API_Server
{

public:
	RPC_API_Server();
	~RPC_API_Server();
	void start(IRole* role, int port_receiver);	
	int	 receive();
	void receive_rpc();

protected:	
	int			port_receiver_;	
	bool		have_to_die_;
};

