#ifndef FACTORY_DESIGN_HPP
#define FACTORY_DESIGN_HPP

#include <iostream>
#include <memory>
#include <string>

// Base IDAQDataTransfer class
class IDAQDataTransfer {
public:
    IDAQDataTransfer(const std::string& name);
    virtual ~IDAQDataTransfer() = default;
    virtual void show() = 0;
    std::string mName;
};

// Concrete DAQDataTransferA class
class DAQDataTransferA : public IDAQDataTransfer {
public:
    using IDAQDataTransfer::IDAQDataTransfer;
    void show() override;
};

// Concrete DAQDataTransferB class
class DAQDataTransferB : public IDAQDataTransfer {
public:
    using IDAQDataTransfer::IDAQDataTransfer;
    void show() override;
};

// Singleton DAQDataTransferFactory class
class DAQDataTransferFactory {
public:
    static DAQDataTransferFactory& getInstance();
    static std::unique_ptr<IDAQDataTransfer> CreateChannel(const std::string& name, 
                                                           const std::string& channelType, 
                                                           const std::string& mmap_path);
    static std::unique_ptr<IDAQDataTransfer> CreateChannel(const std::string& name, 
                                                           const std::string& channelType, 
                                                           const std::string& mmap_path,
                                                           const std::string& fifo_path);


private:
    DAQDataTransferFactory() {}
    DAQDataTransferFactory(const DAQDataTransferFactory&) = delete;
    DAQDataTransferFactory& operator=(const DAQDataTransferFactory&) = delete;
};

#endif // FACTORY_DESIGN_HPP