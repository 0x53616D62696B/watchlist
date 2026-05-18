
// template<typename Type>
template<class Type>
class SmartPointer{
    public:
        SmartPointer(Type* ptr){
            ref = ptr;
        }
        SmartPointer(Type* ptr): _ptr(ptr) {
            ref = ptr;
        }
        SmartPointer(SmartPointer<Type>& sptr){
            ref = sptr.ref;
        }
        ~SmartPointer() {
            delete _ptr;
        }
        int ReferenceCount() const;

        // SmartPointer::__entry__();
};



int main_smart_ptr_428();

// SmartPointer<int> ptr(new int(5)); // Example usage
SmartPointer<int> smart_ptr(); // Example usage





// My implementation of Exceptions as example

//* Error Throwables
class RuntimeException : public std::runtime_error {
   public:
    explicit RuntimeException(const std::string &message) : std::runtime_error(message) {}
};
template <const char *PREMSG>
class RuntimeExceptionTemplate : public RuntimeException {
   public:
    RuntimeExceptionTemplate(const std::string &message = "") : RuntimeException(std::string(PREMSG) + message) {}
};

struct ExceptionMsgs {
    static constexpr char COMMUNICATION_FAILURE_MSG[] = "Transfer Communication Failure. ";
    static constexpr char MMAP_FAILURE_MSG[] = "MMap Failure. ";
    static constexpr char READ_FAILURE_MSG[] = "Data Transfer Read Failure. ";
    static constexpr char READ_TIMEOUT_FAILURE_MSG[] = "Data Transfer Read Timeout Failure. ";
    static constexpr char READ_NOT_CONNECTED_FAILURE_MSG[] = "Data Transfer Read Not Connected Failure. ";
    static constexpr char WRITE_FAILURE_MSG[] = "Data Transfer Write Failure. ";
};

using DataTransferFailure = RuntimeException;
using DataTransferCommunicationFailure = RuntimeExceptionTemplate<ExceptionMsgs::COMMUNICATION_FAILURE_MSG>;
using DataTransferMemMapFailure = RuntimeExceptionTemplate<ExceptionMsgs::MMAP_FAILURE_MSG>;
using DataTransferReadFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_FAILURE_MSG>;
using DataTransferReadTimeoutFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_TIMEOUT_FAILURE_MSG>;
using DataTransferReadNotConnectedFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_NOT_CONNECTED_FAILURE_MSG>;
using DataTransferWriteFailure = RuntimeExceptionTemplate<ExceptionMsgs::WRITE_FAILURE_MSG>;
