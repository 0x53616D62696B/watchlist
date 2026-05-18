#include "smart_ptr_428.hpp"

#include <cstdio>
#include <memory>

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
    delete smrtp;

    printf("\nmain_smart_ptr_428() ended");

    return 0;
}
