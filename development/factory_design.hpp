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
    virtual void Show() = 0;
    std::string mName;
};

// Concrete DAQDataTransferA class
class DAQDataTransferA : public IDAQDataTransfer {
public:
    using IDAQDataTransfer::IDAQDataTransfer;
private:
    void Show() override;
};

// Concrete DAQDataTransferB class
class DAQDataTransferB : public IDAQDataTransfer {
public:
    using IDAQDataTransfer::IDAQDataTransfer;
    void Show() override;
};

// Singleton DAQDataTransferFactory class
class DAQDataTransferFactory {
public:
    static DAQDataTransferFactory& GetInstance();
    static std::unique_ptr<IDAQDataTransfer> CreateChannel(const std::string& name, 
                                                           const std::string& channel_type, 
                                                           const std::string& mmap_path);
    static std::unique_ptr<IDAQDataTransfer> CreateChannel(const std::string& name, 
                                                           const std::string& channel_type, 
                                                           const std::string& mmap_path,
                                                           const std::string& fifo_path);

private:
    DAQDataTransferFactory() {}
    DAQDataTransferFactory(const DAQDataTransferFactory&) = delete;
    DAQDataTransferFactory& operator=(const DAQDataTransferFactory&) = delete;
};

#endif // FACTORY_DESIGN_HPP