/**
 * @file Watchlist.cpp
 * @author Patrik Maraczek (https://github.com/0x53616D62696B)
 * @brief 
 * @version 0.1
 * @date 2022-08-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Utils.hpp"

#include <iostream>

  
// A dummy function
void foo(int Z)
{
    for (int i = 0; i < Z; i++) {
        std::cout << "Thread using function"
               " pointer as callable\n";
    }
}
  

int main()
{
    std::cout << "Test" << std::endl;



    return EXIT_SUCCESS;
}