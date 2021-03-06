#include "RaftUtils.h"
#include "ClientDefs.h"


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


std::string RaftUtils::parse_from_socket_enum_to_text(ClientRequesTypeEnum type)
{
	std::string ret;
	switch (type) {

	case ClientRequesTypeEnum::client_write_master:
		ret = std::string(CLIENT_REQUEST_VALUE_TEXT);
		break;
	case ClientRequesTypeEnum::client_request_leader:
		ret = std::string(CLIENT_REQUEST_LEADER_TEXT);
		break;
	case ClientRequesTypeEnum::client_ping_master:
		ret = std::string(CLIENT_PING_TEXT);
		break;
	default:
		ret = std::string(UNKNOWN_TEXT);
	}
	return ret;
}


std::string RaftUtils::parse_from_rpc_enum_to_text(RPCTypeEnum type)
{
	std::string ret;
	switch (type) {
	case RPCTypeEnum::rpc_append_entry:
		ret = std::string(APPEND_ENTRY_TEXT);
		break;
	case RPCTypeEnum::rpc_append_heart_beat:
		ret = std::string(HEART_BEAT_TEXT);
		break;
	case RPCTypeEnum::rpc_append_request_vote:
		ret = std::string(REQUEST_VOTE_TEXT);
		break;
	default:
		ret = std::string(UNKNOWN_TEXT);
	}
	return ret;
}

