# Smart Tiles firmware


## Overview 

This demo is relevant using M3 nodes on the Grenoble site located on
corridor floor. The firmware on each M3 nodes located on a tile sends
information on serial link about the detection of a mobile node
(turtlebot2) or human steps. The detection is done using
accelerometers (for human detection) or magnetometers (for robot
detection) or radio on IoT-LAB node tiles.

## Prerequisites

- Tutorial : [Submit an experiment with M3 nodes using the web portal](https://www.iot-lab.info/tutorials/submit-an-experiment-with-web-portal-and-m3-nodes)
- Tutorial : [Get and compile a M3 Firmware code](https://www.iot-lab.info/tutorials/get-compile-a-m3-firmware-code/)
- Tutorial : [Experiment CLI client](https://www.iot-lab.info/tutorials/experiment-cli-client/)
- Tutorial : [Nodes Serial Link Aggregation](https://www.iot-lab.info/tutorials/nodes-serial-link-aggregation/)


## Code

There is two firmwares on this repository:
- smart_tiles 
    - ``smart_tiles.c``: periodic sensors acquisition, detection and print values on serial link
    - ``detectpeak.c/h``: peak and state detection algorithm used by smart_tiles.c
    - ``radio_recv.c``: packet receive callback used by smart_tile
- radio_pinger to be worn by robot or human
    - ``radio_pinger.c``: send a ping message "Robot!" every 0.5 sec
    - ``radio.c/h``: send packet used by radio_pinger.c

## Running

#### Build Firmware 

```
 $ git clone https://github.com/iot-lab/openlab.git
 $ cd openlab/ && mkdir build.m3
 $ cd build.m3/ && cmake .. -DPLATFORM=iotlab-m3
 $ make smart_tiles radio_pinger
```

#### Deploy firmware on nodes

The firmware deployment can be carried out conventionally using web-portal
([tutorial](https://www.iot-lab.info/tutorials/submit-an-experiment-with-web-portal-and-m3-nodes)) or CLI-tools
([tutorial](https://www.iot-lab.info/tutorials/experiment-cli-client/))

You can also deploy more easily the firmware using the dedicated web application used for a complete demo with GUI (see [demo](https://github.com/iot-lab/iot-lab/wiki/Running-the-Smart-Tiles-Demo))

#### smart_tiles : Sensors value on serial link

When the deployement is ok, you can connect to the nodes serial link using serial aggregator (see [tutorial](https://www.iot-lab.info/tutorials/nodes-serial-link-aggregation/))

Then, you can observe the serial aggregator output :
```
...
1413815115.101462;m3-30;Acc;-1.21046655E-1;6.746863E-2;-9.7928727E-1
1413815115.101995;m3-26;AccPeak;1.0002241;0.0;0.0
1413815115.102325;m3-26;Acc;-8.020561E-2;3.8617518E-2;-9.911829E-1
1413815115.102878;m3-27;Acc;-7.8868106E-2;9.983304E-3;-9.8335546E-1
...
```

The format of a line is :
```
gateway timestamps;id-node;SensorType;value1;value2;value3;
```

Where SensorType is a sensor value, printed each second:
- ``Acc`` (Accelerometer) with value1/2/3 : acceleration in on X/Y/Z
- ``Mag`` (Magnetometer) with value1/2/3 : magnetic field on X/Y/Z
- ``Gyr`` (Gyrometer) with value1/2/3 : speed rotation on X/Y/Z

Or information printed when a detection is done:
- ``AccPeak`` is an acceleration peak detection : value1 = algo peak output, value2/3= 0.0
- ``MagPeak`` is a magnetometer state change detection : value1 = algo state change output, value2/3= 0.0

When a packet radio is received, the message printed is :
``Radio recv: from=<node_id> data="Robot!"\n"``

The output can be saved on a file and plotted using ``plot_sensors.py``

```
plot sensors  values from <node_id> printed by smart_tiles firmware 
saved in filename (by serial_aggregator)

Example of use :

After firmware deployement on m3-29 to m3-32
mypc> aggr.sh 29 30 31 32 > data.txt
mypc> python myplot.py data.txt 29 30 31 32
```

#### radio_pinger 

The node must be worn by robot (iot-lab node) or human (agile-fox for example) and send every 0.5 second a radio string message.
Then, this mobile node can be detected by the nodes with smart_tiles firmware.

## Smart Tiles Demo

A complete demo using this firmware is described on the [wiki iotlab github](https://github.com/iot-lab/iot-lab/wiki/Running-the-Smart-Tiles-Demo)
