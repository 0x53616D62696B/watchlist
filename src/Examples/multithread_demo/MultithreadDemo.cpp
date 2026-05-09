// CPP program to demonstrate multithreading
// using three different callables.
#include <thread>

#include "src/Utils/Logger/Logger.hpp"

// A dummy function
void foo(int Z)
{
    for (int i = 0; i < Z; i++) {
        LOG_INFO("Thread using function pointer as callable");
    }
}
  
// A callable object
class thread_obj {
public:
    void operator()(int x)
    {
        for (int i = 0; i < x; i++)
           LOG_INFO("Thread using function object as callable");
    }
};
  

int main()
{
    LOG_INFO("Threads 1 and 2 and 3 operating independently");
  
    // This thread is launched by using 
    // function pointer as callable
    std::thread th1(foo, 3);
  
    // This thread is launched by using
    // function object as callable
    std::thread th2(thread_obj(), 3);
  
    // Define a Lambda Expression
    auto f = [](int x) {
        for (int i = 0; i < x; i++)
            LOG_INFO("Thread using lambda expression as callable");
    };
  
    // This thread is launched by using 
    // lamda expression as callable
    std::thread th3(f, 3);
  
    // Wait for the threads to finish
    // Wait for thread t1 to finish
    th1.join();
  
    // Wait for thread t2 to finish
    th2.join();
  
    // Wait for thread t3 to finish
    th3.join();
    
    return 0;
}
