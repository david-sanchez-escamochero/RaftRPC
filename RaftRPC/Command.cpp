#include "Command.h"

void Command::set_state_machime_command(int state_machine_command)
{
	state_machine_command_ = state_machine_command;
}

int Command::get_state_machime_command()
{
	return state_machine_command_;
}

void Command::set_term_when_entry_was_received_by_leader(int term)
{
	term_ = term;
}

int Command::get_term_when_entry_was_received_by_leader()
{
	return term_;
}

