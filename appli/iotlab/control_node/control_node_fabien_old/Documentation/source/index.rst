============
Control Node
============

.. contents:: Table of Contents

Description
===========

The Control Node extends the Gateway features of controlling and monitoring the Open Node.

The Control Node is used to perform the following functions:

  * control the Open Node power supply (off/battery/DC/Battery Charge/No Battery Charge)

  * reset time reference or get the current time versus time reference

  * monitor the Open Node power consumption (periodic)

  * monitor the radio activity (periodic)

  * radio jam the Open Node

  * radio listening (sniffer)

  * emulate a sensor by its I2C link with the open node, through a loaded data table

The Control Node is interfaced from the Gateway Node, with a specific frame based protocol over a serial connection.

Hardware
========

The Control Node is a very similar hardware as the corresponding M3 Open Node.
Note that the Control Node is bound to the Gateway ARM A8 by its UARTx port on the STM32, and to the M3 of the Open Node by its I2C2.

Software
========

The Control Node software and its associated dependencies may be found in the SensTools(TODO) repository.

The following folders are necessary:

    appli/control_node: the main application source files

    drivers/: the STM32 drivers

    OS/FreeRTOS: the FreeRTOS operating system implementation

Functional Parts
----------------

The Control Node software does mainly the following:

  * **synchronous request handling**: execute the request coming from the Gateway, and sending back responses

  * **periodic measurements**: periodically measuring some physical quantities, and reporting them to the Gateway for further forwarding

  * **periodic radio jam the Open Node**: periodically create noise on the RF band uses by the open node

  * **periodic radio listening (sniffer)**: periodically send to the gateway what the control node found on 2.4GHz bandwith 802.15.4 protocol

  * **emulate a sensor**: give data from a loaded table to open node when this one read it through the I2C or interrupt to the open node

Frame Format
------------

We define here six types of frames, used by the Control Node: 

  # command frames, received by the Control Node
    Type coded with the command code specific to this command

  # response frames, sent by the Control Node after receiving a command frame
    Type coded with the same code of the command frame it answers. See the command listing.

  # acknowledge frames, sent by the Control Node following some commands marking the transition between a previous setup to a new one.
    Type coded as 0xFA

  # periodic power measurement frames, sent by the Control Node after each power periodic measurement
    Type coded as 0xFF

  # periodic radio measurement frames, sent by the Control Node after each radio periodic measurement
    Type coded as 0xFE

  # error frame, sent by the control node and signalling an issue
    Type coded as 0xEE

Command Frame Format
````````````````````

Here is the Command Frame format:

+------------------------------------+
| Command Frame                      |
+====================================+
|  sync  |  len   |  type |  payload |
+--------+--------+-------+----------+
|   1    |   1    |   1   |  0-32    |
+--------+--------+-------+----------+

The different fields are described here:

  * **sync**: this is a synchronization byte, or a start of frame, it should always be 0x80

  * **len**: this is the length of the following bytes, namely the type byte and the payload bytes

  * **type**: this is the type of command, defining what action the Control Node should do

  * **payload**: the command payload, that may be required as parameters for a command

Behavior on Command received
````````````````````````````

Whenever a command is received by the Control Node, it should check it has a valid command type, and execute it accordingly to the payload parameters.
If the execution succeeds, a response frame should be sent with the type byte matching the command type, and the ack byte should be set to ACK=0xa.
If the command is not known, or its execution fails, the response ack field should be set to NACK: 0x2, and in some case an error frame is generated.
For some command changing the setup, in addition to the response frame, an acknowledge frame is sent.


Response Frame Format
`````````````````````

Here is the Response Frame format:

+--------------------------------------------+
| Response Frame                             |
+============================================+
|  sync  |  len   |  type |  ack  |  payload |
+--------+--------+-------+-------+----------+
|   1    |   1    |   1   |   1   |   0-32   |
+--------+--------+-------+-------+----------+


The different fields are described here:

  * **sync**: this is a synchronization byte, or a start of frame, it should always be 0x80

  * **len**: this is the length of the following bytes, namely the type byte, the ack byte and the payload bytes

  * **type**: this byte equals the command type byte; it is used to identify which command this response matched

  * **ack**: the response value, indicating if the command succeeded or not

  * **payload**: additional bytes that may go with the response, the RSSI value read for example

*Response ACK Values*

In a response frame, the ACK field may take the following values:

  * ACK: 0xa, this acknowledges the command received, indicating its execution went well;

  * NACK: 0x2, this indicated the command was either badly formed, or unknown, or the execution failed;

Commands Listing
````````````````

Notice than CONFIG_RADIO_POLL and CONFIG_POWER_POLL leads to prepare a concatenated set of measurement and send it each 40 to 50ms.
In case the configurated period of sampling is above 40-50ms then there will be only 1 measure in the frame sent at each period.

Here is the list of the available commands, and their payloads, and their expected responses payloads:

+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| Command Name       | Command | Payload description               | Command description                         | Response Payload      | Acknowledge |
|                    | Value   |                                   |                                             |                       | Frame       |
+====================+=========+===================================+=============================================+=======================+=============+
| OPEN_NODE_START    | 0x70    | 1 byte indicating BATTERY=0x0     | Power the Open Node with battery or DC      | No Response Payload   | No          |
|                    |         |  or DC=0x1                        |                                             |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| OPEN_NODE_STOP     | 0x71    | 1 byte indicating CHARGE=0        | Power off the Open Node,                    | No Response Payload   | No          |
|                    |         | or NOCHARGE=1                     | CHARGE or DISCHARGE Battery                 |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| RESET_TIME         | 0x72    | No Payload                        | Set the time reference of the control node  | No Response Payload   | Yes         |
|                    |         |                                   | to 0, time is 32kHz tick                    | Time coded as an      |             |
|                    |         |                                   |                                             | unsigned 32bits int   |             |
|                    |         |                                   |                                             | inserted in periodic  |             |
|                    |         |                                   |                                             | frame                 |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| CONFIG_RADIO       | 0x74    | 1 byte for the emission Power     | Configure the Radio                         | No Response Payload   | Yes         |
|                    |         | Strength                          |                                             | But ACK frame with    |             |
|                    |         | 1 byte for the channel from 11 to |                                             | the 2 config input    |             |
|                    |         | 26                                |                                             | bytes                 |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| CONFIG_RADIO_POLL  | 0x75    | 1 byte of payload for enabling    | Start/stop recurrent radio measurement of   | No Response Payload   | No          |
|                    |         | /disabling the radio recurrent    | potential received packet RSSI & LQI. At    | But dedicated         |             |
|                    |         | measurement                       | each period a frame of measurement is sent. | recurrent frame       |             |
|                    |         | 2 bytes indicated the period in   | If no packet reiceved then LQI=0 while RSSI | containing RSSI/LQI   |             |
|                    |         | ms to sample the measures         | is the one measured. See below.             |                       |             |
|                    |         | possible to change code to 32bits |                                             |                       |             |
|                    |         | currently limited to 65535ms      |                                             |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| CONFIG_RADIO_NOISE | 0x76    | 1 byte of payload for enabling    | Configure the radio jamming. Set 1 in the   | No Response Payload   | No          |
|                    |         | /disabling the radio jamming      |  payload to enable it, 0 to disable it      |                       |             |
|                    |         |                                   | Send continuously packets containing ascii  |                       |             |
|                    |         |                                   | characters from 0x20 to 0x60                |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| CONFIG_SNIFFER     | 0x77    | 1 byte for enabling/disabling     | Start/stop continuous sniffer on the        | No Response Payload   | No          |
|                    |         | the radio sniffer                 | channel set by CONFIG_RADIO.                | but dedicated frame   |             |
|                    |         |                                   | start = 1, stop =0                          | for each packet       |             |
|                    |         |                                   |                                             | received              |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| CONFIG_SENSOR      | 0x78    | 1 byte to indicate looping or not | A sensor emulation by making available or   | No Response Payload   | No          |
|                    |         | from last data to the 1st         | sending data (16 or 32 bits) at a specific  | results seen on the   |             |
|                    |         | x bytes to indicate a period at   | period . The data comes from a table of     | open node side        |             |
|                    |         | which data is ready/to be sent    | maximum 4KB.                                |                       |             |
|                    |         | 1 to 4096 bytes as sensor data    |                                             |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| CONFIG_POWER_POLL  | 0x79    | 2 bytes of payload:               | Enable / disable periodic send of a power   | No Response Payload   | Yes         |
|                    |         | 1 byte for what to measure in PVC | frame.                                      | but recurrent power   |             |
|                    |         | order and on which power supply,  | If Enable, then configure the measurement   | measures frame        |             |
|                    |         | 1 byte for the half period        |                                             |                       |             |
|                    |         | measurement and the number of     |                                             |                       |             |
|                    |         | measure to be done to issue an    |                                             |                       |             |
|                    |         | average value                     |                                             |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| TST_I2C2           | 0xB9    | 1 byte of payload                 | START/STOP the control node I2C2 test       | No Response Payload   | No          |
|                    |         | 0x00 is to stop the rx/tx         | I2C2 is in slave mode, expecting to receive |                       |             |
|                    |         | Other values to start             | word "Transfer" and answering accordingly   |                       |             |
|                    |         |                                   | "Ok Trans" or "Bad RX  "                    |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| PINGPONG           | 0xBB    | 1 byte of payload                 | START/STOP the pingpong test                | No Response Payload   | No          |
|                    |         | 0x00 is to stop the pingpong      | This is for the autotest, listening for 1   |                       |             |
|                    |         | Other values to start             | radio packet reception. If the expecting    |                       |             |
|                    |         |                                   | one, send 1 predefined packet and go back   |                       |             |
|                    |         |                                   | to the listening step.                      |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+
| GPIO               | 0xBE    | 1 byte of payload                 | START/STOP GPIO1 GPIO2 test                 | No Response Payload   | No          |
|                    |         | 0x00 is to stop the rx/tx         | listen on GPIO1 to detect 5 edges then      |                       |             |
|                    |         | Other values to start             | toggle the GPIO2 in answer                  |                       |             |
+--------------------+---------+-----------------------------------+---------------------------------------------+-----------------------+-------------+

   **CONFIG_POWER_POLL payload**:

1st byte of payload:

+---------------------------------------------------------------------------------------------------------+
| Configuration byte of a Power Measurement Frame also 1st payload byte for a CONFIG_POWER_POLL command   |
+-----+------------------+--------------------------------------------------------------------------------+
| Bit | Bit meaning      | Bit explanation                                                                |
+=====+==================+================================================================================+
|  0  | POWER_BIT        | 1 means power supply consumption measurement inside power periodic frame       |
+-----+------------------+--------------------------------------------------------------------------------+
|  1  | VOLTAGE_BIT      | 1 means voltage supply consumption measurement inside power periodic frame     |
+-----+------------------+--------------------------------------------------------------------------------+
|  2  | CURRENT_BIT      | 1 means current supply consumption measurement inside power periodic frame     |
+-----+------------------+--------------------------------------------------------------------------------+
|  3  | NOT USED         |                                                                                |
+-----+------------------+--------------------------------------------------------------------------------+
|  4  | MODE_3V_BIT      | 1 means measurement are done on the 3.3V power supply                          |
+-----+------------------+--------------------------------------------------------------------------------+
|  5  | MODE_5V_BIT      | 1 means measurement are done on the 5V power supply                            |
+-----+------------------+--------------------------------------------------------------------------------+
|  6  | MODE_BATTERY_BIT | 1 means measurement are done on the BATTERY power supply                       |
+-----+------------------+--------------------------------------------------------------------------------+
|  7  | NOT USED         |                                                                                |
+-----+------------------+--------------------------------------------------------------------------------+

2nd byte of payload linked to ina226:

+-------------------------------------------------------+
| 2nd payload byte for a CONFIG_POWER_POLL command      |
+-------------------------------------------------------+
| bit 2 | bit 1 | bit 0 | ina226_sampling_period_t in s |
+=======+=======+=======+===============================+
|  0    |   0   |   0   |        140e-6                 |
+-------+-------+-------+-------------------------------+
|  0    |   0   |   1   |        204e-6                 |
+-------+-------+-------+-------------------------------+
|  0    |   1   |   0   |        332e-6                 |
+-------+-------+-------+-------------------------------+
|  0    |   1   |   1   |        588e-6                 |
+-------+-------+-------+-------------------------------+
|  1    |   0   |   0   |       1100e-6                 |
+-------+-------+-------+-------------------------------+
|  1    |   0   |   1   |       2116e-6                 |
+-------+-------+-------+-------------------------------+
|  1    |   1   |   0   |       4156e-6                 |
+-------+-------+-------+-------------------------------+
|  1    |   1   |   1   |       8244e-6                 |
+-------+-------+-------+-------------------------------+

+-------------------------------------------------------+
| 2nd payload byte for a CONFIG_POWER_POLL command      |
+-------------------------------------------------------+
| bit 6 | bit 5 | bit 4 | ina226_averaging_factor_t     |
+=======+=======+=======+===============================+
|  0    |   0   |   0   |            1                  |
+-------+-------+-------+-------------------------------+
|  0    |   0   |   1   |            4                  |
+-------+-------+-------+-------------------------------+
|  0    |   1   |   0   |           16                  |
+-------+-------+-------+-------------------------------+
|  0    |   1   |   1   |           64                  |
+-------+-------+-------+-------------------------------+
|  1    |   0   |   0   |          128                  |
+-------+-------+-------+-------------------------------+
|  1    |   0   |   1   |          256                  |
+-------+-------+-------+-------------------------------+
|  1    |   1   |   0   |          512                  |
+-------+-------+-------+-------------------------------+
|  1    |   1   |   1   |         1024                  |
+-------+-------+-------+-------------------------------+

+-------------------------------------------------------+
| 2nd payload byte for a CONFIG_POWER_POLL command      |
+=======+=======+=======+===============================+
| bit 3 | NOT USED                                      |
+-------+-----------------------------------------------+
| bit 7 | 1/0 ENABLE/DISABLE PERIODIC POWER FRAME       |
+-------+-------+-------+-------------------------------+

   **CONFIG_RADIO payload**:

The 1st byte of payload concatenate the emission(Tx) power strength and the emission/reception radio channel.

+----------+-------------------+
| 1st byte | Tx power strength |
| payload  |                   |
+==========+===================+
| 13       | PHY_POWER_m17dBm  |
+----------+-------------------+
| 18       | PHY_POWER_m12dBm  |
+----------+-------------------+
| 21       | PHY_POWER_m9dBm   |
+----------+-------------------+
| 23       | PHY_POWER_m7dBm   |
+----------+-------------------+
| 25       | PHY_POWER_m5dBm   |
+----------+-------------------+
| 26       | PHY_POWER_m4dBm   |
+----------+-------------------+
| 27       | PHY_POWER_m3dBm   |
+----------+-------------------+
| 28       | PHY_POWER_m2dBm   |
+----------+-------------------+
| 29       | PHY_POWER_m1dBm   |
+----------+-------------------+
| 30       | PHY_POWER_m0dBm   |
+----------+-------------------+
| 31       | PHY_POWER_0_7dBm  |
+----------+-------------------+
| 33       | PHY_POWER_1_3dBm  |
+----------+-------------------+
| 34       | PHY_POWER_1_8dBm  |
+----------+-------------------+
| 36       | PHY_POWER_2_3dBm  |
+----------+-------------------+
| 37       | PHY_POWER_2_8dBm  |
+----------+-------------------+
| 38       | PHY_POWER_3dBm    |
+----------+-------------------+

+----------+------------------------+
| 2nd byte | radio 802.15.4 channel |
| payload  |                        |
+==========+========================+
| 26       | channel 26             |
+----------+------------------------+
| 25       | channel 25             |
+----------+------------------------+
| 24       | channel 24             |
+----------+------------------------+
| 12       | channel 23             |
+----------+------------------------+
| 11       | channel 22             |
+----------+------------------------+
| 10       | channel 21             |
+----------+------------------------+
|  9       | channel 20             |
+----------+------------------------+
|  8       | channel 19             |
+----------+------------------------+
|  7       | channel 18             |
+----------+------------------------+
|  6       | channel 17             |
+----------+------------------------+
|  5       | channel 16             |
+----------+------------------------+
|  4       | channel 15             |
+----------+------------------------+
|  3       | channel 14             |
+----------+------------------------+
|  2       | channel 13             |
+----------+------------------------+
|  1       | channel 12             |
+----------+------------------------+
|  0       | channel 11             |
+----------+------------------------+


   **CONFIG_RADIO_POLL payload**:

1st payload byte
    If this byte value is 0 then this means STOP the radio measures.
    If this byte value is 1 then this means START the radio measures.
    Other values are meaningless and the response frame will be nack.

2nd and 3rd payload bytes
    They are concatened to have a 16bits value.
    The 2nd byte is the LSB byte, the 3rd byte is the MSB byte.
    The 16bits value is a time coded in ms. Its supported value are in the range of [2 to 65535ms].
    In case the value is not in this range then the response frame will be nack.


Periodic Power Measurement Frame Format
```````````````````````````````````````

Here is the Power Measurement Frame format:

+-------------------------------------------------------+
| Power Measurement Frame                               |
+=======================================================+
|  sync  |  len   |  type | count | measures            |
+--------+--------+-------+-------+---------------------+
|   1    |   1    |   1   |   1   | 2 to 4 (x time)     |
+--------+--------+-------+-------+---------------------+

The different fields are described here:

  * **sync**: this is a synchronization byte, or a start of frame, it should always be 0x80;

  * **len**: this is the length of all the following bytes;

  * **type**: this is the periodic type, and should be set to 0xFX for any periodic frame, X indicate which periodic frame is it, X=0xF for a power measurement frame

  * **count**: this is the number of bunch of measures that is contained in this frame ; currently this could be only 1.

  * **measures**: compose by count bunch of data. This bunch is a list of data from 1 to 4 32 bits size (if 1 then no measure in the frame, only the time => no more accepted). So the measures in the power measurement frame is composed from 2 to 32bits word, count times.

    +--------------------------------------------------------------------+
    | unitary measure = Bunch of data                                    |
    +====================================================================+
    | uint32_t | time of the measure in 32kHz tick since last RESET_TIME |
    +----------+---------------------------------------------------------+
    |   float  | Power measure if selected in the configuration byte     |
    +----------+---------------------------------------------------------+
    |   float  | Voltage measure if selected in the configuration byte   |
    +----------+---------------------------------------------------------+
    |   float  | Current measure if selected in the configuration byte   |
    +----------+---------------------------------------------------------+

Periodic Radio Measurement Frame Format
```````````````````````````````````````

Here is the Radio Measurement Frame format:

+-------------------------------------------------------+
| Radio Measurement Frame                               |
+=======================================================+
|  sync  |  len   |  type | count | measures            |
+--------+--------+-------+-------+---------------------+
|   1    |   1    |   1   |   1   | 3 (x times)         |
+--------+--------+-------+-------+---------------------+

The different fields are described here:

  * **sync**: this is a synchronization byte, or a start of frame, it should always be 0x80

  * **len**: this is the length of all the following bytes

  * **type**: this is the periodic type, and should be set to 0xFX for any periodic frame, X indicate which periodic frame is it, X=0xE for a radio measurement frame

  * **count**: this is the number of bunch of measures that is contained in this frame ; currently this could be only 1.

  * **measures**: compose by count bunch of data. This bunch is a list of data 1 32bits word for the time, and 1 byte for the RSSI and anoter byte for LQI measure. So the measures in the radio measurement frame is composed of 6 bytes, count times.

    +--------------------------------------------------------------------+
    | unitary measure = Bunch of data                                    |
    +====================================================================+
    | uint32_t | time of the measure in 32kHz tick since last RESET_TIME |
    +----------+---------------------------------------------------------+
    | uint8_t  | RSSI measure                                            |
    +----------+---------------------------------------------------------+
    | uint8_t  | LQI measure                                             |
    +----------+---------------------------------------------------------+

Acknowledge Config Changed Frame Format
```````````````````````````````````````

Here is the Acknowledge Config Changed Frame format:

+----------------------------------------------------------+
| Acknowledge Config Changed Frame                         |
+==========================================================+
|  sync  | len | type | config_type | (config) | (config2) |
+--------+-----+------+-------------+----------+-----------+
|   1    |  1  |  1   |   1         |   1      |   1       |
+--------+-----+------+-------------+----------+-----------+

The different fields are described here:

  * **sync**: this is a synchronization byte, or a start of frame, it should always be 0x80

  * **len**: this is the length of all the following bytes

  * **type**: this is the Acknowledge Config Changed Frame Format type = 0xFA

  * **config_type**: this is the type of the command the Acknowledge Config Changed Frame Format is about.
    By example this could be RESET_TIME, CONFIG_RADIO, CONFIG_POWER_POLL.

  * **config / config2**: Both are optional and specific to which command this acknowledge frame refers to.
    This is the config applied right now on the command the Acknowledge Config Changed Frame Format is about.
    Not send for RESET_TIME.
    Only config send for CONFIG_RADIO.
    config and config2 send for CONFIG_POWER_POLL, respectively emission power strength in config and 802.15.4 RF channel in config2

Error Frame Format
``````````````````

Here is the Error Frame format:

+-----------------------------+
| Error Frame                 |
+=============================+
|  sync  | len | type | error |
+--------+-----+------+-------+
|   1    |  1  |  1   |   1   |
+--------+-----+------+-------+

The different fields are described here:

  * **sync**: this is a synchronization byte, or a start of frame, it should always be 0x80

  * **len**: this is the length of all the following bytes

  * **type**: this is the Error Frame Format type = 0xEE

  * **error**: this is a description of the error type such as describe in the following table.
    Here are the various config_type:

    +--------------------------------------------------------------------------------------------------------------------------------+
    | Error field of an Error Frame                                                                                                  |
    +================================================================================================================================+
    +----------------------------+-------+-------------------------------------------------------------------------------------------+
    | error                      | value | description                                                                               |
    +----------------------------+-------+-------------------------------------------------------------------------------------------+
    | DEFENSIVE                  |  -3   | The execution goes in some part of the code not expected to. Ex: default in a switch case |
    +----------------------------+-------+-------------------------------------------------------------------------------------------+
    | NETWORK_QUEUE_OVERFLOW     |  -2   | Overflow of the high priority queue used to received command and to response to it        |
    +----------------------------+-------+-------------------------------------------------------------------------------------------+
    | APPLICATION_QUEUE_OVERFLOW |  -1   | Overflow of the low priority queue used to send periodic measurement...                   |
    +----------------------------+-------+-------------------------------------------------------------------------------------------+
