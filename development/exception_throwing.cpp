#include <string>
// #include <exception>
#include <stdexcept>
#include <iostream>


class DataTransferFailure: public std::runtime_error
{
public:
	DataTransferFailure(const std::string& message)
		: std::runtime_error(message)
	{}
};

template <const char* premsg>
class DataTransferFailureTemplate: public DataTransferFailure
{
public:
	DataTransferFailureTemplate(const std::string& message = "")
		: DataTransferFailure(premsg + message)
	{}
};

constexpr char premsg_[] = "Some premsq: ";
constexpr char premsg_2_[] = "Premsq 2: ";

// Create a type alias
using MyNewClass = DataTransferFailureTemplate<premsg_>;
using MyNewClass_2 = DataTransferFailureTemplate<premsg_2_>;


// DataTransferFailureTemplate<premsg_> MyNewClass;
// MyNewClass("Constructor message");

// class MyNewClass : public DataTransferFailureTemplate<premsg_>
// {
// public:
//     MyNewClass(const std::string& message = "")
//         : DataTransferFailureTemplate<premsg_>(message)
//     {}
// };





//TODO create template for class? Only diff would be the message
class DataTransferCommunicationFailure: public DataTransferFailure
{
public:
	DataTransferCommunicationFailure(const std::string& message = "Data transfer communication failure")
		: DataTransferFailure(message)
	{}
};

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

        throw MyNewClass("Test message");
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }
    try{

        throw MyNewClass_2("Test message 2");
    }
    catch (DataTransferFailure& e){
        std::cout << e.what() << std::endl;
    }
}
