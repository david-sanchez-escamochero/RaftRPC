#include "RPC_API_Client.h"
#include <iostream>
#include "RaftTFM_rpc_client.h"
#include <string>

void RPC_API_Client::send_append_entry(
    // Only Debug
    RPCTypeEnum	 rpc_type,
    RPCDirection rpc_direction,
    int	server_id_origin,
    int	server_id_target,
    //Arguments :
    int	argument_term_,						// Leader's term
    int	argument_leader_id_,				// So follower can redirect clients
    int	argument_prev_log_index_,			// Index of log entry immediately preceding	new ones
    int	argument_prev_log_term_,			// Term of argument_prev_log_index entry
    int	argument_entries_[],	            // Log entries to store(empty for heartbeat; may send more than one for efficiency)
    int	argument_leader_commit_,			// Leader’s commitIndex
    // Results :
    int* result_term_,						// CurrentTerm, for leader to update itself
    int* result_success_					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
)

{
    RPC_STATUS status;
    RPC_CSTR szStringBinding = NULL;
    
    char port[6];    
    sprintf_s(port, "%d", server_id_target);    
    
    // Creates a string binding handle.
    // This function is nothing more than a printf.
    // Connection is not done here.
    
        status = RpcStringBindingComposeA(
            NULL, // UUID to bind to.
            (RPC_CSTR)("ncacn_ip_tcp"), // Use TCP/IP
                                                              // protocol.
            (RPC_CSTR)("localhost"), // TCP/IP network
                                                           // address to use.            
            (RPC_CSTR)(port) , // TCP/IP port to use.
            NULL, // Protocol dependent network options to use.
            &szStringBinding); // String binding output.



        if (status)
            exit(status);


        handle_t hRaftTFMExplicitBinding = NULL;


        // Validates the format of the string binding handle and converts
        // it to a binding handle.
        // Connection is not done here either.
        status = RpcBindingFromStringBindingA(
            szStringBinding,    // The string binding to validate.
            &hRaftTFMExplicitBinding);         // Put the result in the implicit binding
                                // handle defined in the IDL file.

        if (status)
            exit(status);

        RpcTryExcept
        {
            // Calls the RPC function. The hExample1Binding binding handle
            // is used implicitly.
            // Connection is done here.
            
            rpc_client::append_entry_rpc(hRaftTFMExplicitBinding, argument_term_, argument_leader_id_, argument_prev_log_index_, argument_prev_log_term_, argument_entries_, argument_leader_commit_, result_term_, result_success_);
        }
            RpcExcept(1)
        {
            std::cerr << "Runtime reported exception " << RpcExceptionCode()
                << std::endl;
        }
        RpcEndExcept

            // Free the memory allocated by a string.
            status = RpcStringFreeA(
                &szStringBinding); // String to be freed.

        if (status)
            exit(status);

        // Releases binding handle resources and disconnects from the server.
        status = RpcBindingFree(
            &hRaftTFMExplicitBinding); // Frees the implicit binding handle defined in
                                // the IDL file.

        if (status)
            exit(status);

}


// Memory allocation function for RPC.
// The runtime uses these two functions for allocating/deallocating
// enough memory to pass the string to the server.
void* __RPC_USER midl_user_allocate(size_t size)
{
    return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
    free(p);
}


void RPC_API_Client::send_request_vote(
    // Only Debug
    RPCTypeEnum	 rpc_type,
    RPCDirection rpc_direction,
    int	server_id_origin,
    int	server_id_target,
    // Arguments:
    int argument_term,				// Candidate's term
    int argument_candidate_id,		// Candidate requesting vote
    int argument_last_log_index,		// Index of candidate's last log entry (§5.4)
    int argument_last_log_term,		// Term of candidate's last log entry (§5.4)
    //Results :
    int *result_term,				    // CurrentTerm, for candidate to update itself
    int *result_vote_granted		    // True means candidate received vote    
)
{
    RPC_STATUS status;
    RPC_CSTR szStringBinding = NULL;

    char port[6];
    sprintf_s(port, "%d", server_id_target);

    // Creates a string binding handle.
    // This function is nothing more than a printf.
    // Connection is not done here.

    status = RpcStringBindingComposeA(
        NULL, // UUID to bind to.
        (RPC_CSTR)("ncacn_ip_tcp"), // Use TCP/IP
                                                          // protocol.
        (RPC_CSTR)("localhost"), // TCP/IP network
                                                       // address to use.            
        (RPC_CSTR)(port), // TCP/IP port to use.
        NULL, // Protocol dependent network options to use.
        &szStringBinding); // String binding output.



    if (status)
        exit(status);


    handle_t hRaftTFMExplicitBinding = NULL;


    // Validates the format of the string binding handle and converts
    // it to a binding handle.
    // Connection is not done here either.
    status = RpcBindingFromStringBindingA(
        szStringBinding,    // The string binding to validate.
        &hRaftTFMExplicitBinding);         // Put the result in the implicit binding
                            // handle defined in the IDL file.

    if (status)
        exit(status);

    RpcTryExcept
    {
        // Calls the RPC function. The hExample1Binding binding handle
        // is used implicitly.
        // Connection is done here.

       rpc_client::request_vote_rpc(hRaftTFMExplicitBinding, argument_term,	argument_candidate_id, argument_last_log_index,	argument_last_log_term, result_term, result_vote_granted);

    }
        RpcExcept(1)
    {
        std::cerr << "Runtime reported exception " << RpcExceptionCode()
            << std::endl;
    }
    RpcEndExcept

        // Free the memory allocated by a string.
        status = RpcStringFreeA(
            &szStringBinding); // String to be freed.

    if (status)
        exit(status);

    // Releases binding handle resources and disconnects from the server.
    status = RpcBindingFree(
        &hRaftTFMExplicitBinding); // Frees the implicit binding handle defined in
                            // the IDL file.

    if (status)
        exit(status);
}


