Welcome to OpenLab for IoT-LAB !

Supported platforms:
- iotlab-m3
- iotlab-a8-m3

Requirements:
- gcc-toolchain: https://launchpad.net/gcc-arm-embedded
- cmake
- gcc and g++ installed (not used, but cmake checks these)

Basic setup:
- ``$ mkdir build.m3 && cd build.m3 && cmake .. -DPLATFORM=iotlab-m3 ``
- ``$ mkdir build.a8 && cd build.a8 && cmake .. -DPLATFORM=iotlab-a8-m3 ``

Further doc:
- README_COMPILING (for local usage of OpenLab)

Note:
- OpenLab is required by Contiki for IoT-LAB
