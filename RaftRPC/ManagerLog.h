#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdint.h>

using namespace std;

#define MANAGER_NO_ERROR                0
#define MANAGER_ERROR_TO_OPEN_FILE      1
#define MANAGER_ERROR_NOT_OPENED_FILE   2
#define MANAGER_ERROR_NOT_READ_LOG      3
#define MANAGER_ERROR_NOT_WRITE_LOG     4


class ManagerLog
{
public:
    ManagerLog();
    int write_log(std::string file_name, void* log, int size_to_write);
    int read_log(std::string file_name, void* log, int size_to_read);
};

