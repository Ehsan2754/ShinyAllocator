# ShinyAllocator 
![build](https://github.com/Ehsan2754/ShinyAllocator/actions/workflows/build.yml/badge.svg)  ![unittest](https://github.com/Ehsan2754/ShinyAllocator/actions/workflows/unittest.yml/badge.svg) [![docs](https://github.com/Ehsan2754/ShinyAllocator/actions/workflows/docs.yml/badge.svg)](https://ehsan2754.github.io/ShinyAllocator/)

ShinyAllocator is  a library of a block memory allocator for real-time high-integrity embedded systems. 

# [Documentation](https://ehsan2754.github.io/ShinyAllocator/)

## Static Dependency UML Diagram

### Source and Header Dependencies
![shinyAllocator_8c__incl](https://user-images.githubusercontent.com/53513242/210043414-0d941499-2be5-4dd8-81a0-7960a076f36e.png)

### Allocator Instance
![structshinyAllocatorInstance__coll__graph](https://user-images.githubusercontent.com/53513242/210043502-7475e3ad-b502-4e98-96b4-1fa5888abeb4.png)

### Fragment/FragmentHeader  Struct
![structFragment__coll__graph](https://user-images.githubusercontent.com/53513242/210043323-946f4b6e-1e16-4744-a124-082f9cb190d0.png)

### UnitTest
![unitTests_8cc__incl](https://user-images.githubusercontent.com/53513242/210043192-32153b5f-0f06-44b3-8cfc-1530975b19cd.png)


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
