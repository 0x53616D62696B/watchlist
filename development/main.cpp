

#include "dev_tests.hpp"
#include "sine_wave.hpp"
#include "memory_mng.hpp"


int main(int argc, char* argv[]){
    if (argc!= 2){
    }

    // * Data generators
    //DevTests::dev_tests_main();
    //SineWave::sine_wave_main();

    //* Memory management
    MemoryManagement::hermes_usb_acqu_setup e;
    MemoryManagement::memory_mng_main('a', &e);

    return 0; 
}