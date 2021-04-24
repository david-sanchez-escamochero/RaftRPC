#pragma once

#include <string>

#include "Tracer.h"
#include <thread>
#include "Semaphore.h"
#include "Communication.h"

class Client
{
public:
	Client();
	~Client();
	bool					start(std::string file_name, uint32_t client_id);
private:
	void					send_request_to_find_a_leader(uint32_t num_server);
	bool					send_request(std::string file_name, uint32_t leader_id);
	void*					receive();
	bool					have_to_die_;
	Communication			communication_;
	std::string				file_name_;
	int32_t					leader_;
	std::thread				thread_server_receive_leader_;
	Semaphore				semaphore_wait_reveive_leader_;
	Semaphore				semaphore_wait_commit_value_;
	void					find_a_leader();
	std::condition_variable cv_found_a_leader_;
	std::condition_variable cv_committed_value_;
	uint32_t				client_id_;
};

