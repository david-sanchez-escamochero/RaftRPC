#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string>

class Command
{
private:
	int state_machine_command_;
	int term_;
	
public:
	void set_state_machime_command(int state_machine_command);
	int get_state_machime_command();

	void set_term_when_entry_was_received_by_leader(int term);
	int get_term_when_entry_was_received_by_leader();

};

