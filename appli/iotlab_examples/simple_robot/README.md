# Simple robot firmware

## Overview 

This example shows how to use a mobile node embedded on a robot (Turtlebot 2 or Wifibot) with predefined trajectories. The code is here: 

https://github.com/iot-lab/openlab/tree/master/appli/iotlab_examples/simple_robot

This firmware is used in the tutorial:

https://www.iot-lab.info/tutorials/submit-an-experiment-with-a-mobile-m3-nodes-using-the-web-portal/


## Prerequisites

- Tutorial : [Submit an experiment with a mobile M3 nodes using the web portal](https://www.iot-lab.info/tutorials/submit-an-experiment-with-a-mobile-m3-nodes-using-the-web-portal/)

## Code

- ``main.c``: periodic IMU sensors acquisition (accelerometer, magnetometer, gyrometer) and print values on serial link every 1 second.
    
## Running

#### Build Firmware 

```
 $ git clone https://github.com/iot-lab/openlab.git
 $ cd openlab/ && mkdir build.m3
 $ cd build.m3/ && cmake .. -DPLATFORM=iotlab-m3
 $ make simple_robot
```

#### Deploy firmware on nodes

The firmware deployment can be carried out conventionally using web-portal
on a mobile node see ([tutorial](https://www.iot-lab.info/tutorials/submit-an-experiment-with-a-mobile-m3-nodes-using-the-web-portal/)).

#### simple_robot

When the deployement is ok, you can print the mobile node serial link,
for example on Grenoble site:

```
ssh <login>@grenoble.iot-lab.info 
<login>@grenoble:~$ nc 20000 m3-381
```

You will see :

```
...
Acc;-4.8E-2;-4.3E-2;8.880001E-1
Gyr;7.9625E-1;-1.19875;-3.85E-1
Mag;-1.82E2;6.8E1;-8.5E1
Ang;-287
Acc;-1.1000001E-2;1E-3;1.0710001
Gyr;-9.1E-1;-1.225E-1;-6.65E-1
Mag;-1.86E2;7.1E1;-7.9E1
Ang;-301
Acc;-1.0000001E-2;4E-3;1.041
Gyr;7.525E-1;2.45E-1;-1.5575
...
```

The format of a line is :
```
SensorType;Value1;[..;ValueN]
```

Where SensorType is a sensor value, printed each second:
- ``Acc`` (Accelerometer) with value1/2/3 : acceleration in on X/Y/Z
- ``Mag`` (Magnetometer) with value1/2/3 : magnetic field on X/Y/Z
- ``Gyr`` (Gyrometer) with value1/2/3 : speed rotation on X/Y/Z
- ``Ang`` (Robot angle tilt) with value1 in degree


The output can be saved on a file (e.g. ``nc 20000 m3-381 > robot.txt``) and plotted
using ``python plot_imu.py robot.txt``


## Demo with animated plot

A complete demo using this firmware is described on the [wiki iotlab github](https://www.iot-lab.info/tutorials/submit-an-experiment-with-a-mobile-m3-nodes-using-the-web-portal/)
