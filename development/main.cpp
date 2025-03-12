

#include "dev_tests.hpp"
#include "sine_wave.hpp"
#include "memory_mng.hpp"
#include "factory_design.cpp"

int main(int argc, char* argv[]){
    if (argc!= 2){
    }

    // * Data generators
    //DevTests::dev_tests_main();
    //SineWave::sine_wave_main();

    //* Memory management
    //MemoryManagement::hermes_usb_acqu_setup e;
    //MemoryManagement::memory_mng_main('a', &e);

    //* Factory Design
    auto channelA = DAQTransferFactory::createChannel("A");
    channelA->show();
    
    auto channelB = DAQTransferFactory::createChannel("B");
    channelB->show();

    auto& factory = DAQTransferFactory::getInstance();
    auto channelB2 = factory.createChannel("B");
    channelB2->show();

    // DAQTransferFactory factory3 = DAQTransferFactory(); //TODO create better singleton error handling
    // DAQTransferFactory factory4; //TODO create better singleton error handling
    // auto factory2 = DAQTransferFactory::getInstance(); //TODO create better singleton error handling

    return 0; 
}