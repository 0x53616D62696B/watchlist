#include "factory_design.hpp"

// Implementation of IDAQDataTransfer constructor
IDAQDataTransfer::IDAQDataTransfer(const std::string& name) : mName(name) {}

// Implementation of DAQDataTransferA's Show method
void DAQDataTransferA::Show() {
    std::cout << "This is DAQDataTransfer A with name: " << mName << "\n" << std::endl;
}

// Implementation of DAQDataTransferB's Show method
void DAQDataTransferB::Show() {
    std::cout << "This is DAQDataTransfer B with name: " << mName << "\n" << std::endl;
}

// Implementation of DAQDataTransferFactory's GetInstance method
DAQDataTransferFactory& DAQDataTransferFactory::GetInstance() {
    static DAQDataTransferFactory instance;
    return instance;
}

// Implementation of DAQDataTransferFactory's CreateChannel method 
std::unique_ptr<IDAQDataTransfer> DAQDataTransferFactory::CreateChannel(const std::string& name, 
                                                                        const std::string& channelType,
                                                                        const std::string& mmap_path) {
    return std::unique_ptr<IDAQDataTransfer>(new DAQDataTransferA(name)); //TODO redo new
}

std::unique_ptr<IDAQDataTransfer> DAQDataTransferFactory::CreateChannel(const std::string& name, 
                                                                        const std::string& channelType,
                                                                        const std::string& mmap_path,
                                                                        const std::string& fifo_path) {
    return std::unique_ptr<IDAQDataTransfer>(new DAQDataTransferB(name)); //TODO redo new
}
