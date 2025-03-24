#include "exception_throwing.hpp"

void main_exception_throwing(){
    std::cout << "\n";
    try{

        throw DataTransferFailure("Test message");
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }

    try{

        throw DataTransferCommunicationFailure();
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }

    try{

        throw DataTransferCommunicationFailure("Test message");
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }

    try{

        throw DataTransferCommunicationFailure("ID 123");
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }
    try{

        throw DataTransferDisconnectedFailure("Disconnected while transfering data in time 12:23");
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }
}
