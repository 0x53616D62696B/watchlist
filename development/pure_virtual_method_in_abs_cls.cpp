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

class ConnectableBase {
public:
    ConnectableBase(const std::string& aName):mName(aName) {
        std::cout << "ConnectableBase constructed with name " << aName << "\n"; 
    }
    ~ConnectableBase() { std::cout << "ConnectableBase destroyed\n"; }
    std::string mName;
};

// Base class using CRTP (curiously reocurring template pattern)
template <typename Derived>
class Connectable: public ConnectableBase {
public:
    // Makes connectImpl "pure virtual" - has to be defined in derived class
    Connectable(const std::string& aName): ConnectableBase(aName) {
        // Automatically call connect when object is constructed
        static_cast<Derived*>(this)->connectImpl();
        std::cout << "Constructed name is: " << static_cast<Derived*>(this)->mName << std::endl;
    }

    // Makes disconnectImpl "pure virtual" - has to be defined in derived class
    ~Connectable() {
        // Automatically call disconnect when object is destroyed
        static_cast<Derived*>(this)->disconnectImpl();
    }

    // Prevent copying to avoid confusion with connect/disconnect
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;
};

// Derived class 1
class DeviceA : public Connectable<DeviceA> {
public:
    DeviceA(const std::string& aName):Connectable(aName) { std::cout << "DeviceA constructed\n"; }
    ~DeviceA() { std::cout << "DeviceA destroyed\n"; }

    void connectImpl() {
        std::cout << "DeviceA connecting\n";
        mValue = 4;
        getValue();
    }

    void disconnectImpl() {
        std::cout << "DeviceA disconnecting\n";
        mValue = 0;
        getValue();
        std::cout << "DeviceA name is:" << mName << "\n";
    }

    int getValue() const { return mValue; }

    int mValue = 1; //? Define before implementation?

};

// Derived class 2
class DeviceB : public Connectable<DeviceB> {
public:
    DeviceB(const std::string& aName):Connectable(aName) { std::cout << "DeviceB constructed\n"; }
    ~DeviceB() { std::cout << "DeviceB destroyed\n"; }

    void connectImpl() {
        std::cout << "DeviceB connecting\n";
    }

    void disconnectImpl() {
        std::cout << "DeviceB disconnecting\n";
        mValue = 0;
        std::cout << "DeviceB name is:" << mName << "\n";

    }

    int mValue = 2;
};

void abs_cls_main() {

    DeviceA a("DeviceA");
    std::cout << "---\n";
    DeviceB b("DeviceB");
// Destruction happens here
}