#! /usr/bin/env python
#---------------------------------------------------------------------------
# Read information from serial port and output it to wireshark on loopback
# as UDP packets
#
# Format of data from serial port is: (<length of ZEP info><ZEP info>)*
# <ZEP info> is sent inside an UDP packet.
#
# Cedric Adjih, Inria, 2014
#---------------------------------------------------------------------------

import serial, socket, sys

NODE_PORT = '/dev/ttyA8_M3'

ZepPort = 17754

sd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
mote = serial.Serial(port=NODE_PORT, baudrate=500000)  #XXX: port

# provide a destination for UDP packets to avoid IP stack ICMP error messages
udp_srv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_srv.bind(("", ZepPort))

Magic = "EX\x02"

data = ""
while True:

    while True:
        pos = data.find(Magic)
        if pos > 0:
            break
        elif pos == 0:
            data = data[len(Magic):]
        data += mote.read(1)

    assert pos > 0
    if pos > 1:
        sys.stderr.write(".")
        sys.stderr.flush()
    zepSize = ord(data[pos-1])

    data = data[pos:]
    while len(data) < zepSize:
        newData = mote.read(zepSize - len(data))
        assert len(newData) > 0
        data += newData
    sys.stderr.write("+")
    sys.stderr.flush()

    sd.sendto(data[:zepSize], ("", ZepPort))
    data = data[zepSize:]

#---------------------------------------------------------------------------
