#include <cstdint>
#include <memory>
#include <iostream>

using u32 = uint32_t;

typedef struct hermes_usb_acqu_setup
{
	u32 id;			/* id returned with event */
	u32 board;		/* board address */
	u32 address;	/* address offset in the acquisition mem to place the data */
	u32 size;		/* expected size in bytes */
	u32 preview_a;	/* preview expected size in bytes, if >0, an event will be generated to wake the application */
	u32 preview_b;	/* preview expected size in bytes, if >0, an event will be generated to wake the application */

} hermes_usb_acqu_setup;

typedef struct AcquSetupBuffer
{
    char cmd;
    hermes_usb_acqu_setup data;
} AcquSetupBuffer;

template <typename AcquBufferStruct>
std::unique_ptr<AcquBufferStruct> SetupBuffer(char aCmd, void* aXchangeData)
{
    //* Concatenate data buffer
    // Allocate memory for char + hermes_usb_acqu_setup 
    std::unique_ptr<AcquBufferStruct> concatenatedBuf = std::unique_ptr<AcquBufferStruct>(new AcquBufferStruct);

    //* Memcpy aXchangeData into 1. position for data
    //reinterpret_cast<AcquBufferStruct*>(concatenatedBuf.get())->data = *reinterpret_cast<hermes_usb_acqu_setup*>(aXchangeData);
    
    //AcquBufferStruct::data* aXchangeDataReinterpreted = reinterpret_cast<AcquBufferStruct::data*>(aXchangeData); //! NEW
    hermes_usb_acqu_setup* aXchangeDataReinterpreted = reinterpret_cast<hermes_usb_acqu_setup*>(aXchangeData); //! NEW

    //memcpy(concatenatedBuf->data, aXchangeDataReinterpreted, sizeof(AcquBufferStruct::data));
    //memcpy(concatenatedBuf->data, aXchangeDataReinterpreted, sizeof(AcquBufferStruct::data));

    //* Initialize cmd
    concatenatedBuf.get()->cmd = aCmd; //? will this work?

    // Memory will be automatically freed when concatenatedBuf goes out of scope
    //* Concatenate data buffer end
    return concatenatedBuf;
}


void main_unique_ptrs(){
    hermes_usb_acqu_setup* acqu_setup = new hermes_usb_acqu_setup;
    std::unique_ptr<AcquSetupBuffer> my_buff = SetupBuffer<AcquSetupBuffer>('a', acqu_setup);
    std::cout << "my_buff->cmd: " << my_buff->cmd << std::endl;
    std::cout << "my_buff.get()->cmd: " << my_buff.get()->cmd << std::endl; // Same
    std::cout << "&(my_buff->cmd): " << &(my_buff->cmd) << std::endl; // Pointer to cmd - strange output because it is not initialized
    std::cout << "&(my_buff->data): " << &(my_buff->data) << std::endl; //Pointer to data
    std::cout << "Size of data member in AcquSetupBuffer: " << sizeof(AcquSetupBuffer::data) << std::endl;
} 