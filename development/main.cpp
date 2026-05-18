#include "dev_tests.hpp"
#include "sine_wave.hpp"
#include "memory_mng.hpp"
#include "factory_design.hpp"
#include "diamond_problem.cpp"
#include "pure_virtual_method_in_abs_cls_constructor.hpp"
#include "exception_throwing.hpp"
#include "unique_ptrs.hpp"
#include "cracking_code_interview/smart_ptr_428.hpp"

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
    IDAQDataTransfer* channelA = DAQDataTransferFactory::CreateChannel("First Channel", "A", "test_path");
    channelA->Show();
    
    IDAQDataTransfer* channelB = DAQDataTransferFactory::CreateChannel("Second Channel", "B", "test_path", "test_path_2");
    channelB->Show();

    auto& factory = DAQDataTransferFactory::GetInstance();
    auto channelB2 = factory.CreateChannel("Third Channel", "B", "test_path", "test_path_2");
    channelB2->Show();

    //? These could be part of Unit Tests
    // DAQDataTransferFactory factory3 = DAQDataTransferFactory(); //TODO create better singleton error handling
    // DAQDataTransferFactory factory4; //TODO create better singleton error handling
    // auto factory2 = DAQDataTransferFactory::GetInstance(); //TODO create better singleton error handling

    //TODO unit tests

    //TODO error handling


    //* Diamond Problem solution
    /*
    Final obj;
    obj.Show(); // Calls Final's Show method - could throw Error: 'Show' is ambiguous
    // Accessing Base class method through Final object
    obj.Base::Show(); // Calls Base's Show method, avoiding ambiguity
    obj.Derived1::Show();
    obj.Derived2::show();
    */

    // Handle undefined states of int and size_t
    #include <limits>
    const int INT_UNDEFINED = -1;
    const size_t SIZE_T_UNDEFINED = std::numeric_limits<size_t>::max();
    const size_t SIZE_T_UNDEFINED_MIN = std::numeric_limits<size_t>::min();
    printf("INT_UNDEFINED: %d\n", INT_UNDEFINED);
    printf("SIZE_T_UNDEFINED: %zu\n", SIZE_T_UNDEFINED);
    printf("SIZE_T_UNDEFINED_MIN: %zu\n", SIZE_T_UNDEFINED_MIN);

    //* abs cls
    abs_cls_main();


    //* exception handling
    main_exception_throwing();

    //* unique pointers
    main_unique_ptrs();

    //* Cracking code interview: Smart Pointer: pg 428:
    main_smart_ptr_428();


    return 0; 
}