#include "Client.h"
#include <iostream>
#include <string>
#include <fstream>
#include "ClientRequest.h"
#include "ClientDefs.h"
#include "RaftDefs.h"


Client::Client()
{
    have_to_die_ = false;
    leader_      = NONE;
}

Client::~Client()
{
    have_to_die_ = true;

    if (thread_server_receive_leader_.joinable())
        thread_server_receive_leader_.join();
}

bool Client::start(std::string file_name, uint32_t client_id)
{
    bool ret    = true;
	file_name_  = file_name;	
    client_id_  = client_id;
    
    find_a_leader();

    send_request(file_name, leader_);

    return ret;
}

void Client::find_a_leader() {
    bool ret = false;
    leader_  = NONE;

    // Start server receive leader 
    thread_server_receive_leader_ = std::thread(&Client::receive, this);
    uint32_t num_server = 0;

    do {
        send_request_to_find_a_leader(num_server++);
        {
            // Time waiting servers replay saying who is the leader...
            Tracer::trace("Client - Leader does not respond, error: " + std::to_string(ret) + "\r\n", SeverityTrace::error_trace);
            Tracer::trace("Client - Waiting " + std::to_string(TIME_WAITING_A_LEADER) + "(ms) to retry...\r\n");
            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            cv_found_a_leader_.wait_for(lck, std::chrono::milliseconds(TIME_WAITING_A_LEADER));
        }

        if (num_server == NUM_SERVERS)
            num_server = 0;

    } while (leader_ == NONE);    
}

void* Client::receive()
{
    while (!have_to_die_) {
        ClientRequest client_request;
        int error = communication_.receiveMessage(&client_request, SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + client_id_, LEADER_TEXT);

        if (error) {
            Tracer::trace("Client::receive - FAILED!!!  - error" + std::to_string(error) + "\r\n");
        }
        else {                        
            if (                
                (client_request.client_request_type == ClientRequesTypeEnum::client_request_leader) &&
                (client_request.client_result_ == (uint32_t)true)                
                ) {
                if (client_request.client_leader_ == NONE) {
                    Tracer::trace("Client - I do not know yet who is the leader\r\n", SeverityTrace::action_trace);
                }
                else {
                    leader_ = client_request.client_leader_;
                    Tracer::trace("Client - I have found a Leader: " + std::to_string(leader_) + "\r\n", SeverityTrace::action_trace);
                    cv_found_a_leader_.notify_one();
                }
            } 
            else if (                
                (client_request.client_request_type == ClientRequesTypeEnum::client_request_value) &&
                (client_request.client_result_ == (uint32_t)true)
                ) {                
                cv_committed_value_.notify_one();
            }
        }
    }
    return nullptr;
}

void Client::send_request_to_find_a_leader(uint32_t num_server) {
    
    ClientRequest client_request;
    
    client_request.client_request_type = ClientRequesTypeEnum::client_request_leader;
    client_request.client_id_ = client_id_;

    int ret = NONE;
        
    ret = communication_.sendMessage(&client_request, SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + num_server, CLIENT_TEXT, CLIENT_REQUEST_LEADER_TEXT, LEADER_TEXT);
    if (ret) {
        Tracer::trace("Client - Failed to request leader from server " + std::to_string(num_server) + ", error " + std::to_string(ret) + "\r\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }    
}

bool Client::send_request(std::string file_name, uint32_t leader_id)
{
    bool ret = true;
    if (file_name_ != "") {
                        
        std::string value;

        std::ifstream infile(file_name_);

        while (std::getline(infile, value))
        {
            Tracer::trace("Client - Value proposed:" + value + "\r\n", SeverityTrace::action_trace);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            ClientRequest client_request;             
            client_request.client_request_type = ClientRequesTypeEnum::client_request_value;
            client_request.client_id_ = client_id_;
            client_request.client_value_ = atoi(value.c_str());
            cv_status time_out; 

            do {
                int ret = 0;
                do
                {
                    int ret = communication_.sendMessage(&client_request, SOCKET_BASE_PORT + SOCKET_RECEIVER_PORT + leader_id, CLIENT_TEXT, CLIENT_REQUEST_LEADER_TEXT, LEADER_TEXT);
                    if (ret) {
                        Tracer::trace("Client - Leader does not respond, error: " + std::to_string(ret) + "\r\n");
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                } while (ret);
                {

                    // Time waiting servers replay if value is commited...
                    std::mutex mtx;
                    std::unique_lock<std::mutex> lck(mtx);
                    time_out = cv_committed_value_.wait_for(lck, std::chrono::milliseconds(TIME_WAITING_COMMIT_VALUE));
                }
                if (time_out == std::cv_status::timeout) {
                    find_a_leader();
                }
            } while (time_out != std::cv_status::timeout);
        }        
    }
    else {
        Tracer::trace("Client::start - FAILED!!! file_name  == '' \r\n");
        ret = false;
    }

    return ret;
}
