#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string>

class Command
{
private:
	uint32_t state_machine_command_;
	uint32_t term_;
	
public:
	void set_state_machime_command(uint32_t state_machine_command);
	uint32_t get_state_machime_command();

	void set_term_when_entry_was_received_by_leader(uint32_t term);
	uint32_t get_term_when_entry_was_received_by_leader();

};

