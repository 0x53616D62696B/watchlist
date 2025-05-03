/*
*/

#include <string>
#include <cstring>

#include "datastream_tcp_client.h"
#include "comm/ut_network.h"
#include "fw_monitor.h"

namespace DatastreamTCP{

void SetSocket()
{
    std::string ip_address = SERVER_IP_ADDRESS;
    char* ip_address_char = new char[ip_address.length() + 1]; // Initialize the ip_address_char variable
    std::strncpy(reinterpret_cast<char*>(ip_address_char), ip_address.c_str(), ip_address.length());
    //strncpy_s(ip_address_char, ip_address.length() + 1, ip_address.c_str(), ip_address.length());
    DAQSock = new OsTcpClient("DAQData", ip_address_char, SERVER_PORT, false, false);
}

void SendMsg()
{
    std::string msg = "Hello, server!";
    char* buffer_char = new char[msg.length() + 1]; // Initialize the buffer_char variable
    std::strncpy(reinterpret_cast<char*>(buffer_char), msg.c_str(), msg.length());
    DAQSock->Send(buffer_char, msg.length(), true);
}

void ReceiveMsg()
{
    //char* retrieved_msg_char[1024];
    mBuffer.reserve(100); // TODO not a member of a class! YET
    size_t retrieved_msg_len = DAQSock->Receive(&mBuffer, 100, 1); // ! to get a pointer I could use mBuffer.data()
    std::cout << "Reply from server: " << &mBuffer << ". With msg length: " << retrieved_msg_len << std::endl;
}

void RunSocketDevTests()
{
    SetSocket();
    //WaitForConnection(); //TODO how to wait? async await? 
    // Connect to the server
    DAQSock->ConnectionOk(true);
    SendMsg();
    ReceiveMsg();
}

} // namespace DatastreamTCP