#include "exception_throwing.hpp"

void main_exception_throwing(){
    std::cout << "\n";
    try{

        throw DataTransferFailure("Test message");
    }
    catch (RuntimeException& e){
        std::cout << e.what() << std::endl;
    }

    try{

        throw DataTransferCommunicationFailure();
    }
    catch (DataTransferCommunicationFailure& e){
        std::cout << e.what() << std::endl;
    }

    try{

        throw DataTransferCommunicationFailure("Test message");
    }
    catch (DataTransferCommunicationFailure& e){
        std::cout << e.what() << std::endl;
    }

    try{

        throw DataTransferCommunicationFailure("ID 123");
    }
    catch (DataTransferCommunicationFailure& e){
        std::cout << e.what() << std::endl;
    }
    try{

        throw DataTransferDisconnectedFailure("Disconnected while transfering data in time 12:23");
    }
    catch (DataTransferDisconnectedFailure& e){
        std::cout << e.what() << std::endl;
    }
}
