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

// Implementation of DAQTransferFactory's createChannel method 
//TODO - creation should be done based on polymorphism and arguments passed. They are different amount of args passed. Just like in my first example in ftapp code
std::unique_ptr<IDAQTransfer> DAQTransferFactory::createChannel(const std::string& name, const std::string& channelType) {
    if (channelType == "A")
        return std::unique_ptr<IDAQTransfer>(new DAQTransferA(name)); //TODO redo new
    else if (channelType == "B")
        return std::unique_ptr<IDAQTransfer>(new DAQTransferB(name)); //TODO redo new
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