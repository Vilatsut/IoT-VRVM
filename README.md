# IoT-VRVM
RIOT and FIT-IOT-LAB based example project for learning.

## Cloning the repo
RIOT OS is included to this repository as a submodule. After cloning this repository the submodules must be downloaded using the command
```
git submodule update --init
```

## Building firmware
Correct tools must be instelled to build the RIOT OS firmware. Using Ubuntu-based distro the required tools can be installed using the following command:
```
sudo apt install git gcc-arm-none-eabi make gcc-multilib libstdc++-arm-none-eabi-newlib openocd gdb-multiarch doxygen wget unzip python3-serial
```
To build the firmware for target iotlab-m3 run the following under the 'node' directory:
```
make all
```
To build the firmware for linux (native) use the following command:
```
make all BOARD=native
```