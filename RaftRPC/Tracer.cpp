#include "Tracer.h"
#include <thread>
#include <chrono>
#include <ctime> 


 

unsigned long long GetTickCount()
{
	long long tick; 

	//try {
		using namespace std::chrono;
		tick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	//}
	//catch (...)
	//{
	//	tick = 0;
	//}
	return tick; 
}


Tracer::Tracer() 
{
	
}

void Tracer::trace(std::string str_log, SeverityTrace severity)
{	
		if (severity == SeverityTrace::info_trace) 
			printf("\033[0;37m");// White					
		else if (severity == SeverityTrace::warning_trace)
			printf("\033[0;33m");// Yellow
		else if (severity == SeverityTrace::error_trace)
			printf("\033[1;31m");// Red
		else if(severity == SeverityTrace::action_trace)
			printf("\033[0;32m");// Green
		else if(severity == SeverityTrace::change_status_trace)
			printf("\033[0;36m");// Cyan
			

		
		static int count_line_ = 0;
		//std::lock_guard<std::mutex> guard(mu_); // RAII
		str_log = std::to_string(count_line_++) + ".-" + "[" + std::to_string(GetTickCount()) + "]" + str_log;
		printf(str_log.c_str());

		
}
