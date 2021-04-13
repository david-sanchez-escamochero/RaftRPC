#include "RaftUtils.h"

std::string RaftUtils::parse_state_to_string(StateEnum state) {
	std::string str;
	if (state == StateEnum::candidate_state)
		str = std::string(CANDIDATE_TEXT);
	else if (state == StateEnum::follower_state)
		str = std::string(FOLLOWER_TEXT);
	else if (state == StateEnum::leader_state)
		str = std::string(LEADER_TEXT);
	else
		str = std::string(UNKNOWN_TEXT);

	return str;
}


std::string RaftUtils::parse_from_rcp_enum_to_text(RPCTypeEnum_sockets type)
{
	std::string ret;
	switch (type) {
	case RPCTypeEnum_sockets::rpc_append_entry:
		ret = std::string(APPEND_ENTRY_TEXT);
		break;
	case RPCTypeEnum_sockets::rpc_append_request_vote:
		ret = std::string(REQUEST_VOTE_TEXT);
		break;
	case RPCTypeEnum_sockets::rpc_append_heart_beat:
		ret = std::string(HEART_BEAT_TEXT);
		break;
	case RPCTypeEnum_sockets::rpc_client_request_value:
		ret = std::string(CLIENT_REQUEST_VALUE_TEXT);
		break;
	case RPCTypeEnum_sockets::rpc_client_request_leader:
		ret = std::string(CLIENT_REQUEST_LEADER_TEXT);
		break;
	default:
		ret = std::string(UNKNOWN_TEXT);
	}
	return ret;
}
