// #include <iostream>
// #include <string>

// // Base class with a pure virtual method
// class AbsCls {
// public:
//     // AbsCls(const std::string& aName)
//     //     //: mName(aName) 
//     //     {
//     //     // Call the virtual method
//     //     //Initialize();
//     //     std::cout << "AbsCls constructor" << std::endl;
//     // }

//     virtual ~AbsCls() {
//         std::cout << "AbsCls destructor called" << std::endl;
//         DeInitialize();
//     }

//     // Pure virtual method
//     virtual void DeInitialize() {};

// protected:
//     std::string mName;
// };

// // Derived class implementing the pure virtual method
// class DerivedCls : public AbsCls {
// public:
//     DerivedCls(const std::string& aName)
//         //: AbsCls(aName) 
//         {
//             std::cout << "DerivedCls constructor" << std::endl;
//         }
//     ~DerivedCls(){
//         std::cout << "DerivedCls destructor" << std::endl;
//         DeInitialize();
//     } 

//     void DeInitialize() override {
//         std::cout << "DeInitializing DerivedCls for " << mName << std::endl;
//     }
// };

// void abs_cls_main() {
//     DerivedCls transfer("Example");
// }


// Jixxy's example
#include <iostream>

// Base class using CRTP (curiously reocurring template pattern)
template <typename Derived>
class Connectable {
public:
    // Makes connectImpl "pure virtual"
    Connectable() {
        // Automatically call connect when object is constructed
        static_cast<Derived*>(this)->connectImpl();
    }

    // Makes disconnectImpl "pure virtual"
    ~Connectable() {
        // Automatically call disconnect when object is destroyed
        static_cast<Derived*>(this)->disconnectImpl();
    }

    // Prevent copying to avoid confusion with connect/disconnect
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;

    // virtual void connectImpl() = 0;
    // virtual void disconnectImpl() = 0;

};

// Derived class 1
class DeviceA : public Connectable<DeviceA> {
public:
    DeviceA() { std::cout << "DeviceA constructed\n"; }
    ~DeviceA() { std::cout << "DeviceA destroyed\n"; }

    void connectImpl() {
        std::cout << "DeviceA connecting\n";
    }

    void disconnectImpl() {
        std::cout << "DeviceA disconnecting\n";
        mValue = 0;
        getValue();
    }

    int getValue() const { return mValue; }

    int mValue = 1;
};

// Derived class 2
class DeviceB : public Connectable<DeviceB> {
public:
    DeviceB() { std::cout << "DeviceB constructed\n"; }
    ~DeviceB() { std::cout << "DeviceB destroyed\n"; }

    void connectImpl() {
        std::cout << "DeviceB connecting\n";
    }

    void disconnectImpl() {
        std::cout << "DeviceB disconnecting\n";
        mValue = 0;
    }

    int mValue = 2;
};

void abs_cls_main() {

    DeviceA a;
    std::cout << "---\n";
    DeviceB b;
// Destruction happens here
}