

#include "dev_tests.hpp"
#include "sine_wave.hpp"


int main(int argc, char* argv[]){
    if (argc!= 2){
    }

    DevTests::dev_tests_main();
    SineWave::sine_wave_main();

    return 0; 
}