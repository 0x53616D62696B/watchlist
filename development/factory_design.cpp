#include "factory_design.hpp"

// Implementation of DAQTransferA's show method
void DAQTransferA::show() {
    std::cout << "This is Product A\n";
}

// Implementation of DAQTransferB's show method
void DAQTransferB::show() {
    std::cout << "This is Product B\n";
}

// Implementation of DAQTransferFactory's getInstance method
DAQTransferFactory& DAQTransferFactory::getInstance() {
    static DAQTransferFactory instance;
    return instance;
}

// Implementation of DAQTransferFactory's createChannel method
std::unique_ptr<IDAQTransfer> DAQTransferFactory::createChannel(const std::string& productType) {
    if (productType == "A")
        return std::unique_ptr<IDAQTransfer>(new DAQTransferA());
    else if (productType == "B")
        return std::unique_ptr<IDAQTransfer>(new DAQTransferB());
    else
        return nullptr;
}
