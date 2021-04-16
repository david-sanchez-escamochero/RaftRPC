#pragma once

#include <stdint.h>
#include <string>

class ClientRequest
{
public:
	// Arguments:
	int client_id_;
	int client_value_;
	// Results: 
	int client_result_;
	int client_leader_;
};



