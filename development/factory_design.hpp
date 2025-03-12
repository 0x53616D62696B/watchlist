#ifndef FACTORY_DESIGN_HPP
#define FACTORY_DESIGN_HPP

#include <iostream>
#include <memory>
#include <string>

// Base IDAQTransfer class
class IDAQTransfer {
public:
    virtual void show() = 0;
    virtual ~IDAQTransfer() = default;
};

// Concrete DAQTransferA class
class DAQTransferA : public IDAQTransfer {
public:
    void show() override;
};

// Concrete DAQTransferB class
class DAQTransferB : public IDAQTransfer {
public:
    void show() override;
};

// Singleton DAQTransferFactory class
class DAQTransferFactory {
public:
    static DAQTransferFactory& getInstance();
    static std::unique_ptr<IDAQTransfer> createChannel(const std::string& channelType);

private:
    DAQTransferFactory() {}
    DAQTransferFactory(const DAQTransferFactory&) = delete;
    DAQTransferFactory& operator=(const DAQTransferFactory&) = delete;
};

#endif // FACTORY_DESIGN_HPP