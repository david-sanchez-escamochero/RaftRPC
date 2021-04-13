#include "Tracer.h"
#include <thread>
#include <chrono>
#include <ctime> 


unsigned long long GetTickCount()
{
	long long tick; 

	try {
		using namespace std::chrono;
		tick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	}
	catch (...)
	{
		tick = 0;
	}
	return tick; 
}


Tracer::Tracer() 
{
	
}

void Tracer::trace(std::string str_log)
{	
		static uint32_t count_line_ = 0;
		//std::lock_guard<std::mutex> guard(mu_); // RAII
		str_log = std::to_string(count_line_++) + ".-" + "[" + std::to_string(GetTickCount()) + "]" + str_log;
		printf(str_log.c_str());
}
