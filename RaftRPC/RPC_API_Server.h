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

protected:
	std::thread	thread_receive_;
	int			port_receiver_;
};

