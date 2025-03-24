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

#include "pure_virtual_method_in_abs_cls.hpp"


ConnectableBase::ConnectableBase(const std::string& aName):mName(aName) {
    std::cout << "ConnectableBase constructed with name " << aName << "\n"; 
}
ConnectableBase::~ConnectableBase() { std::cout << "ConnectableBase destroyed\n"; }


// Base class using CRTP (curiously reocurring template pattern)

// Makes connectImpl "pure virtual" - has to be defined in derived class
template <typename Derived>
Connectable<Derived>::Connectable(const std::string& aName): ConnectableBase(aName) {
    // Automatically call connect when object is constructed
    static_cast<Derived*>(this)->connectImpl();
    std::cout << "Constructed name is: " << static_cast<Derived*>(this)->mName << std::endl;
}

// Makes disconnectImpl "pure virtual" - has to be defined in derived class
template <typename Derived>
Connectable<Derived>::~Connectable() {
    // Automatically call disconnect when object is destroyed
    static_cast<Derived*>(this)->disconnectImpl();
}


// Derived class 1
DeviceA::DeviceA(const std::string& aName):Connectable(aName) { std::cout << "DeviceA constructed\n"; }
DeviceA::~DeviceA() { std::cout << "DeviceA destroyed\n"; }

void DeviceA::connectImpl() {
    std::cout << "DeviceA connecting\n";
    mValue = 4;
    getValue();
}

void DeviceA::disconnectImpl() {
    std::cout << "DeviceA disconnecting\n";
    mValue = 0;
    getValue();
    std::cout << "DeviceA name is:" << mName << "\n";
}


// Derived class 2

DeviceB::DeviceB(const std::string& aName):Connectable(aName) { std::cout << "DeviceB constructed\n"; }
DeviceB::~DeviceB() { std::cout << "DeviceB destroyed\n"; }

void DeviceB::connectImpl() {
    std::cout << "DeviceB connecting\n";
}

void DeviceB::disconnectImpl() {
    std::cout << "DeviceB disconnecting\n";
    mValue = 0;
    std::cout << "DeviceB name is:" << mName << "\n";
}

std::unique_ptr<DeviceA> get_device(const std::string& aName){
    // return std::make_unique<DeviceA>(aName);
    return std::unique_ptr<DeviceA>(new DeviceA(aName));
}

void abs_cls_main() {
    std::unique_ptr<DeviceA> dev = get_device("DeviceA");
    std::cout << "---\n";
    DeviceB b("DeviceB");
    // Destruction happens here
}