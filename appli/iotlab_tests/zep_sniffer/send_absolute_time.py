#! /usr/bin/env python
from datetime import datetime
import time, struct, serial

ser = serial.Serial("/dev/ttyUSB1", 500000, timeout=1)
ser.close()
ser.open()
# Wait for the next second's half.
t = datetime.utcnow()
sleeptime = 1.5 - t.microsecond/1000000.0
print  "Will sleep for %s seconds to intercept next half second." % sleeptime
time.sleep(sleeptime)
# Send epoch through serial port.
epoch = int(time.time())
print "Sending current epoch : %s" % epoch
ser.write("utc" + struct.pack("!I", epoch) + "\n")
time.sleep(2)
read_val = ser.read(size=64)          # Read ack if any.
print "Received reply : %s" % read_val
