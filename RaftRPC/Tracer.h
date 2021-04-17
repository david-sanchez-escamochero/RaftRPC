#pragma once

#include <string>
#include <mutex>

enum class SeverityTrace { warning_trace, info_trace, error_trace, action_trace, change_status_trace };

class Tracer
{
public:
	Tracer();
	static void trace(std::string str_log, SeverityTrace severity = SeverityTrace::info_trace);
	
private:
	
	std::mutex mu_;
};

