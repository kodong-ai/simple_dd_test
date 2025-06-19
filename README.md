# SimpleTest01

SimpleTest01 is a C++ console application designed to test a Windows Device Driver's DMA buffer interface. It reads a specified input file into a DMA buffer of up to 1 MB, processes the file in 256 KB chunks, and issues an IOCTL trigger when the buffer is full or the transfer is complete.

---

## Features

- Reads and transfers an input file using the `-input <filename>` option
- Requests a DMA buffer allocation of up to 1 MB from the driver
- Splits the file into 256 KB chunks for transfer
- Calls `IOCTL_TRIGGER_DMA` when the buffer is full or after the final chunk

## Requirements

- Windows 10 or later (x64)
- Visual Studio 2022
- Administrator privileges (required to access `\\.\DeviceNode`)

## Project Structure

SimpleTest01/
├─ SimpleTest01.vcxproj # VS2022 project file
└─ main.cpp # Application source code


## Build Instructions

### Using Visual Studio IDE

1. Open `SimpleTest01.sln` in Visual Studio 2022  
2. Select Configuration: `Release`, Platform: `x64`  
3. Build → Build Solution (Ctrl+Shift+B)

### Using Command Line

```bat
REM Run Developer Command Prompt for VS 2022 as Administrator
cd <project_directory>
msbuild SimpleTest01.vcxproj /p:Configuration=Release /p:Platform=x64
Usage
SimpleTest01.exe -input C:\path\to\yourfile.bin
Use -input to specify the binary file to transfer.

Run as administrator to access the device node.

IOCTL Definitions
Name	Definition
IOCTL_ALLOC_DMA_BUFFER

CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_TRIGGER_DMA

CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

Update CDI_NODE_NAME and IOCTL codes to match your driver specifications.

Contributing
Please open issues and feature requests on GitHub Issues. Pull requests are welcome.

License
MIT License
