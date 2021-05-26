#pragma once

#include <stdint.h>
#include <string>


enum class ClientRequesTypeEnum { client_write_master, client_request_leader, client_ping_master };

class ClientRequest
{
public:
	// Arguments:
	ClientRequesTypeEnum	client_request_type;	
	int						client_id_;
	int						client_value_;
	char					client_ip_[16];
	long					client_seconds_january_1_1970;
	// Results: 
	int						client_result_;
	int						client_leader_;
	int						client_master_;
};



