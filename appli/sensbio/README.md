=================
Sensbio firmware
=================

Overview
--------

[Sensbio INRIA project](http://sensas.gforge.inria.fr/wiki/doku.php?id=sensbio) 
aims to propose rehabilitation and bio-logging applications based on
wireless sensor and actuator network nodes. Sensbio proposes a toolkit
named sensbioTk which consists in :
  * hardware wireless sensor nodes : [HikoB Fox nodes](http://www.hikob.com/assets/uploads/2014/07/HIKOB_FOX_ProductSheet_EN.pdf)
  * firmware dedicated to these nodes based on [Openlab](https://github.com/iot-lab/openlab)
  * software in python to collect and process sensor data on a PC connected to a sink node.

This part concerns the Sensbio firmware.

Directories
-----------

The firmware provided:

# banet
body-area network dedicated to collect IMU and sensors data based on
the TDMA mac protocol.

# synchro_in  
firmware for a FOX with a daughter board to manage digital input
synchronization

# synchro_out
firmware for a FOX with a daughter board to manage digital output
synchronization

# pedometer
The Fox Pedometer system provides a wearable solution for counting the
number of walking steps and detecting free-fall situations.