
// template<typename Type>
template<class T>
class SmartPointer{
    public:
        SmartPointer(T* ptr){
            ref = ptr;
            ref_count = (unsigned*)malloc(sizeof(unsigned));
            *ref_count = 1;
        }
        // SmartPointer(Type* ptr): _ptr(ptr) {
        //     ref_count = (unsigned*)malloc(sizeof(unsigned));
        //     *ref_count = 1;
        // }
        SmartPointer(SmartPointer<T>& sptr){
            ref = sptr.ref;
            ref_count = sptr.ref_count;
            (*ref_count)++;
        }

        SmartPointer<T>& operator=(SmartPointer<T>& sptr){
            if (this != &sptr) return *this;
            if (*ref_count > 0) {
                remove(); // Remove current reference
            }

            ref = sptr.ref;
            ref_count = sptr.ref_count;
            ++(*ref_count);
            return *this;
        }

        ~SmartPointer() {
            remove(); // Remove reference to object.
        }

        T getValue() {
            return *ref;
        }

    protected:
        void remove () {
            --(*ref_count);
            if (*ref_count == 0) {
                delete ref;
                free(ref_count);
                ref = NULL;
                ref_count = NULL;
            }
        }

        T * ref;
        unsigned * ref_count;
};



int main_smart_ptr_428(){
    printf("main_smart_ptr_428() called");
    SmartPointer<int> *smrtp = new SmartPointer<int>(new int(5)); // Example usage
    SmartPointer<int> smrtp_r = SmartPointer<int>(new int(5)); // Example usage
    SmartPointer<int> smrtp2(new int(11)); // Example usage

    int value = smrtp_r.getValue(); // Increment reference count
    value++;

    // std::unique_ptr example
    std::unique_ptr<int> uptr(new int(10));

    // std::shared_ptr example
    std::shared_ptr<int> sptr1(new int(15));
    std::shared_ptr<int> sptr2 = sptr1;  // Reference counting

    // Raw pointer example
    int* raw_ptr = new int(20);
    
    // std::weak_ptr example (for breaking circular references)
    std::weak_ptr<int> weak_ptr = sptr1;
    
    // Cleanup raw pointer
    delete raw_ptr;

    printf("\nmain_smart_ptr_428() ended");
    
    return 0;
}






// My implementation of Exceptions as example

//* Error Throwables
// class RuntimeException : public std::runtime_error {
//    public:
//     explicit RuntimeException(const std::string &message) : std::runtime_error(message) {}
// };
// template <const char *PREMSG>
// class RuntimeExceptionTemplate : public RuntimeException {
//    public:
//     RuntimeExceptionTemplate(const std::string &message = "") : RuntimeException(std::string(PREMSG) + message) {}
// };

// struct ExceptionMsgs {
//     static constexpr char COMMUNICATION_FAILURE_MSG[] = "Transfer Communication Failure. ";
//     static constexpr char MMAP_FAILURE_MSG[] = "MMap Failure. ";
//     static constexpr char READ_FAILURE_MSG[] = "Data Transfer Read Failure. ";
//     static constexpr char READ_TIMEOUT_FAILURE_MSG[] = "Data Transfer Read Timeout Failure. ";
//     static constexpr char READ_NOT_CONNECTED_FAILURE_MSG[] = "Data Transfer Read Not Connected Failure. ";
//     static constexpr char WRITE_FAILURE_MSG[] = "Data Transfer Write Failure. ";
// };

// using DataTransferFailure = RuntimeException;
// using DataTransferCommunicationFailure = RuntimeExceptionTemplate<ExceptionMsgs::COMMUNICATION_FAILURE_MSG>;
// using DataTransferMemMapFailure = RuntimeExceptionTemplate<ExceptionMsgs::MMAP_FAILURE_MSG>;
// using DataTransferReadFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_FAILURE_MSG>;
// using DataTransferReadTimeoutFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_TIMEOUT_FAILURE_MSG>;
// using DataTransferReadNotConnectedFailure = RuntimeExceptionTemplate<ExceptionMsgs::READ_NOT_CONNECTED_FAILURE_MSG>;
// using DataTransferWriteFailure = RuntimeExceptionTemplate<ExceptionMsgs::WRITE_FAILURE_MSG>;
