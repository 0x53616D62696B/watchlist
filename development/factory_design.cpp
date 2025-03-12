#include <iostream>
#include <memory>

class Product {
public:
    virtual void show() = 0;
};

class ProductA : public Product {
public:
    void show() override {
        std::cout << "This is Product A\n";
    }
};

class ProductB : public Product {
public:
    void show() override {
        std::cout << "This is Product B\n";
    }
};

class ProductFactory {
public:
    static std::unique_ptr<Product> createProduct(const std::string& productType) {
        if (productType == "A")
            return std::unique_ptr<Product>(new ProductA());
        else if (productType == "B")
            return std::unique_ptr<Product>(new ProductB());
        else
            return nullptr;
    }
};


class IDAQTransfer {
public:
    virtual void show() = 0;
};

class DAQTransferA : public IDAQTransfer {
public:
    void show() override {
        std::cout << "This is Channel A\n";
    }
};

class DAQTransferB : public IDAQTransfer {
public:
    void show() override {
        std::cout << "This is Channel B\n";
    }
};

class DAQTransferFactory {
public:
    //* Singleton pattern instance
    static DAQTransferFactory& getInstance() {
        static DAQTransferFactory instance;
        return instance;
    }

    static std::unique_ptr<IDAQTransfer> createChannel(const std::string& channelType) {
        if (channelType == "A")
            return std::unique_ptr<IDAQTransfer>(new DAQTransferA());
        else if (channelType == "B")
            return std::unique_ptr<IDAQTransfer>(new DAQTransferB());
        else
            return nullptr;
    }

private:
    //* Singleton pattern
    // Private constructor to prevent instantiation
    DAQTransferFactory() {}
    // Delete copy constructor and assignment operator
    DAQTransferFactory(const DAQTransferFactory&) = delete;
    DAQTransferFactory& operator=(const DAQTransferFactory&) = delete;

};