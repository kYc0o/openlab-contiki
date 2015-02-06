============
TDMA example
=============

Overview
--------

This code is an example of use of the TDMA MAC protocol
[library](https://github.com/iot-lab/openlab/tree/master/net/mac_tdma)

TDMA for Time Division Multiple Access allows several nodes to share
the same frequency channel by dividing the signal into different time
slots. A node coordinator generates network beacons that provide a
timing indication for client nodes to access the channel.


Prerequisites
-------------

- Tutorial : [Submit an experiment with M3 nodes using the web portal](https://www.iot-lab.info/tutorials/submit-an-experiment-with-web-portal-and-m3-nodes)
- Tutorial : [Get and compile a M3 Firmware code](https://www.iot-lab.info/tutorials/get-compile-a-m3-firmware-code/)
- Tutorial : [Experiment CLI client](https://www.iot-lab.info/tutorials/experiment-cli-client/)
- Tutorial : [Nodes Serial Link Aggregation](https://www.iot-lab.info/tutorials/nodes-serial-link-aggregation/)


Implementation 
---- 

In this implementation, a set of N client nodes share a network
(panid) on the same radio channel, but each client only uses the
channel during predetermined slots. A frame consists of M slots one
for each client nodes. A slot is reserved to the node coordinator and
one for the broadcast. Frames are repeated continuously.

maximium number of frame    = MAC_TDMA_MAX_FRAMES 10
maximum size of slots-frame = TDMA_MAX_SLOTS 50
slot time unit = TDMA_SLOT_DURATION_FACTOR_US 100u 

Code
---- 

In the code, you can find two types of coordinator node :
  * ``example_tdma_coord.c`` : a dynamic affectation of the slotframe for the client nodes
  * ``example_tdma_coord_static.c`` : a static affectation node of the slotframe specifying the node client adress. 

For the client, there is :
  * a TDMA node example ``example_tdma_node.c`` 
  * the same example adding a user-callback on slot ``nodexample_tdma_node_cb.c``

#### The scenario example consists of :

  * The coordinator node sends periodically SOFT_TIMER_FREQUENCY a
    counter index and prints on serial line the begin of sending and
    the status of sending : ``Send packet..?/Packet sending result...``

  * Each client node does the same at a lower period (5 * SOFT_TIMER_FREQUENCY)

  * Nodes print when they receipt a radio message. For the example
    with the user-callback on slot, client nodes print a message when
    their slot is active.

Example of Use with IoT-LAB
----------------------------

This is a trace for a scenario, using dynamic coordinator
``example_tdma_coord.elf`` and the node client without callback
``example_tdma_node.elf``.

In this example, we book nodes of grenoble : *m3-33*, *m3-34* and *m3-36* without
assigning firmware.

Then, we assign :

  * to *m3-35* the ``example_tdma_coord.elf`` firmware which becomes the
   TDMA coordinator node
 
  * to *m3-34* the ``example_tdma_node.elf`` firmware which becomes a
    TDMA client node

You can observe nodes (*m3-35 address: 0xA581 and m3-35 address:
0x3968*) printing from your PC by :

```
my_computer$ ssh <login>@grenoble.iot-lab.info "serial_aggregator"
```

After the initialisation period, you can see the two nodes exchanging
messages as expected :

```
...
418749540.351909;m3-35;Send packet 66
1418749540.357089;m3-35;Packet sending result 0
1418749540.359868;m3-34;Packet received from 0xA581 : 66
1418749541.350265;m3-35;Send packet 67
1418749541.458564;m3-35;Packet sending result 0
1418749541.458769;m3-34;Packet received from 0xA581 : 67
1418749541.990594;m3-34;Send packet 64
1418749542.016626;m3-34;Packet sending result 0
1418749542.022590;m3-35;Packet received from 0xB968 : 64
1418749542.350077;m3-35;Send packet 68
1418749542.358595;m3-35;Packet sending result 0
1418749542.358796;m3-34;Packet received from 0xA581 : 68
...
```

You can add dynamically the *m3-33* (*address: 0xB482*) node by flashing
it with ``example_tdma_node.elf``. Then you can see the node matching
and the slotframe affectation

```
...
418749747.914711;m3-33;[in event_init() INFO] Priority of event task #1: 7/7
1418749747.916756;m3-33;
1418749747.916874;m3-33;
1418749748.094922;m3-33;Platform starting in 1... [in event_init() INFO] Priority of event task #0: 6/7
1418749748.095809;m3-33;[in event_init() INFO] Priority of event task #1: 7/7
1418749748.104065;m3-33;
1418749748.104189;m3-33;
1418749748.325093;m3-35;Send packet 18
1418749748.387863;m3-35;Packet sending result 0
1418749748.388070;m3-34;Packet received from 0xA581 : 18
1418749749.099876;m3-33;Platform starting in 1... 
1418749749.099997;m3-33;GO!
1418749749.100483;m3-33;FreeRTOS Heap Free: 19744
1418749749.101622;m3-33;[in scan_start() INFO] Start to scan on channel 21
1418749749.325084;m3-35;Send packet 19
1418749749.384710;m3-33;[in scan_handler() INFO] Scanned beacon from 0xA581 : 10*10000us
1418749749.385645;m3-33;[in tdma_slot_start() INFO] pan id: 6666
1418749749.386282;m3-33;[in tdma_slot_start() INFO] pan slots: 10*10000 us
1418749749.395135;m3-33;[in tdma_slot_configure() INFO] Set slot[0] to A581 (TX:0)
1418749749.395263;m3-33;[in tdma_slot_configure() INFO] Set slot[5] to B482 (TX:1)
1418749749.395374;m3-33;[in scan_handler() INFO] Wait 10 slots-frame before associating
1418749749.481619;m3-34;Packet received from 0xA581 : 19
1418749749.490523;m3-35;Packet sending result 0
1418749750.326570;m3-35;Send packet 20
1418749750.338539;m3-35;[in tdma_slot_configure() INFO] Set slot[7] to B482 (TX:1)
1418749750.338668;m3-35;[in tdma_slot_configure() INFO] Set slot[2] to B482 (TX:1)
1418749750.386533;m3-34;[in tdma_slot_configure() INFO] Set slot[2] to B482 (TX:2)
1418749750.386664;m3-34;[in tdma_slot_configure() INFO] Set slot[7] to B482 (TX:2)
1418749750.386810;m3-33;[in tdma_slot_configure() INFO] Set slot[1] to B968 (TX:1)
1418749750.386921;m3-33;[in tdma_slot_configure() INFO] Set slot[2] to B482 (TX:2)
1418749750.387082;m3-33;[in tdma_slot_configure() INFO] Set slot[5] to 0000 (TX:1)
1418749750.388056;m3-33;[in tdma_slot_configure() INFO] Set slot[6] to B968 (TX:1)
1418749750.389971;m3-33;[in tdma_slot_configure() INFO] Set slot[7] to B482 (TX:2)
1418749750.398639;m3-33;[in phy_rx() WARNING] RX too late or timeout too late
1418749750.482629;m3-35;Packet sending result 0
1418749750.482833;m3-34;Packet received from 0xA581 : 20
1418749750.483688;m3-33;Packet received from 0xA581 : 20
1418749751.331909;m3-35;Send packet 21
1418749751.383904;m3-34;[in tdma_slot_update_frame_start() INFO] Shifted frame of -1 ticks
1418749751.480750;m3-35;Packet sending result 0
...
```

Then the communication and the *m3-33 node* is added in the network:

```
...
1418749759.159805;m3-35;Packet received from 0xB482 : 1
1418749759.160049;m3-33;Packet sending result 0
1418749759.323784;m3-35;Send packet 29
1418749759.479192;m3-35;Packet sending result 0
1418749759.479437;m3-34;Packet received from 0xA581 : 29
1418749759.479620;m3-33;Packet received from 0xA581 : 29
1418749760.323794;m3-35;Send packet 30
1418749760.377354;m3-34;Packet received from 0xA581 : 30
1418749760.383791;m3-35;Packet sending result 0
1418749760.383791;m3-33;Packet received from 0xA581 : 30
1418749761.326551;m3-35;Send packet 31
...
```

