#pragma once

#include <stdint.h>
#include <string>


enum class ClientRequesTypeEnum { client_request_value, client_request_leader };

class ClientRequest
{
public:
	// Arguments:
	ClientRequesTypeEnum	client_request_type;	
	int						client_id_;
	int						client_value_;
	// Results: 
	int						client_result_;
	int						client_leader_;
};



