#include <string>
#include <stdexcept>
#include <iostream>

class RuntimeException: public std::runtime_error
{
public:
    RuntimeException(const std::string& message)
        : std::runtime_error(message)
    {}
};

//* In GCC compiler for cpp11 it is not possible to have string as template parameter 
//* because it cannot be compile time constant variable. Replace with const char* as template arg, 
//* and constexpr char MY_CONST[] = "my_const"; as a variable.



template <const char* PREMSG>
class RuntimeExceptionTemplate: public RuntimeException
{
public:
    RuntimeExceptionTemplate(const std::string& message = "")
        : RuntimeException(std::string(PREMSG) + message)
    {}
};

// Create a type alias
// const std::string COMMUNICATION_FAILURE = "Data transfer communication failure: ";
// using DataTransferCommunicationFailure = DataTransferFailureTemplate<COMMUNICATION_FAILURE>;
// const std::string DISCONNECTED = "Disconnected while transfering data: ";
// using DataTransferDisconnectedFailure = DataTransferFailureTemplate<DISCONNECTED>;

struct ExceptionMsgs {
    static constexpr char COMMUNICATION_FAILURE_MSG[] = "Transfer Communication Failure. ";
    static constexpr char MMAP_FAILURE_MSG[] = "MMap Failure. ";
    static constexpr char READ_FAILURE_MSG[] = "Data Transfer Read Failure. ";
    static constexpr char READ_TIMEOUT_FAILURE_MSG[] = "Data Transfer Read Timeout Failure. ";
    static constexpr char READ_NOT_CONNECTED_FAILURE_MSG[] = "Data Transfer Read Not Connected Failure. ";
    static constexpr char WRITE_FAILURE_MSG[] = "Data Transfer Write Failure. ";
    static constexpr char DISCONNECTED[] = "Disconnected while transfering data. ";
};

using DataTransferFailure = RuntimeException;
using DataTransferCommunicationFailure = RuntimeExceptionTemplate<ExceptionMsgs::COMMUNICATION_FAILURE_MSG>;
using DataTransferMemMapFailure = RuntimeExceptionTemplate<ExceptionMsgs::MMAP_FAILURE_MSG>;
using DataTransferReadFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_FAILURE_MSG>;
using DataTransferReadTimeoutFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_TIMEOUT_FAILURE_MSG>;
using DataTransferReadNotConnectedFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_NOT_CONNECTED_FAILURE_MSG>;
using DataTransferWriteFailure = RuntimeExceptionTemplate<ExceptionMsgs::WRITE_FAILURE_MSG>;
using DataTransferDisconnectedFailure = RuntimeExceptionTemplate<ExceptionMsgs::DISCONNECTED>;

// constexpr char DATA_TRANSFER_FAILURE_MSG[] = "Data Transfer Generic Failure.";
// using DataTransferFailure = RuntimeExceptionTemplate<DATA_TRANSFER_FAILURE_MSG>;
// constexpr char COMMUNICATION_FAILURE_MSG[] = "Transfer Communication Failure.";
// using DataTransferCommunicationFailure = RuntimeExceptionTemplate<COMMUNICATION_FAILURE_MSG>;
// constexpr char MMAP_FAILURE_MSG[] = "MMap Failure.";
// using DataTransferMemMapFailure = RuntimeExceptionTemplate<MMAP_FAILURE_MSG>;
// constexpr char READ_FAILURE_MSG[] = "Data Transfer Read Failure.";
// using DataTransferReadFailure = RuntimeExceptionTemplate<READ_FAILURE_MSG>;
// constexpr char READ_TIMEOUT_FAILURE_MSG[] = "Data Transfer Read Timeout Failure.";
// using DataTransferReadTimeoutFailure = RuntimeExceptionTemplate<READ_TIMEOUT_FAILURE_MSG>;
// constexpr char READ_NOT_CONNECTED_FAILURE_MSG[] = "Data Transfer Read Not Connected Failure.";
// using DataTransferReadNotConnectedFailure = RuntimeExceptionTemplate<READ_NOT_CONNECTED_FAILURE_MSG>;
// constexpr char WRITE_FAILURE_MSG[] = "Data Transfer Write Failure.";
// using DataTransferWriteFailure = RuntimeExceptionTemplate<WRITE_FAILURE_MSG>;
// constexpr char DISCONNECTED[] = "Disconnected while transfering data: ";
// using DataTransferDisconnectedFailure = RuntimeExceptionTemplate<DISCONNECTED>;


void main_exception_throwing();
