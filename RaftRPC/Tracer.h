#pragma once

#include <string>
#include <mutex>

class Tracer
{
public:
	Tracer();
	static void trace(std::string str_log);
	
private:
	
	std::mutex mu_;
};

