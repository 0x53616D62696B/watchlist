#include <string>
#include <stdexcept>
#include <iostream>


class DataTransferFailure: public std::runtime_error
{
public:
    DataTransferFailure(const std::string& message)
        : std::runtime_error(message)
    {}
};

template <const std::string& PREMSG>
class DataTransferFailureTemplate: public DataTransferFailure
{
public:
    DataTransferFailureTemplate(const std::string& message = "")
        : DataTransferFailure(PREMSG + message)
    {}
};


// Create a type alias
const std::string COMMUNICATION_FAILURE = "Data transfer communication failure: ";
using DataTransferCommunicationFailure = DataTransferFailureTemplate<COMMUNICATION_FAILURE>;
const std::string DISCONNECTED = "Disconnected while transfering data: ";
using DataTransferDisconnectedFailure = DataTransferFailureTemplate<DISCONNECTED>;


void main_exception_throwing(){
    std::cout << "\n" << std::endl;
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
