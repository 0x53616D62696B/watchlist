#pragma once

#include <vector>

namespace DevTests{

void print_compiler_version();
// Allocate memory and initialize the retrieved_msg_char array with all elements set to 0
extern const char* retrieved_msg_char; //* Has to be extern because it is included in multiple translation units. 
                                       //*   Definition completed in dev_tests.cpp

extern std::vector<std::uint8_t> mBuffer;

void dev_tests_main();

} // namespace DevTests