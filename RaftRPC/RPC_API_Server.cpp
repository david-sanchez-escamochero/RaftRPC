#include "RPC_API_Server.h"


// ServerRPC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// File Example1Server.cpp
#include <iostream>


IRole* role_ = nullptr;

// Server function.

RPC_API_Server::RPC_API_Server()
{    
    role_ = nullptr;
    port_receiver_ = NONE;
}

RPC_API_Server::~RPC_API_Server()
{
    RPC_STATUS status;

    status = RpcMgmtStopServerListening(NULL);

    if (status)
    {
        exit(status);
    }

    status = RpcServerUnregisterIf(NULL, NULL, FALSE);

    if (status)
    {
        exit(status);
    }

    if (thread_receive_.joinable())
        thread_receive_.join();
}

void RPC_API_Server::start(IRole* role, int port_receiver)
{    
    port_receiver_ = port_receiver;
    role_ = role;
    thread_receive_ = std::thread(&RPC_API_Server::receive, this);
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
    if(role_)
        role_->append_entry_role(
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
    if (role_)
        role_->request_vote_role(
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


int RPC_API_Server::receive()
{
    printf("RPC receiving...\r\n");

    RPC_STATUS status;

    char port[6];
    sprintf_s(port, "%d", port_receiver_);


    // Uses the protocol combined with the endpoint for receiving
    // remote procedure calls.
    status = RpcServerUseProtseqEpA(
        (RPC_CSTR)("ncacn_ip_tcp"), // Use TCP/IP protocol.
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // Backlog queue length for TCP/IP.
        (RPC_CSTR)(port),         // TCP/IP port to use.
        NULL);                          // No security.

    if (status)
        exit(status);

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
        exit(status);

    // Start to listen for remote procedure
    // calls for all registered interfaces.
    // This call will not return until
    // RpcMgmtStopServerListening is called.
    status = RpcServerListen(
        1,                                   // Recommended minimum number of threads.
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Recommended maximum number of threads.
        FALSE);                              // Start listening now.

    if (status)
        exit(status);
}

