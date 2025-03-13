#include "factory_design.hpp"

// Implementation of IDAQDataTransfer constructor
IDAQDataTransfer::IDAQDataTransfer(const std::string& name) : mName(name) {}

// Implementation of DAQDataTransferA's show method
void DAQDataTransferA::show() {
    std::cout << "This is DAQDataTransfer A with name: " << mName << "\n" << std::endl;
}

// Implementation of DAQDataTransferB's show method
void DAQDataTransferB::show() {
    std::cout << "This is DAQDataTransfer B with name: " << mName << "\n" << std::endl;
}

// Implementation of DAQDataTransferFactory's getInstance method
DAQDataTransferFactory& DAQDataTransferFactory::getInstance() {
    static DAQDataTransferFactory instance;
    return instance;
}

// Implementation of DAQDataTransferFactory's createChannel method 
//TODO - creation should be done based on polymorphism and arguments passed. They are different amount of args passed. Just like in my first example in ftapp code
std::unique_ptr<IDAQDataTransfer> DAQDataTransferFactory::createChannel(const std::string& name, const std::string& channelType) {
    if (channelType == "A")
        return std::unique_ptr<IDAQDataTransfer>(new DAQDataTransferA(name)); //TODO redo new
    else if (channelType == "B")
        return std::unique_ptr<IDAQDataTransfer>(new DAQDataTransferB(name)); //TODO redo new
    else
        return nullptr;
}


/*TODO NOTES
This is how current channels are created. My solution with unique_ptrs should fit as well..

MemChannel=new DatastreamDAQImplementation::DatastreamDAQ(
				DatastreamDAQImplementation::DRV_RESP_FILE_PATH,
				DatastreamDAQImplementation::DRV_REQ_FILE_PATH,
				DatastreamDAQImplementation::MMAP_FILE_PATH,
				OverallSize*sizeof(RawSample)
			);

*/ 