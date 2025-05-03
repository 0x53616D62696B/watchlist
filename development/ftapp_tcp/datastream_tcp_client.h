#pragma once

#ifndef DATASTREAM_CLIENT_H_
    #define DATASTREAM_CLIENT_H_
#endif

#include "util/fw_opsys.h"

namespace DatastreamTCP{

#define SERVER_IP_ADDRESS "172.16.0.222"
#define SERVER_PORT 12345

static OsTcpClient* DAQSock;
//const char* retrieved_msg_char = new char[1024]{}; // 1KB buffer
static std::vector<std::uint8_t> mBuffer; // Instead of retrieved_msg_char, use this vector


void SetSocket();
void SendMsg();
void ReceiveMsg(); 
void RunSocketDevTests();

} // namespace DatastreamTCP