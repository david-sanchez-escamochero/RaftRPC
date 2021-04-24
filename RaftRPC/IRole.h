#pragma once

#include "ClientRequest.h"
#include <string>


class IRole {

public:
    virtual ~IRole() {}
	virtual void start()																	        						= 0;
	virtual void send_msg_socket(ClientRequest* client_request, unsigned short port, std::string sender, std::string action, std::string receiver)	= 0;
	virtual void receive_msg_socket(ClientRequest* client_request)																					= 0;
    virtual void append_entry_role(
                                /* [in] */ int argument_term,
                                /* [in] */ int argument_leader_id,
                                /* [in] */ int argument_prev_log_index,
                                /* [in] */ int argument_prev_log_term,
                                /* [in] */ int argument_entries[10],
                                /* [in] */ int argument_leader_commit,
                                /* [out] */ int* result_term,
                                /* [out] */ int* result_success)                                                           = 0;
    

    virtual void request_vote_role(
                                /* [in] */ int argument_term,
                                /* [in] */ int argument_candidate_id,
                                /* [in] */ int argument_last_log_index,
                                /* [in] */ int argument_last_log_term,
                                /* [out] */ int* result_term,
                                /* [out] */ int* result_vote_granted)                                                       = 0;

	
};
