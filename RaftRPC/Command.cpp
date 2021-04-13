#include "Command.h"

void Command::set_state_machime_command(uint32_t state_machine_command)
{
	state_machine_command_ = state_machine_command;
}

uint32_t Command::get_state_machime_command()
{
	return state_machine_command_;
}

void Command::set_term_when_entry_was_received_by_leader(uint32_t term)
{
	term_ = term;
}

uint32_t Command::get_term_when_entry_was_received_by_leader()
{
	return term_;
}

