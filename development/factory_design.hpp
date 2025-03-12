#ifndef FACTORY_DESIGN_HPP
#define FACTORY_DESIGN_HPP

#include <iostream>
#include <memory>
#include <string>

// Base IDAQTransfer class
class IDAQTransfer {
public:
    IDAQTransfer(const std::string& name);
    virtual ~IDAQTransfer() = default;
    virtual void show() = 0;
    std::string mName;
};

// Concrete DAQTransferA class
class DAQTransferA : public IDAQTransfer {
public:
    using IDAQTransfer::IDAQTransfer;
    void show() override;
};

// Concrete DAQTransferB class
class DAQTransferB : public IDAQTransfer {
public:
    using IDAQTransfer::IDAQTransfer;
    void show() override;
};

// Singleton DAQTransferFactory class
class DAQTransferFactory {
public:
    static DAQTransferFactory& getInstance();
    static std::unique_ptr<IDAQTransfer> createChannel(const std::string& name, const std::string& channelType);

private:
    DAQTransferFactory() {}
    DAQTransferFactory(const DAQTransferFactory&) = delete;
    DAQTransferFactory& operator=(const DAQTransferFactory&) = delete;
};

#endif // FACTORY_DESIGN_HPP