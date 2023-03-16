# FreeEMG device - configuration files

Here we store YARP configuration files to use the FreeEMG wearable device.

### Publish data

The `yarprobotinterface` configuration file [freeEmg](freeEmg.xml) opens the wearable device and publishes the data on a YARP port as a [WearableData](https://github.com/robotology/wearables/blob/a3a20863161e755f55277f204ca206764c3ed5f0/msgs/thrift/WearableData.thrift) messages. 

List of overridable parameters when launching `yarprobotinterface`:

| Param name | Description |Required | Example/default | 
|-------|---|----|------|
| period  | Device frequency in seconds | :x: | 0.01|
| COM-ports | List of BMs COM ports| :heavy_check_mark: | (26) |
| labelList | List of probe labels to expose. Must have the same size as `sensorNames` | :heavy_check_mark: | (1 2) |
| sensorNames | List of the names associated to the probes. Must have the same size as `labelList` | :heavy_check_mark: | (r_tib_lat r_gastro_med) |
| useCustomProfile | Flag to set a custom sensor dictionary. If set to true, the parameter profile must be added (see file for an example) | :heavy_check_mark: | false |
| dataPortName | Name of the YARP port where data is published | :x: | /FreeEmg/WearableData/data:o | 

To run the configuration:

```bat
yarprobotinterface --config freeemg.xml
```

### Log or stream analog data

The `yarprobotinterface` configuration file [`freeEMG_logger`](freeEMG_logger.xml) allows for two purposes:

1. Log the data as a `.mat` file 
2. Stream the data as analog values

>:warning: This configuration file requires the data to being already streamed on a YARP port (e.g. using [the provided configuration file](#publish-data)).

List of overridable parameters when launching `yarprobotinterface`:

| Param name | Description |Required | Example/default | 
|-------|---|----|------|
| period  | Log/stream frequency | :x: | 0.01 |
| wearableDataPorts | YARP port(s) where data is being streamed | :x: | /FreeEmg/WearableData/data:o |
| loggerType | `matlab` if you want to log the data, `yarp` if you want to stream it | :x: | (matlab) |
| experimentName| Prefix of the file(s) created by the logger | :x: |  wearable_data |

>:warning: The logger will create the file(s) in the working directory where the process using the file is running.

In streaming mode, the process will create one YARP port per sensor (probe) with the format `/FreeEmg/emg/<sensor_name>`, where the sensor names are the one configured in the wearable device via the `sensorNames` parameter.

To run the logger:

```bat
yarprobotinterface --config freeEMG_logger.xml
```

### Visualize EMG signals

The file [`freeEMG_scope`](freeEMG_scope.xml) is a `yarpscope` configuration file for visualizing the EMG signals streamed by the device.

In order to run the device, the values must be streamed in analog ports like the ones provided by the [related configuration file](#log-or-stream-analog-data).

