===============
Fox-Pedometer
===============
Date : 16/12/2014

Fox Pedometer firmware and Python driver.

The Fox Pedometer system provides a wearable solution for counting
the number of walking steps and detecting free-fall situations.

The system combines python software (driver), embedded software (firmwares)
and hardware (Fox devices and serial-link adapter). The minimal setup
is constituted by 2 Fox nodes. One node is a pedometer placed on a
person to compute the number of walking steps, the other node is a 'dongle'
connected to a PC or a set-top box through USB. The pedometer node sends
information to the dongle node via radio, the driver code collects data
from the dongle via usb serial link.


Dependencies
============

To run Fox-Pedometer demo and drivers, you will need :

* python_ >= 2.7 (tested with 2.7)
* pyserial_

.. _python: http://python.org
.. _pyserial: http://pyserial.sourceforge.net/

For the hardware, you need two fox nodes programed with firmware :

* pedometer_receive.elf for the dongle node
* pedometer_send.elf for the pedometer node

To build the firmwares, you will need various build-tools.  Check section
"How to build the firmwares" below.

Sources files
=============

Directory dongle/ contains python driver sources file :

* fox_pedometer.py : class to collect Fox pedometer data through USB
* fox_dongle.py    : API class for collecting Fox data 

Current directory contains the sources for the firmwares:
* pedometer_send.c    : pedometer firmware main
* pedometer_receive.c : dongle firmware main

How to build the doc
====================

```
> cd doc; make html
```

How to launch a test
====================

1) Connect the Dongle Fox node on a USB Port
   switch-on. The two leds (orange, green) are blinking

2) Switch-on the Pedometer Fox-node. The two leds (orange, green) 
   are on and steady

3) Verify the configuration file : .hikob.ini
   file is located where the application is launched (pedometer)
   or in $HOME. File is plain ascii and contains:

  node_id = 0x38BC
  period = 1
  silent = 0

4) Launch the test :

  $ export PYTHONPATH=mypath/pedometer/pedometer/

  $ cd pedometer

  $ python fox_pedometer.py

   This test initializes the Dongle Fox and runs a data acquisition 
   without GUI. During the initialization, the Dongle's orange LED
   and the Pedometer's green LED are on.
   Stop the thread with ctl-Z and kill %i.
   During the acquisition the Dongle's green LED and the 
   Pedometer's orange LED are blinking.


How to build the firmwares
==========================

To build the firmwares, you need the following tools installed:
* make
* cmake
* arm-none-eabi-gcc
* openocd

Make sure that arm-none-eabi-gcc and openocd are installed and reachable
via your PATH, so that make can find them.
 
To build the firmware: say ``make build-firmware``.
```
> cd ../openlab/
> mkdir build.fox/
> cd  build.fox/
> cmake .. -DPLATFORM=agile-fox
> make pedometer_send pedometer_receive
```

To flash the firmware, connect the target fox to the USB port, and say
``make flash-dongle`` or ``make flash-pedometer``.

Note: on first-time invocation, make sets up the necessary build environment
i.e. downloads and configures openlab, and then builds both the dongle and
the pedometer firmwares.  Subsequent invocations of make will just re-build
firmwares as needed e.g. if you have modified sources.


Authors
=======
Roger Pissard-Gibollet  <roger.pissard.at.inria.fr>
Olivier Fambon <olivier.fambon.at.inria.fr>

INRIA PTL Fox-Orly demo
Copyright (C) 2013-2014 INRIA


Version
========
 stable version v1.1 (2014-04-18)

