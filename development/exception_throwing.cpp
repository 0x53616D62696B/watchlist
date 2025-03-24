#include <string>
// #include <exception>
#include <stdexcept>
#include <iostream>

class DataTransferFailure: public std::runtime_error
{
public:
	DataTransferFailure(const std::string& message = "")
		: std::runtime_error("Data transfer failure: " + message)
	{}
};


class DataTransferCommunicationFailure: public DataTransferFailure
{
public:
	DataTransferCommunicationFailure(const std::string& message = "Data transfer communication failure")
		: DataTransferFailure(message)
	{}
};

void main_exception_throwing(){
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
}
