#pragma once

#include <string>
#include <mutex>

enum class ServeryTrace { warning_trace, info_trace, error_trace, action_trace };

class Tracer
{
public:
	Tracer();
	static void trace(std::string str_log, ServeryTrace severity = ServeryTrace::info_trace);
	
private:
	
	std::mutex mu_;
};

