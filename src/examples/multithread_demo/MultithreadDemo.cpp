// CPP program to demonstrate multithreading
// using three different callables.
#include <iostream>
#include <thread>

#include <ctime>
// A dummy function
void foo(int Z)
{
    for (int i = 0; i < Z; i++) {
        std::cout << "Thread using function"
               " pointer as callable\n";
    }
}
  
// A callable object
class thread_obj {
public:
    void operator()(int x)
    {
        for (int i = 0; i < x; i++)
           std::cout << "Thread using function"
                  " object as  callable\n";
    }
};
  

int main()
{
    std::cout << "Threads 1 and 2 and 3 "
         "operating independently" << std::endl;
  
    // This thread is launched by using 
    // function pointer as callable
    std::thread th1(foo, 3);
  
    // This thread is launched by using
    // function object as callable
    std::thread th2(thread_obj(), 3);
  
    // Define a Lambda Expression
    auto f = [](int x) {
        for (int i = 0; i < x; i++)
            std::cout << "Thread using lambda"
             " expression as callable\n";
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

    //const std::chrono::time_point tp = std::chrono::system_clock::now();

    //std::cout << "Heloo" << tp << std::endl;

    /*
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char buf[100];
    std::strftime(buf, sizeof buf, "%D %T", std::gmtime(&ts.tv_sec));
    std::cout << "Current time: " << buf << '.' << ts.tv_nsec << " UTC\n";
    */
    return 0;
}