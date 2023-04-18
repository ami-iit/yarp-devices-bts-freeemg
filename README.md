# yarp-devices-bts-freeemg
YARP wearable device implementation for the BTS FREEEMG system

## Installation (Windows only)

Dependencies:
- [`YARP`](https://github.com/robotology/yarp)
- `IWear` from [`wearables`](https://github.com/robotology/wearables)
- FREEEMG SDK: ask the vendor for the libraries
- `.Net Framework v4`

Starting from the directory where you cloned this repo, follow the steps below to install the device:

>:warning::warning: The installation procedure (and only the installation) requires Administrator privileges. Open the terminal with right-click -> Run as administrator :warning::warning:

```bat
:: create the build directory
mkdir build
:: enter the directory
cd build
:: set the variable FREEEMG_SDK_DIR
:: replace <freeemg_sdk_root_directory> with your specific directory. Beware the spaces in the path! Enclose it in ""
set FREEEMG_SDK_DIR=<freeemg_sdk_root_directory>
cmake -G"Visual Studio 17 2022" .. -DCMAKE_INSTALL_PREFIX="./install"
:: build and install
cmake --build . --config Release --target INSTALL
```
>:warning: Change the Visual Studio version according to your installed version

>:warning: You can also set FREEEMG_SDK_DIR as environment variable before running the instructions above

These will install the device and related files in the subdirectory `build/install`.

Finally, make the device available for YARP to use it by updating the following environment variables:

- `PATH`  
  Add `<installation-directory>\lib\yarp`
- `YARP_DATA_DIRS`  
  Add `<installation-directory>\share\yarp`  
  Add `<installation-directory>\share\FreeEmg`

### Running the device

The device can be open using `yarpdev --device freeemg` (`yarpserver` must be running). See also the `yarprobotinterface` [configuration files](devices/FreeEmgWearableDevice/conf/README.md) for the parameters that must/can be used.

### Applications

There are some default applications that can be run using the device. You can check them out [here](devices/FreeEmgWearableDevice/applications).

Maintainers
--------------
This repository is maintained by:

| | |
|:---:|:---:|
| [<img src="https://github.com/RiccardoGrieco.png" width="40">](https://github.com/RiccardoGrieco) | [@RiccardoGrieco](https://github.com/RiccardoGrieco) |
