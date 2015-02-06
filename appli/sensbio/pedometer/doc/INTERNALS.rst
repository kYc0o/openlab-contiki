=========================
HikoB-Pedometer Internals
=========================

The Fox Pedometer system contains 3 main pieces of software:

* python driver
* dongle firmware
* pedometer firmware

Source files are located in directories:

* hikob_pedometer/ (python driver)
* firmware/ (dongle and pedometer C code)
 
The following sections provide an overview of the software
components and functions.  Please refer to the code for full details.
 


Driver
======

Directory hikob_pedometer/ contains the python driver source files:

* fox_pedometer.py : class to collect Fox pedometer data through USB
* fox_dongle.py    : API class for collecting Fox data 

Ths role of the python driver is to hide the low-level communication
that happens between the dongle and the host via usb.

The system as a whole is seen and operated through the driver, which
implements the client side of the usb protocol.  The usb protocol is
split in two categories: command-response and pedometer data transfer.

Command-Response
----------------

Command-response is a set of simple text-based commands, used for configuring
and starting the system.  Each command provides success or error responses as 
plain text.  The following commands are available to control the system:

* FI (fox init)
* FR (fox run)
* FB (beamer mode)

Command FI must be called prior to issuing command FR
Command FR must be used to activate the pedometer node, and thus produce data
Command FI can be used at any time, and causes a reset of the pedometer node
Command FB can be used at any time, and turns the dongle into a "beamer"

The driver code issues commands FI and FR as part of the init_dongle() 
primitive of the API, via underlying connect_serial() and fox_run() internals.
Command FB is currently not exposed by the driver.

Command FI accepts configuration options, currently not exposed by the driver.
The following options can be specified:

* period (1 integer, in seconds)
* pedometer count-steps parameters (2 integers, 1 float)
* pedometer free-fall parameters (1 float, 1 integers, 2 floats, 1 integer)

Command FI may be used in one of 3 ways:

* with no parameters (e.g. in the driver),
* with a single parameter (the <period> parameter alone),
* with all parameters at once.

When used with no parameters, <period> is set to a default of 1 second;
issuing commands "FI" or "FI 1" is thus equivalent.

When used with the <period> parameter alone, command FI sets only the period
and other parameters remain unchanged.

When used with all parameters, command FI sets all parameters. No sanity checks
are performed on parameters, so use with care or expect a pedometer crash.

Please check function parse_parameters() in file pedometer_receive.c
for details on pedometer count-steps and free-fall parameters.

Default values for all parameters may be restored using command:
"FI 1 25 50 11 1.5 15 10 5 15"

Data transfer  
-------------

Pedometer data samples are sent by the pedometer node to the dongle node
when radio connectivity is available.  Upon reception, the dongle node
re-transmits data as plain text over the usb link to the driver, which
in turn exposes them via the read_all() primitive.

The format of the data available as text over the usb link is as follows:

  <pedometer id>: <msg count> <data count> <nb steps> <activity> <location id>

An example of a data sample is shown below:

  0x365E: 230 197 42 1 0x6DDC

Values for <msg count> and <data count> are internal and currently not used.
Values for <pedometer id> and <location id> are both based on HiKoB node uids,
and are currently not exposed and not used.
Values for <activity> are currently in the range 0-5.  Activity is a 3-bits
bit-field providing activity hint (bit 1), free-fall detection (bit 2), and
rotation detection (bit 3).

The frequency at which data samples are delivered by the dongle node over
the serial link is grosso modo set by the <period> parameter, provided radio
connectivity is fine.  When radio connectivity is not available, the pedometer
node stores data samples, and bursts them all out when connectivity to the
dongle is restored, causing the dongle to burst all data in turn to the driver.

Beamer mode
-----------

Any dongle node can be turned into a "beamer-only" node by using command FB.
When a dongle functions in beamer mode, no data is collected by the dongle,
rather "location" information is beamed to the pedometer node, allowing to
associate neightbourhood to data samples as they are generated.

Please refer to section "Beamer mode" under section "Dongle" below.



Dongle
======

Directory firmware/ contains the dongle source files:
* pedometer_receive.c (main entry point, USB and radio communication)
* msg.h (radio messages protocol)

The dongle firmware implements the server side of the usb protocol, and
handles all radio communication with the pedometer node.  The usb protocol
is text-based, as described above in section "Driver".  Radio communication
between HiKoB Fox nodes is implemented using binary-format messages sent over
csma over 802.15.4 phy.

Serial link communication is not crc-ed or ack-ed in any way in the software.

Radio communication messages are ack-ed in the firmware, and crc-ed by PHY.


The server-side usb protocol is two fold:
* commands handling
* pedometer data forwarding

Commands handling
-----------------

Commands are read from the serial line and executed immediately.  Commands FI
and FR cause "init" and "run" messages to be sent to the pedometer node.  These 
messages are acknowledged by the pedometer node.  The ACK is expected within 1s
and causes an ok status to be returned over the serial link to the driver.  If
no ACK is received in time, an error status is returned.

Pedometer data forwarding
-------------------------

Pedometer data is converted to text and forwarded to the driver over the serial
link immediately upon reception.  The dongle sends an ACK message to the
pedometer to indicate that the last sent data message was received.

Beamer mode
-----------

When a dongle functions in beamer mode, no data is collected i.e. acknowledged
when the pedometer sends samples; rather than sending an ACK response message
to the pedometer, the dongle replies with a beamer message.  When received by
the pedometer, the beamer message origin identifies the dongle by id.
The pedometer keeps the sample in cache for resend, as no ack was received,
and stores the "location" information, i.e. the id of the beamer;  new data
samples generated by the pedometer are then stamped with the updated location.



Pedometer
=========

Directory firmware/ contains the pedometer source files:

* pedometer_send.c (main entry point, radio communication, cache)
* countstep.c (pedometer, number of steps counter)
* freefall.c  (free-fall detector)
* culbuto.c   (rotation  detector)


Radio communication
-------------------

Init and Run command messages are acknowledged to the dongle node.  There is no
ack acknowledge build into the system.

Data messages are expected to be ack-ed by the dongle node.  The ack system
works in conjuction with the cache system, described below.  Data messages are
sent one at a time.  The currently "sending" message is resent at each loop, as set by parameter <period>, until an ack from the dongle acknowledges reception.

When an ack is received from the dongle, the next message stored in the cache
becomes current, and is sent immediately, if any.

Beamer messages sent by the dongle in response to (ignored) data messages cause
internal variable "location" to be updated with the beamer id.  Newly generated
pedometer data samples are stamped with the updated location, and stored in the
cache.

Cache
-----

The cache system is a circular buffer which stores pedometer data samples
as they are produced by the periodic pedometer+fall detection system.  Data
samples are consumed one by one as a data packet ack is received from the
dongle, as described above.

The cache size is set to 100, which amounts to approx. 2 minutes of offline
storage with period set to 1 minute (default).  Older samples get overwritten
if the cache is not consumed in time (i.e. messages acked by the dongle).

Counting steps
--------------
The fonction of counting human steps  (pedometer) is usefull to compute
an indicator of its activity. More details can  be read here:

http://titan.medhyg.ch/mh/formation/print.php3?sid=23948

The algorithm implemented is inspired with simplification from :

http://ubicomp.cs.washington.edu/uwar/libby_peak_detection.pdf

It consists in searching peaks :

* Signal norm computation of 3 accelerometers axis
* lower-pass filter by moving averarage (size of *window_size* samples)
* jerk computation
* detection of jerk zero threshold indicating peak
* validate the peak if its norm is >  *threshold* g
* tempo of *peak_tempo* samples before to perform an other detection

By default the parameters are :

* *window_size* = 25 
* *peak_tempo* = 50
* *threshold* = 11.0

The algorithm basic sample time is 5 ms.

Free-fall detection
-------------------

The fall detector is inspired with simplification from :

http://www.analog.com/library/analogdialogue/archives/43-07/fall_detector.html

It consists in :

* normAcc = Signal norm computation of 3 accelerometers axis
* if  (normAcc < *level_fall* g) during *min_dur_fall* ms
  Then Free-fall detection 
* After Free-fall detection if normAcc > (*level_inactive* g +/-*epsilon_inactiveduring*) during *min_dur_inactive*
  Then Fall detection

By default the parameters are :

* *level_fall* = 1.5
* *min_dur_fall* = 15
* *level_inactive* = 10.0
* *epsilon_inactive* = 5.0
* *min_dur_inactive* = 15

The algorithm basic sample time is 5 ms.
