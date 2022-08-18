#!/bin/bash

sudo apt update
sudo apt install gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100
sudo apt install ninja-build
sudo apt install ccache
sudo apt install clang-format
sudo apt install cppcheck
