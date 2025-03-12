#include "factory_design.hpp"

// Implementation of IDAQTransfer constructor
IDAQTransfer::IDAQTransfer(const std::string& name) : mName(name) {}

// Implementation of DAQTransferA's show method
void DAQTransferA::show() {
    std::cout << "This is DAQTransfer A with name: " << mName << "\n" << std::endl;
}

// Implementation of DAQTransferB's show method
void DAQTransferB::show() {
    std::cout << "This is DAQTransfer B with name: " << mName << "\n" << std::endl;
}

// Implementation of DAQTransferFactory's getInstance method
DAQTransferFactory& DAQTransferFactory::getInstance() {
    static DAQTransferFactory instance;
    return instance;
}

// Implementation of DAQTransferFactory's createChannel method //TODO remove unique_ptr?
std::unique_ptr<IDAQTransfer> DAQTransferFactory::createChannel(const std::string& name, const std::string& channelType) {
    if (channelType == "A")
        return std::unique_ptr<IDAQTransfer>(new DAQTransferA(name)); //TODO redo new
    else if (channelType == "B")
        return std::unique_ptr<IDAQTransfer>(new DAQTransferB(name)); //TODO redo new
    else
        return nullptr;
}
