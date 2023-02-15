# Edge Impulse Example: stand-alone inferencing (Alif)

This builds and runs an exported impulse locally on your machine. 

## Prerequisites
1. Create an edge impulse account at [edgeimpulse.com](https://www.edgeimpulse.com/)
2. Install the latest `Alif Security Toolkit`:

    * Navigate to the [Alif Semiconductor Kit documentation](https://alifsemi.com/kits) page (you will need to register to create an account or log in). and download the latest `App Security Toolkit` (tested with version 0.54.0) for windows or linux. If you are using MacOS, download the linux version.
    * Extract archive and place `app-release` in the root of this repo
    * Follow the instructions in local the `Alif Security Toolkit Quickstart Guide` to finalize the installation.
3. Install either `docker`, arm gcc, or arm clang: 

### gcc
Tested with:
```
gcc version 10.2.1 20201103 (release) (GNU Arm Embedded Toolchain 10-2020-q4-major)
```

### clang
[ARM clang compiler](https://developer.arm.com/tools-and-software/embedded/arm-compiler/downloads/version-6) and ensure it is added to your path:
```
which armclang
```

## Downloading your Edge Impulse model
Using an Edge Impulse project navigate to the **Deployment** tab. Here you should see an `Ethos-U library` deployment option. Select this and click **build** to download a zip file of the contents. You may also download a prebuilt deployment using this repository by selecting the `Alif Ensemble E7` option under **Development Kits**.

## Build the firmware 


### with Docker

This repository contains a docker image with the required dependencies for building with gcc:
```
docker build -t alif-firmware .
```

Build the the firmware with following command:
```
docker run --rm -it -v "${PWD}":/app alif-firmware /bin/bash -c "sh build.sh"
```

### With your local gcc or armclang toolchain 
1. Extract the zip file downloaded from edge impulse into the `source` directory of this repository
2. Choose one of the following:
    1. run cmake .. -DTARGET_SUBSYSTEM=HP to target Cortex M55 core 0 (high performance) OR
    2. run cmake .. -DTARGET_SUBSYSTEM=HE to target Cortex M55 core 1 (high efficiency)
3. If you wish to use gcc, add the cmake flag: -DCMAKE_TOOLCHAIN_FILE=../scripts/cmake/toolchains/bare-metal-gcc.cmake
4. armclang is the default toolchain file (-DCMAKE_TOOLCHAIN_FILE=../scripts/cmake/toolchains/bare-metal-armclang.cmake)
5. run make to build to app.axf
6. Example:

```
mkdir build
cd build
cmake .. -DTARGET_SUBSYSTEM=HP -DCMAKE_TOOLCHAIN_FILE=../scripts/cmake/toolchains/bare-metal-gcc.cmake -G"Unix Makefiles"
make -j8
```

### Flash your device
0. Ensure your board is configured for programming over the SEUART interface by following the [Edge Impulse Ensemble E7 setup guide](https://docs.edgeimpulse.com/docs/development-platforms/officially-supported-mcu-targets/alif-ensemble-e7#connecting-to-edge-impulse)
1. Copy `./build/bin/app.bin` into the Alif Security Toolkit directory `./app-release/build/images`
2. Copy the `ei-app-gcc.json` file into the Alif Security Toolkit directory `./app-release/build/config`
3. Execute the following commands to program your board:

```
cd app-release
./app-gen-toc -f ./build/config/ei-app-gcc.json
./app-write-mram.py -d
```

When prompted to select the serial interface to program over. Select the corresponding the FTDI COM interface.

Once programmed, the firmware will run inference and print the results over the UART2 serial port.

### Serial out is on UART2, which is connected to baseboard pins P3_16,P3_17 (on connector J413).
- Baud 115200
- "8,N,1" (the typical settings for everything else, 8 bit, no parity, 1 stop bit)
- 1.8 Vccf you are using the Alif Dev kit, see Alif `AN0002` for instructions on flashing your Alif development kit with `build/bin/app.axf`

Once programmed, the firmware will run inference and print the results over the serial port.

## Serial out is on UART2, which is connected to pins P3_16,P3_17 (on connector J413)

- Baud 115200
- "8,N,1" (the typical settings for everything else, 8 bit, no parity, 1 stop bit)
- 1.8 Vcc

# Other details

## Timing

Timing calculations are performed in [ei_classifier_porting.cpp](source/ei_classifier_porting.cpp) and make use of an interrupt attached to SysTick.
- An RTOS may take over this interrupt handler, in which case you should reimplement ei_read_timer_us and _ms.
- The default calculation is based on the default clock rates of the Alif dev kit (400 MHz for HP core, 160 MHz for HE core).  If you change this, redefine EI_CORE_CLOCK_HZ.

## Memory placement

Alif M55 processors have a private fast DTCM, and also access to a larger, but slower, chip global SRAM.
- For armclang the linker file attempts to place as much as possible in DTCM, and overflows into SRAM if needed.
- For gcc, the linker is unable to auto place based on size.  If you get an error during link, see [The linker file for GCC, arena placement](ensemble.ld#L116) and uncomment the line that places the arena in SRAM (line 116) (instead of DTCM).  This will only slow down DSP, as the U55 has to use the SRAM bus to access the model regardless of placement.

When your entire program can't fit into DTCM, sometimes customizing placement of objects can improve performance.
See [ensemble.sct](ensemble.sct) for example placement commands
