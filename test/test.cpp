#include "../src/Utils/Logger/Logger.hpp"

int main(int argc, char* argv[]){
    if (argc!= 2){
    }

    Log(LogLevel::Info, "Logging from main thread");
    printf("Starting");

    return 0;
}