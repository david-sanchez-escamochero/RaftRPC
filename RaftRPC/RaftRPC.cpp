// RaftTFM.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Server.h"
#include <stdint.h>
#include <string>
#include "Communication.h"
#include "RPC.h"
//#include "Client.h"



int main(int argc, char** argv)
{
    std::cout << "RAFT Test...\n";
    if (argc < 2) {
        printf("Usage:\r\n");
        printf("Param (1): id server\r\n");
        printf("Example RaftTest.exe 1\r\n");
    }
    else {
        // If it is a Client. 
        //if (std::stoi(argv[1]) > NUM_SERVERS) {
        //    Tracer::trace("*************************\r\n");
        //    Tracer::trace("   Starting RaftClient\r\n");
        //    Tracer::trace("************************\r\n");
        //    Client client;
        //    if (!client.start(".\\..\\Debug\\numbers_starts_with_1.txt", std::stoi(argv[1])))
        //        Tracer::trace("RaftClient - Failed to start. \r\n");
        //}
        // If it is a Server. 
        //else {
            Server server(std::stoi(argv[1]));
            server.start();
        //}
    }
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
