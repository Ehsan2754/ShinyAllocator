# ShinyAllocator
ShinyAllocator is  a library of a block memory allocator for real-time high-integrity embedded systems. 


# Installation [Development]
 0. Choose your Installation path and version of your toolchain in installation [script](./scripts/install_toolchain.sh).
 1. Install GNU Arm Embedded Toolchain
       ```
       chmod +x ./scripts/install_toolchain.sh
       sudo scripts/install_toolchain.sh

       ```
    > script will print two lines to add toolchain directory to your PATH
 2. Install building tools *make,doxygen,graphviz*
       ```
       sudo apt-get update && sudo apt-get upgrade
       sudo apt-get install make cmake doxygen graphviz
       ```
