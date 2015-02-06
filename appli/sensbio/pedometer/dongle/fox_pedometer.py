"""
This file implements fox_pedometer class to collect pedometers Fox data

date: Oct 17, 2013
authors: <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>

INRIA PTL HikoB-Pedometer demo
Copyright (C) 2013 INRIA
"""

import trace
import serial
import re
import time
import threading
from serial.tools import list_ports

class FoxPedometer():
    """ Class to manage sink node data reading """

    def __init__(self):
        """Class Initialization"""
        # Flag/Control of data acquisition thread
        self.run = False
        # count sampletime
        self.count = 0
        # pyserial serial link
        self.ser = None
        # linux name of the serial link
        self.linename = ''
        # Criteria data : [0,0,0, nb_steps, activity, location]
        self.criteria = [0,0,0, 0, 0, ""]
        self.line = ' '
        # Thread identifier
        self.t_task = 0
        self.t_task_exception = ''
        # Verbosity flag
        self.verbose = False
        # User callback called
        self.callback = None

    def connect_serial(self):
        """Search the right USB serial line
        if line found self.ser and self.line  are set and
        the connection is established

        Returns :
        ---------
        found : boolean
                line found (True) or not (False)
        """

        timeout = 2 #sec

        for port in list_ports.comports():
            try:
                if 'USB' in port[0]:
                    baudrate = 500000
                    self.ser = serial.Serial(port[0], baudrate = baudrate, timeout = timeout)
                    # Send a init/reset cmd to found the fox on the serial line
                    if self.fox_init() is True:
                        self.ser.close()
                        self.ser = serial.Serial(port[0], baudrate = baudrate, timeout = 0)
                        self.line = port[0]
                        return True # line found
            except Exception as e:
                pass

        return False # not found

    def __fox_write_command(self, command, response):
        """Write a Fox command on serial. Check response.

        Returns :
        ---------
        cmd_ok : boolean, True if command succeed. else False.
        """

        try:
            self.ser.write(command)
            data = self.ser.read(64)
            if len(data) != 0 and response in data:
                return True # ok
        except Exception as e:
            pass
        return False

    def fox_init(self):
        """Init the Fox

        Returns :
        ---------
        cmd_ok : boolean
                 The init of the fox pedometer is launched (True)
                 or failed (False)
        """

        return self.__fox_write_command("FI\n", "FOX_USB_PEDOMETER:init_ok")

    def fox_run(self):
        """Run the data measure on Fox

        Returns :
        ---------
        cmd_ok : boolean
                 The run on fox pedometer is launched (True)
                 or failed (False)
        """

        return self.__fox_write_command("FR\n", "FOX_USB_PEDOMETER:run_ok")


    def fox_close(self):
        """Reset the Fox

        Returns :
        ---------
        cmd_ok : boolean
                 The close fox pedometer is ok (True) or failed (False)
        """

        cmd_ok = self.__fox_write_command("FI\n", "FOX_USB_PEDOMETER:init_ok")

        # **close** serial line
        try:
            self.ser.close()
        except Exception as e:
            pass

        self.run = False
        return cmd_ok

    def analyse(self):
        """Set criteria values by analysing sink node message"""

	# data read from the serial line has the following format:
        # 0x365E: 2 1 0 0 0x6DDC
	# node_id: msg_count data_count nb_steps activity neighbourhood
	ptn = re.compile('(?P<key>0x[0-9A-F]+): (?P<msg_count>[0-9]+) (?P<data_count>[0-9]+) (?P<nb_steps>[0-9]+) (?P<activity>[0-9]+) (?P<neighbourhood>0x[0-9A-F]+)')

        result = ptn.match(self.line)
        if result:
            self.count = int(result.group('msg_count'))
            self.criteria[3] = int(result.group('nb_steps'))
            self.criteria[4] = int(result.group('activity'))
            self.criteria[5] = result.group('neighbourhood')
        else:
            if self.verbose:
                print trace.WHERE(), 'ERROR: Bad format sink node message'
                print trace.WHERE(), '       line="%s"' % (self.line.replace('\r', '\\r').replace('\n', '\\n'))

    def set_callback(self, funcname = None):
        """Set function callback to be called during reading"""
        self.callback = funcname

    def read_serial(self):
        """Loop reading the rs232/usb line sink node
           put the message in self.line and update criteria
           values
        """

        data = ''
        self.t_task_exception = ''

        try:
            while self.run:
                time.sleep(0.1)
                data += self.ser.read(1024)
                if '\r\n' in data:
                    end_ix = data.index('\r\n')+2
                    self.line = data[:end_ix]
                    data = data[end_ix:]
                    self.analyse()

                    if self.verbose:
                        print "Count= ", self.count, "Criteria= ", self.criteria

                    if not self.callback is None:
                        self.callback()

        except Exception as e:
            self.t_task_exception = str(e)
            if self.verbose: print trace.WHERE(), str(e);

        self.run = False
        if self.verbose: print trace.WHERE(), '>>> thread terminated';
        return

    def start_read(self):
        """ Launch a thread for read_usb """

        if not self.run:
            self.t_task = threading.Thread(None, self.read_serial, None)
            self.run = True
            self.t_task.start()
            if self.verbose: print trace.WHERE(), 'OK: thread started';
        else:
            if self.verbose: print trace.WHERE(), "ERROR: Thread sink node starting failed";

    def stop_read(self):
        """ Stop the reading thread """

        if self.run:
            self.run = False
            self.t_task.join(5)
            if self.verbose: print trace.WHERE(), 'OK: thread stopped';
        else:
            if self.verbose: print trace.WHERE(), "ERROR: Thread sink node stopping failed";

##################
## test functions
##################

##
def test1():
    """  Test of searching USB serial line
    """
    sinknode = FoxPedometer()
    print "Search USB serial line..."
    if (sinknode.connect_serial() is True):
        print "Line found =", sinknode.line
    else:
        print "Line not found"

##
def test2():
    """ Read continously the sink node on the serial link
        without GUI stop with ctl-c
    """
    sinknode = FoxPedometer()
    sinknode.run = True
    sinknode.read_serial()

##
def test3():
    """ Read directly the sink node without GUI
        launching a thread
        stop with ctl-C
    """

    def read_callback():
        """ acq. function callback"""
        print '>', sinknode.criteria

    sinknode = FoxPedometer()

    print 'Enter acquisition loop (type ^C to stop).'

    connected = False
    while True:
        try:
            if not connected:
                if sinknode.connect_serial():
                    connected = True
                    print "Line connected at %s" % (sinknode.line)
                    sinknode.fox_run()
                    sinknode.set_callback(read_callback)
                    sinknode.start_read()
                    print 'Read started.'
                else:
                    time.sleep(1)

            if connected:
                if sinknode.run:
                    time.sleep(1)
                else:
                    sinknode.stop_read()
                    connected = False
                    print 'Line disconnected (%s).\nRead stopped.' % (sinknode.t_task_exception)

        except KeyboardInterrupt:
            print "\nStopped."
            break
        except Exception as e:
            print trace.WHERE(), 'exception reached:', str(e)

    sinknode.stop_read()

##
if __name__ == '__main__':
    test3()
