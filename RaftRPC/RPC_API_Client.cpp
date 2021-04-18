#include "RPC_API_Client.h"
#include <iostream>
#include "RaftTFM_rpc_client.h"
#include <string>
#include "Tracer.h"




int RPC_API_Client::send_append_entry_rpc(RPCTypeEnum rpc_type, RPCDirection rpc_direction, int server_id_origin, int server_id_target, int port_target, int argument_term, int argument_leader_id, int argument_prev_log_index, int argument_prev_log_term, int argument_entries[], int argument_leader_commit, int* result_term, int* result_success)
{
    //Just for debugging(Because if I do not do this, it does not complile.¿?
    Tracer::trace(">>>>>[SEND] RPC(append_entry) from S." + std::to_string(server_id_origin) + " to S." + std::to_string(server_id_target) + "[port:" + std::to_string(port_target) + "]\r\n");
    return send_append_entry(
        // Only Debug
        rpc_type,
        rpc_direction,
        server_id_origin,
        server_id_target,
        port_target,
        //Arguments :
        argument_term,						// Leader's term
        argument_leader_id,				// So follower can redirect clients
        argument_prev_log_index,			// Index of log entry immediately preceding	new ones
        argument_prev_log_term,			// Term of argument_prev_log_index entry
        argument_entries,	            // Log entries to store(empty for heartbeat; may send more than one for efficiency)
        argument_leader_commit,			// Leader’s commitIndex
        // Results :
        result_term,						// CurrentTerm, for leader to update itself
        result_success					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
    );
}
int RPC_API_Client::send_request_vote_rpc(RPCTypeEnum rpc_type, RPCDirection rpc_direction, int server_id_origin, int server_id_target, int port_target, int argument_term, int argument_candidate_id, int argument_last_log_index, int argument_last_log_term, int* result_term, int* result_vote_granted)
{
    Tracer::trace(">>>>>[SEND] RPC(request_vote) from S." + std::to_string(server_id_origin) + " to S." + std::to_string(server_id_target) + "[port:" + std::to_string(port_target) + "]\r\n");
    return send_request_vote(
        // Only Debug
        rpc_type,
        rpc_direction,
        server_id_origin,
        server_id_target,
        port_target,
        // Arguments:
        argument_term,				// Candidate's term
        argument_candidate_id,		// Candidate requesting vote
        argument_last_log_index,		// Index of candidate's last log entry (§5.4)
        argument_last_log_term,		// Term of candidate's last log entry (§5.4)
        //Results :
        result_term,				    // CurrentTerm, for candidate to update itself
        result_vote_granted		    // True means candidate received vote    
    );
}



int RPC_API_Client::send_append_entry(
    // Only Debug
    RPCTypeEnum	 rpc_type,
    RPCDirection rpc_direction,
    int	server_id_origin,
    int	server_id_target,
    int port_target,
    //Arguments :
    int	argument_term,						// Leader's term
    int	argument_leader_id,				// So follower can redirect clients
    int	argument_prev_log_index,			// Index of log entry immediately preceding	new ones
    int	argument_prev_log_term,			// Term of argument_prev_log_index entry
    int	argument_entries[],	            // Log entries to store(empty for heartbeat; may send more than one for efficiency)
    int	argument_leader_commit,			// Leader’s commitIndex
    // Results :
    int* result_term,						// CurrentTerm, for leader to update itself
    int* result_success					// True if follower contained entry matching argument_prev_log_index and argument_prev_log_term
)

{      
    RPC_STATUS status;
    RPC_CSTR szStringBinding = NULL;
    
    char port[6];    
    sprintf_s(port, "%d", port_target);    

    printf("CLIENT PORT:%d\r\n", port_target);
    
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
            return status;


        handle_t hRaftTFMExplicitBinding = NULL;


        // Validates the format of the string binding handle and converts
        // it to a binding handle.
        // Connection is not done here either.
        status = RpcBindingFromStringBindingA(
            szStringBinding,    // The string binding to validate.
            &hRaftTFMExplicitBinding);         // Put the result in the implicit binding
                                // handle defined in the IDL file.

        if (status)
            return status;

        RpcTryExcept
        {
            // Calls the RPC function. The hExample1Binding binding handle
            // is used implicitly.
            // Connection is done here.
            
            rpc_client::append_entry_rpc(hRaftTFMExplicitBinding, argument_term, argument_leader_id, argument_prev_log_index, argument_prev_log_term, argument_entries, argument_leader_commit, result_term, result_success);
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
            return status;

        // Releases binding handle resources and disconnects from the server.
        status = RpcBindingFree(
            &hRaftTFMExplicitBinding); // Frees the implicit binding handle defined in
                                // the IDL file.

        if (status)
            return status;

        return NO_ERROR;
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


int RPC_API_Client::send_request_vote(
    // Only Debug
    RPCTypeEnum	 rpc_type,
    RPCDirection rpc_direction,
    int	server_id_origin,
    int	server_id_target,
    int port_target,
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
    sprintf_s(port, "%d", port_target);

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
        return status;


    handle_t hRaftTFMExplicitBinding = NULL;


    // Validates the format of the string binding handle and converts
    // it to a binding handle.
    // Connection is not done here either.
    status = RpcBindingFromStringBindingA(
        szStringBinding,    // The string binding to validate.
        &hRaftTFMExplicitBinding);         // Put the result in the implicit binding
                            // handle defined in the IDL file.

    if (status)
        return status;

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
        return RpcExceptionCode();
    }
    RpcEndExcept

        // Free the memory allocated by a string.
        status = RpcStringFreeA(
            &szStringBinding); // String to be freed.

    if (status)
        return status;

    // Releases binding handle resources and disconnects from the server.
    status = RpcBindingFree(
        &hRaftTFMExplicitBinding); // Frees the implicit binding handle defined in
                            // the IDL file.

    if (status)
        return status;

    return NO_ERROR;
}


