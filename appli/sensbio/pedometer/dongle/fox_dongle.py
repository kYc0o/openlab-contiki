"""
This file implements fox_pedometer class to collect pedometers Fox data

date: Oct 17, 2013
authors: <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>

INRIA PTL HikoB-Pedometer demo
Copyright (C) 2013 INRIA
"""
import trace
import time
import math
from threading import Lock
import fox_pedometer


class FoxDongle():
    """ Class to manage Fox dongle data pedometer reception """

    def __init__(self):
        """Class Initialization"""
        # sink node FoxPedometer class
        self.sinknode = None
        # user callback
        self.callback = None
        # PC time when the connection has been
        # established with the sink node
        self.time0 = 0.0
        # Sample time of acquisition in seconds
        self.sampletime = 1
        # Data list acquisition [time,nb_steps] (storage)
        self.steps = []
        # Mutex to handle 'steps access' critical section (FRD)
        self.steps_mutex = Lock()
        # Last Data value [time,nb_steps]
        self.laststep = [0, 0]
        # Verbosity flag
        self.verbose = False
        # Fall detection
        self.FALL_ALERT = False

    def init_dongle(self, funcname = None):
        """ Search a Fox pedometer dongle on serial link

        Parameters :
        ------------
        funcname: fonction without return and parameters
            user callback function to be called during the data reception
        """

        # Reset data
        self.steps = []
        self.laststep = [0, 0]

        # Class fox_pedometer instantiation
        self.sinknode = fox_pedometer.FoxPedometer()
        self.sinknode.verbose = self.verbose

        if self.verbose: print "Search USB serial line...";

        if self.sinknode.connect_serial() is False:
            if self.verbose: print "Line not found";
            return False # failed

        if self.verbose: print "Line found =", self.line();

        # PC time:time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())
        self.sinknode.fox_run()
        self.time0 = round(time.time())
        self.callback = funcname
        self.sinknode.set_callback(self.store)
        self.sinknode.start_read()

        return True # ok

    def close_dongle(self):
        """ Close the serial link of the Fox pedometer dongle
        """

        if not self.sinknode is None:
            self.sinknode.fox_close()
        return

    def is_running(self):
        """ Say if the Fox pedometer dongle 'read process' is running
        """
        return self.sinknode.run

    def line(self):
        """ Return the sinknode line value (string)
        """
        return str(self.sinknode.line)

    def read(self):
        """ Read one data stored

        Returns :
        ---------
        onesample : [float, int]
        read/pop the last step value [time, nb_of_steps]

        Remarks :
        ---------
        The numbers of steps are cumulative since the initial
        connection or a reset.
        """

        self.steps_mutex.acquire() # enter critical section
        if len(self.steps) != 0:
            onesample = self.steps.pop()
        else:
            onesample = None
        self.steps_mutex.release() # leave critical section

        return onesample

    def read_all(self):
        """ Read one data stored

        Returns :
        ---------
        time : list of [float, int]
        read/pop the complete list ([t0, steps0],...,[t0, steps0])
        """

        self.steps_mutex.acquire() # enter critical section
        copy_steps = self.steps
        self.steps = []
        self.steps_mutex.release() # leave critical section

        return copy_steps

    def store(self):
        """ Fox pedometer read callback to store data
        """

        self.steps_mutex.acquire() # enter critical section

        # handle podometer
        try:
            do_append = False
            nbsample = len(self.steps)

            if nbsample == 0:
                # First sample is stored
                do_append = True
            elif self.laststep[1] != self.sinknode.criteria[3]:
                # Sample is stored only if there is a change
                do_append = True

            # append last acquiered sample in steps set
            if do_append:
                onesample = [(self.sinknode.count * self.sampletime) + self.time0,
                             int(self.sinknode.criteria[3])]
                self.steps.append(onesample)
                self.laststep = onesample # saved
                if self.verbose: print "Store", nbsample, onesample;

            activity = self.sinknode.criteria[4]

            if activity & 0x2 > 0:
                self.FALL_ALERT = True

        except Exception as e:
            print trace.WHERE(), 'exception reached:', str(e)

        self.steps_mutex.release() # leave critical section

        # Launch the user callback
        try:
            if self.callback is not None:
                self.callback(self)
        except:
            print trace.WHERE(), 'exception reached:', str(e)

        return

##
## main test part
##

def test_acq_callback(obj):
    """ Fake function to test callback"""
    print "Nb of step sample", len(obj.steps)

def test():
    """ Example of use of the Fox Pedometer dongle
    """

    # instanciate dongle
    foxdongle = FoxDongle()

    print 'Enter acquisition loop (type ^C to stop).'

    init = False
    while True:
        try:
            # handle dongle initialization
            if not init and foxdongle.init_dongle(test_acq_callback):
                init = True
                print 'Device is connected to %s.' % (foxdongle.line())

            if foxdongle.is_running():
                time.sleep(5)
                val = foxdongle.read_all()
                print "READ_ALL:", val
            elif init:
                init = False
                print 'Device is disconnected. Please reconnect.'

        except KeyboardInterrupt:
            print "\nStopped."
            break
        except Exception as e:
            print trace.WHERE(), 'exception reached:', str(e)

    # must close to kill read thread (fox_pedometer)
    foxdongle.close_dongle()

    print 'Done'

if __name__ == '__main__':
    test()
