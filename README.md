# ShinyAllocator ![build](https://github.com/Ehsan2754/ShinyAllocator/actions/workflows/build.yml/badge.svg)  ![unittest](https://github.com/Ehsan2754/ShinyAllocator/actions/workflows/unittest.yml/badge.svg) [![docs](https://github.com/Ehsan2754/ShinyAllocator/actions/workflows/docs.yml/badge.svg)](https://ehsan2754.github.io/ShinyAllocator/)
ShinyAllocator is  a library of a block memory allocator for real-time high-integrity embedded systems. 

# Documentation
https://ehsan2754.github.io/ShinyAllocator/

# Installation [Development]
 0. Choose your Installation path and version of your toolchain in installation [script](./scripts/install_toolchain.sh).
 1. Install GNU Arm Embedded Toolchain
       ```
       chmod +x ./scripts/install_toolchain.sh
       sudo scripts/install_toolchain.sh

       ```
 2. script will print two lines to add toolchain directory to your PATH with default settings you can run
     ```export PATH=$PATH:/usr/share/gcc-arm-none-eabi-10.3-2021.10/bin/ && source ~/.bashrc```
 3. Install build, documentation, debug and profiling tools *make,doxygen,graphviz,valgrind,kcachegrind*
       ```
       sudo apt-get update && sudo apt-get upgrade
       sudo apt-get install make cmake doxygen graphviz valgrind kcachegrind
       ```
1. Installing test framework [ [gtest](https://github.com/google/googletest.git) ]
   * On Ubuntu/RPI:
      ```
      sudo apt-get install libgtest-dev
      cd /usr/src/gtest
      sudo cmake CMakeLists.txt
      sudo make
      sudo cp *.a /usr/lib
      sudo ldconfig
      ```
   * Build from source for your targer platform (arm):
      ```
      sudo apt-get install cmake
      git clone https://github.com/google/googletest.git
      cd googletest
      mkdir build
      cd build
      cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/arm-none-eabi.cmake ..
      make
      sudo make install
      ```
