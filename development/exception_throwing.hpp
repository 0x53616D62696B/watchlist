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

//* In GCC compiler for cpp11 it is not possible to have string as template parameter 
//* because it cannot be compile time constant variable. Replace with const char* as template arg, 
//* and constexpr char MY_CONST[] = "my_const"; as a variable.
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


void main_exception_throwing();