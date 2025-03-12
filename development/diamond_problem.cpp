#include <iostream>

// Base class
class Base {
public:
    virtual void show() {
        std::cout << "Base class" << std::endl;
    }
};

// Derived classes using virtual inheritance
class Derived1 : virtual public Base {
public:
    void show() override {
        std::cout << "Derived1 class" << std::endl;
    }
};

class Derived2 : virtual public Base {
public:
    void show() override {
        std::cout << "Derived2 class" << std::endl;
    }
};

// Class inheriting from both Derived1 and Derived2
class Final : public Derived1, public Derived2 {
public:
    void show() override {
        std::cout << "Final class" << std::endl;
    }
};

