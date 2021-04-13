#pragma once

#include <stdint.h>
#include <string>

class ClientRequest
{
public:
	// Arguments:
	uint32_t client_id_;
	uint32_t client_value_;
	// Results: 
	uint32_t client_result_;
	uint32_t client_leader_;
};



