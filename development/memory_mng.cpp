
#include <iostream>
#include <memory>
#include <cstring>
#include "memory_mng.hpp"


namespace MemoryManagement {


    void memory_mng_main(char aCmd, void* aXchangeData){
        //* Concatenate data buffer
        //unique_ptr concatenated_buf = aCmd + aXchangeData;

        // Allocate memory for char + hermes_usb_acqu_setup
        void* rawMemory = malloc(sizeof(char) + sizeof(hermes_usb_acqu_setup));
        if (!rawMemory) {
            std::cerr << "Memory allocation failed" << std::endl;
            return; //todo Exception raise
        }

        // Create a unique pointer with the custom deleter
        std::unique_ptr<void, CustomDeleter> concatenatedBuf(rawMemory);

        // Initialize the char part
        char* cmd = static_cast<char*>(rawMemory); //? concatenatedBuf instead of rawMem?
        //char* aCmd = static_cast<char*>(concatenatedBuf);
        *cmd = aCmd; // cmd init

        // Initialize the hermes_usb_acqu_setup part
        hermes_usb_acqu_setup* mem_struct = reinterpret_cast<hermes_usb_acqu_setup*>(cmd + 1); //why no rawMemory or unique_ptr?
        mem_struct->id = 42; // Example initialization

        // Access and use the data
        std::cout << "aCmd: " << *cmd << std::endl;
        std::cout << "setup->someField: " << mem_struct->id << std::endl;

        // Memory will be automatically freed when concatenatedBuf goes out of scope
        
        //* Concatenate data buffer end
    }
    

} // namespace MemoryManagement