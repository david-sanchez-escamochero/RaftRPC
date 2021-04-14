#pragma once

#include "RPC_sockets.h"
#include <string>


class IRole {

public:
    virtual ~IRole() {}
	virtual void start()																	        						= 0;
	virtual void send(RPC_sockets* rpc, unsigned short port, std::string sender, std::string action, std::string receiver)	= 0;
	virtual void receive(RPC_sockets* rpc)																					= 0;	
    virtual void append_entry_role(
                                /* [in] */ int argument_term_,
                                /* [in] */ int argument_leader_id_,
                                /* [in] */ int argument_prev_log_index_,
                                /* [in] */ int argument_prev_log_term_,
                                /* [in] */ int argument_entries_[10],
                                /* [in] */ int argument_leader_commit_,
                                /* [out] */ int* result_term_,
                                /* [out] */ int* result_success_)                                                           = 0;
    

    virtual void request_vote_role(
                                /* [in] */ int argument_term_,
                                /* [in] */ int argument_candidate_id_,
                                /* [in] */ int argument_last_log_index_,
                                /* [in] */ int argument_last_log_term_,
                                /* [out] */ int* result_term_,
                                /* [out] */ int* result_vote_granted_)                                                       = 0;

	
};
