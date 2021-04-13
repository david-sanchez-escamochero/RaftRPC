#pragma once

#include "RPC_sockets.h"
#include <string>


class IRole {

public:
	virtual void start()																							= 0;
	virtual void send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver)	= 0;
	virtual void receive(RPC_sockets* rpc)																					= 0;
	virtual ~IRole() {}
	
};
