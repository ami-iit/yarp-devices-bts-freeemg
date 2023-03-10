# yarp-devices-bts-freeemg
YARP wearable device implementation for the BTS FREEEMG system

## Installation (Windows only)

Dependencies:
- [`YARP`](https://github.com/robotology/yarp)
- FREEEMG SDK: ask the vendor for the libraries

Starting from the directory where you cloned this repo, follow the steps below to install the device:

```bat
:: create the build directory
mkdir build
:: enter the directory
cd build
:: setup the build
:: replace <freeemg_sdk_root_directory> with your specific directory
cmake -G"Visual Studio 17 2022" .. -DFREEEMG_SDK_DIR=<freeemg_sdk_root_directory> -DCMAKE_INSTALL_PREFIX="./install"
:: build and install
cmake --build . --config Release --target INSTALL
```
>:warning: Change the visual studio version according to your installed version

>:warning: You can also set the FREEEMG_SDK_DIR environment variable instead of passing it to the cmake command

These will install the device and related files in the subdirectory `build/install`.

Finally, make the device available for YARP to use it by updating the following environment variables:

- `PATH`  
  Add `<installation-directory>\lib\yarp`
- `YARP_DATA_DIRS`  
  Add `<installation-directory>\share\yarp`
  Add `<installation-directory>\share\FreeEmg`

### Running the device

The device can be open using `yarpdev --device freeemg`. See also the `yarprobotinterface` [configuration files](devices/FreeEmgWearableDevice/conf/README.md) for the parameters that must/can be used.

Maintainers
--------------
This repository is maintained by:

| | |
|:---:|:---:|
| [<img src="https://github.com/RiccardoGrieco.png" width="40">](https://github.com/RiccardoGrieco) | [@RiccardoGrieco](https://github.com/RiccardoGrieco) |
