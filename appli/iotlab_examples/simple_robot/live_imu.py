"""
live_imu.py

Display IMU data from IoT-LAB using Python (matplotlib).

Redirector should forward data on localhost with a port givent as a parameter.
The node id shoul be set as argument.
Avalaible sensors for IMU are : 
- Gyr
- Acc


Author: Guillaume Schreiner
"""

import argparse
from collections import deque
import socket
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from threading import Thread

class Redirector(Thread):

    def __init__(self, strPort, strSensor, nodeId, maxLen):
        Thread.__init__(self)
        self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clientsocket.connect(('localhost', int(strPort)))
        self.maxLen = maxLen
        self.strSensor = strSensor
        self.nodeId = nodeId
        self.ax = deque([0.0]*maxLen)
        self.ay = deque([0.0]*maxLen)
        self.az = deque([0.0]*maxLen)
	self.terminated = False

    # add to buffer
    def addToBuf(self, buf, val):
        if len(buf) < self.maxLen:
            buf.append(val)
        else:
            buf.pop()
            buf.appendleft(val)

    # add data
    def add(self, data):
        assert(len(data) == 3)
        self.addToBuf(self.ax, data[0])
        self.addToBuf(self.ay, data[1])
        self.addToBuf(self.az, data[2])

    def run(self):
        while not self.terminated:
            for line in self.readlines(self.clientsocket):
                values = line.split(';')
                if len(values) == 6 and values[2] == self.strSensor and values[1] == self.nodeId:
                    data = []
                    data.append(float(values[3]))
                    data.append(float(values[4]))
                    data.append(float(values[5]))
                    self.add(data)

    def readlines(self, socket, recv_buffer=4096, delim='\n'):
        buffer = ''
        data = True
        while data:
            data = socket.recv(recv_buffer)
            buffer += data
            while buffer.find(delim) != -1:
                line, buffer = buffer.split('\n', 1)
                yield line
        return

    # clean up
    def stop(self):
        # close serial
	self.terminated = True
        self.clientsocket.close()

# plot class
class AnalogPlot:
    # constr
    def __init__(self, redirector, maxLen):
        self.maxLen = maxLen
        self.redirector = redirector

    # update plot
    def update(self, frameNum, a0, a1, a2, axes):
        #print "redraw !"
        a0.set_data(range(self.maxLen), self.redirector.ax)
        a1.set_data(range(self.maxLen), self.redirector.ay)
        a2.set_data(range(self.maxLen), self.redirector.az)
        axes.relim()
        axes.autoscale_view(True, True, True)
        return a0, a1, a2


# main() function
def main():
    # create parser
    parser = argparse.ArgumentParser(description="Live IMU")
    # add expected arguments
    parser.add_argument('--port', dest='port', required=True)
    parser.add_argument('--sensor', dest='sensor', required=True)
    parser.add_argument('--nodeid', dest='nodeid', required=True)

    # parse args
    args = parser.parse_args()
    strPort = args.port
    strSensor = args.sensor
    nodeId = args.nodeid

    print('listening data on port %s...' % strPort)

    # plot parameters
    buf_len = 100
    redirector = Redirector(strPort, strSensor, nodeId, buf_len)
    analogPlot = AnalogPlot(redirector, buf_len)
    redirector.start()

    print('plotting data...')

    # set up animation
    fig = plt.figure()
    ax = fig.add_subplot(111)
    title = "Sensor %s on node %s" % (strSensor, nodeId)
    plt.title(title)

    a0, = ax.plot([], [])
    a1, = ax.plot([], [])
    a2, = ax.plot([], [])
    anim = animation.FuncAnimation(fig, analogPlot.update,
                                   fargs=(a0, a1, a2, ax),
                                   interval=100)

    # show plot
    plt.show()

    # clean up
    redirector.stop()
    print('exiting.')


# call main
if __name__ == '__main__':
    main()
