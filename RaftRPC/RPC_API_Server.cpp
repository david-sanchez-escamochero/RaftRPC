#include "RPC_API_Server.h"
#include "Tracer.h"
#include "Server.h"

// ServerRPC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// File Example1Server.cpp
#include <iostream>


void* server_ = nullptr;

// Server function.

RPC_API_Server::RPC_API_Server()
{    
    server_ = nullptr;
    port_receiver_ = NONE;    
}

RPC_API_Server::~RPC_API_Server()
{
    RPC_STATUS status;
    have_to_die_ = true;                    

    status = RpcMgmtStopServerListening(NULL);

    if (status)
    {
        printf("Muy chungo!!!!");
        //exit(status);
    }

    status = RpcServerUnregisterIf(rpc_server::RaftTFM_v1_0_s_ifspec, NULL, FALSE);

    if (status)
    {
        printf("Super chungo!!!!");
        //exit(status);
    }    
}

void RPC_API_Server::start(void* server, int port_receiver)
{    
    port_receiver_ = port_receiver;
    server_ = server;    
}




void rpc_server::append_entry_rpc_server(
    /* [in] */ handle_t IDL_handle,
    /* [in] */ int argument_term_,
    /* [in] */ int argument_leader_id_,
    /* [in] */ int argument_prev_log_index_,
    /* [in] */ int argument_prev_log_term_,
    /* [in] */ int argument_entries_[1000],
    /* [in] */ int argument_leader_commit_,
    /* [out] */ int* result_term_,
    /* [out] */ int* result_success_)
{
    if(server_)
        ((Server*)server_)->append_entry_server(
                            argument_term_,
                            argument_leader_id_,
                            argument_prev_log_index_,
                            argument_prev_log_term_,
                            argument_entries_,
                            argument_leader_commit_,
                            result_term_,
                            result_success_);
}


void rpc_server::request_vote_rpc_server(
    /* [in] */ handle_t IDL_handle,
    /* [in] */ int argument_term_,
    /* [in] */ int argument_candidate_id_,
    /* [in] */ int argument_last_log_index_,
    /* [in] */ int argument_last_log_term_,
    /* [out] */ int* result_term_,
    /* [out] */ int* result_vote_granted_)
{
    if (server_)
        ((Server*)server_)->request_vote_server(
                            argument_term_,
                            argument_candidate_id_,
                            argument_last_log_index_,
                            argument_last_log_term_,
                            result_term_,
                            result_vote_granted_);
}

// Naive security callback.
RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE /*hInterface*/, void* /*pBindingHandle*/)
{
    return RPC_S_OK; // Always allow anyone.
}

void RPC_API_Server::receive_rpc() 
{
    while (!have_to_die_) 
    {        
        int error = receive();
        if (error != NONE) {
            Tracer::trace("RPC_API_Server::receive_rpc error:" + std::to_string(error)+"\r\n");
            if (!have_to_die_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            }
        }
    }
}


int RPC_API_Server::receive()
{    
    printf("RPC receiving...\r\n");

    RPC_STATUS status;

    char port[6];
    sprintf_s(port, "%d", port_receiver_);
    printf("SERVER PORT:%d\r\n", port_receiver_);

    // Uses the protocol combined with the endpoint for receiving
    // remote procedure calls.
    status = RpcServerUseProtseqEpA(
        (RPC_CSTR)("ncacn_ip_tcp"), // Use TCP/IP protocol.
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // Backlog queue length for TCP/IP.
        (RPC_CSTR)(port),         // TCP/IP port to use.
        NULL);                          // No security.

    if (status)
        return status;

    // Registers the Example1 interface.
    status = RpcServerRegisterIf2(
        rpc_server::RaftTFM_v1_0_s_ifspec,              // Interface to register.
        NULL,                                // Use the MIDL generated entry-point vector.
        NULL,                                // Use the MIDL generated entry-point vector.
        RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, // Forces use of security callback.
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Use default number of concurrent calls.
        (unsigned)-1,                        // Infinite max size of incoming data blocks.
        SecurityCallback);                   // Naive security callback.

    if (status)
        return status;

    // Start to listen for remote procedure
    // calls for all registered interfaces.
    // This call will not return until
    // RpcMgmtStopServerListening is called.
    status = RpcServerListen(
        1,                                   // Recommended minimum number of threads.
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Recommended maximum number of threads.
        FALSE);                              // Start listening now.

    if (status)
        return status;

    return NO_ERROR;
}

