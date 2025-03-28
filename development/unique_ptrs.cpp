#include <cstdint>
#include <memory>
#include <iostream>

#include <typeinfo>

using u32 = uint32_t;

typedef struct hermes_usb_acqu_setup
{
	u32 id;
	u32 address;

} hermes_usb_acqu_setup;

typedef struct AcquSetupBuffer
{
    char cmd;
    hermes_usb_acqu_setup data;
} AcquSetupBuffer;

template <typename AcquBufferStruct>
// std::unique_ptr<AcquBufferStruct> SetupBuffer(char aCmd, void* aXchangeData) //* Or this possibility with raw pointer param
std::unique_ptr<AcquBufferStruct> SetupBuffer(char aCmd, decltype(AcquBufferStruct::data)* aXchangeData)
{
    //* Concatenate data buffer
    // Allocate memory for char + hermes_usb_acqu_setup 
    std::unique_ptr<AcquBufferStruct> concatenatedBuf = std::unique_ptr<AcquBufferStruct>(new AcquBufferStruct);

    //* Cpy aXchangeData into position for data
    concatenatedBuf->data = *(reinterpret_cast<decltype(AcquBufferStruct::data)*>(aXchangeData));

    //* Initialize cmd
    concatenatedBuf->cmd = aCmd;

    // Memory will be automatically freed when concatenatedBuf goes out of scope
    //* Concatenate data buffer end
    return concatenatedBuf;
}


void main_unique_ptrs(){
    hermes_usb_acqu_setup* acqu_setup = new hermes_usb_acqu_setup;
    std::unique_ptr<AcquSetupBuffer> my_buff = SetupBuffer<AcquSetupBuffer>('a', acqu_setup);
    std::cout << "my_buff->cmd: " << my_buff->data.id << std::endl;
    std::cout << "my_buff.get()->cmd: " << my_buff.get()->cmd << std::endl; // Same
    std::cout << "&(my_buff->cmd): " << &(my_buff->cmd) << std::endl; // Pointer to cmd - strange output because it is not initialized
    std::cout << "&(my_buff->data): " << &(my_buff->data) << std::endl; //Pointer to data
    std::cout << "Size of data member in AcquSetupBuffer: " << sizeof(AcquSetupBuffer::data) << std::endl;
} 