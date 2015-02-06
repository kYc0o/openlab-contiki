"""
live_imu.py

IoT-LAB M3 Live Sensors

Display data from an IoT-LAB M3 node running the robot_listener firmware. 
The application uses input from the serial_aggregator.py. 
Please see REAME.md

Avalaible sensors are : 
- Gyroscop and Accerometer (from IMU)
- Light
- Temperature (from pression sensor)

Author: Guillaume Schreiner
"""

import argparse
from collections import deque
import socket
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from threading import Thread

class Redirector(Thread):

    def __init__(self, strPort, nodeId, maxLen):
		Thread.__init__(self)
		self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.clientsocket.connect(('localhost', int(strPort)))
		self.maxLen = maxLen
		self.nodeId = nodeId
		self.t = deque([0.0]*maxLen)
		self.rssi = deque([0.0]*maxLen)
		self.lqi = deque([0.0]*maxLen)
		self.acc_x = deque([0.0]*maxLen)
		self.acc_y = deque([0.0]*maxLen)
		self.acc_z = deque([0.0]*maxLen)
		self.gyr_x = deque([0.0]*maxLen)
		self.gyr_y = deque([0.0]*maxLen)
		self.gyr_z = deque([0.0]*maxLen)
		self.temp = deque([0.0]*maxLen)
		self.light = deque([0.0]*maxLen)
		self.terminated = False

    # add to buffer
    def addToBuf(self, buf, val):
        if len(buf) < self.maxLen:
            buf.append(val)
        else:
            buf.pop()
            buf.appendleft(val)

	# add data 
    def add_data(self, t, rssi, lqi, data_acc, data_gyr, temp, light):
		self.addToBuf(self.t,t)
		self.addToBuf(self.rssi,rssi) 
		self.addToBuf(self.lqi,lqi)
		#assert(len(data_acc) == 3)
		self.addToBuf(self.acc_x, data_acc[0])
		self.addToBuf(self.acc_x, data_acc[1])
		self.addToBuf(self.acc_z, data_acc[2])
		self.addToBuf(self.gyr_x, data_gyr[0])
		self.addToBuf(self.gyr_x, data_gyr[1])
		self.addToBuf(self.gyr_z, data_gyr[2])
		self.addToBuf(self.temp,temp)
		self.addToBuf(self.light,light)
		

    #1414746480.160646;m3-9;broadcast;32:FF:D9;RSSI;-61;LQI;255;Acc;-1.059;5.0000004E-3;6.1000004E-2;Gyr;-3.2375E-1;2.45E-1;2.1875E-1;Temp;4.25E1;Light;0
    def process_data(self, values):
		if len(values) == 20 and values[2] == "broadcast" and values[1] == self.nodeId:
			print(values)
			t = float(values[0])
			rssi = int(values[5])
			lqi = int(values[7])
			data_acc = []
			data_acc.append(float(values[9]))
			data_acc.append(float(values[10]))
			data_acc.append(float(values[11]))
			data_gyr = []
			data_gyr.append(float(values[13]))
			data_gyr.append(float(values[14]))
			data_gyr.append(float(values[15]))
			temp = float(values[17])
			light = float(values[19])
			self.add_data(t, rssi, lqi, data_acc, data_gyr, temp, light)

    def run(self):
        while not self.terminated:
            for line in self.readlines(self.clientsocket):
                values = line.split(';')
                self.process_data(values)

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
    def update(self, frameNum, sf_acc_x, sf_acc_y, sf_acc_z, sf_acc, 
			sf_gyr_x, sf_gyr_y, sf_gyr_z, sf_gyr,
			sf_rssi_x, sf_rssi,
			sf_lqi_x, sf_lqi,
			sf_light_x, sf_light,
			sf_temp_x, sf_temp):
		#print "redraw !"

		sf_acc_x.set_data(range(self.maxLen), self.redirector.acc_x)
		sf_acc_y.set_data(range(self.maxLen), self.redirector.acc_y)
		sf_acc_z.set_data(range(self.maxLen), self.redirector.acc_z)
		sf_acc.relim()
		sf_acc.autoscale_view(True, True, True)
		
		sf_gyr_x.set_data(range(self.maxLen), self.redirector.gyr_x)
		sf_gyr_y.set_data(range(self.maxLen), self.redirector.gyr_y)
		sf_gyr_z.set_data(range(self.maxLen), self.redirector.gyr_z)
		sf_gyr.relim()
		sf_gyr.autoscale_view(True, True, True)
		
		sf_rssi_x.set_data(range(self.maxLen), self.redirector.rssi)
		sf_rssi.relim()
		sf_rssi.autoscale_view(True, True, True)
		
		sf_lqi_x.set_data(range(self.maxLen), self.redirector.lqi)
		sf_lqi.relim()
		sf_lqi.autoscale_view(True, True, True)
		
		sf_light_x.set_data(range(self.maxLen), self.redirector.light)
		sf_light.relim()
		sf_light.autoscale_view(True, True, True)
		
		sf_temp_x.set_data(range(self.maxLen), self.redirector.temp)
		sf_temp.relim()
		sf_temp.autoscale_view(True, True, True)
		
		return sf_acc_x, sf_acc_y, sf_acc_z, sf_gyr_x, sf_gyr_y, sf_gyr_z, sf_rssi_x, sf_lqi_x, sf_light_x, sf_temp_x


# main() function
def main():
	# create parser
	parser = argparse.ArgumentParser(description="IoT-LAB M3 Live Sensors")
	# add expected arguments
	parser.add_argument('--port', dest='port', required=True)
	parser.add_argument('--nodeid', dest='nodeid', required=True)

	# parse args
	args = parser.parse_args()
	strPort = args.port
	nodeId = args.nodeid

	print('listening data on port %s...' % strPort)

	# plot parameters
	buf_len = 100
	redirector = Redirector(strPort, nodeId, buf_len)
	analogPlot = AnalogPlot(redirector, buf_len)
	redirector.start()

	print('plotting data...')

	# Setup figure and subplots
	f0 = plt.figure(num = 0, figsize = (12, 8))#, dpi = 100)
	f0.suptitle("IoT-LAB Robot Broadcaster", fontsize=12)
	
	sf_acc = plt.subplot2grid((3, 2), (0, 0))
	sf_gyr = plt.subplot2grid((3, 2), (1, 0))
	sf_rssi = plt.subplot2grid((3, 2), (0, 1))
	sf_lqi = plt.subplot2grid((3, 2), (1, 1))
	sf_light = plt.subplot2grid((3, 2), (2, 0))
	sf_temp = plt.subplot2grid((3, 2), (2, 1))

	# Set titles of subplots
	sf_acc.set_title('Acceleration')
	sf_gyr.set_title('Gyroscope')
	sf_rssi.set_title('RSSI')
	sf_lqi.set_title('LQI')
	sf_light.set_title('Light')
	sf_temp.set_title('Temp')

	# Turn on grids
	sf_acc.grid(True)
	sf_gyr.grid(True)
	sf_rssi.grid(True)
	sf_lqi.grid(True)
	sf_light.grid(True)
	sf_temp.grid(True)
	
	# set label names
	# sf_acc.set_ylabel("g")
	
	# set plots
	sf_acc_x, = sf_acc.plot([],[],'b-', label="x")
	sf_acc_y, = sf_acc.plot([],[],'g-', label="y")
	sf_acc_z, = sf_acc.plot([],[],'r-', label="z")
	sf_gyr_x, = sf_gyr.plot([],[],'b-', label="x")
	sf_gyr_y, = sf_gyr.plot([],[],'g-', label="y")
	sf_gyr_z, = sf_gyr.plot([],[],'r-', label="z")
	sf_rssi_x, = sf_rssi.plot([],[],'b-', label="dBm")
	sf_lqi_x, = sf_lqi.plot([],[],'b-', label="lqi")
	sf_temp_x, = sf_temp.plot([],[],'b-', label="deg")
	sf_light_x, = sf_light.plot([],[],'b-', label="lux")

	# set legends
	sf_acc.legend([sf_acc_x,sf_acc_y,sf_acc_z], [sf_acc_x.get_label(),sf_acc_y.get_label(),sf_acc_z.get_label()])
	sf_gyr.legend([sf_gyr_x,sf_gyr_y,sf_gyr_z], [sf_gyr_x.get_label(),sf_gyr_y.get_label(),sf_gyr_z.get_label()])

	# set up animation
	anim = animation.FuncAnimation(f0, analogPlot.update,
								   fargs=(sf_acc_x, sf_acc_y, sf_acc_z, sf_acc, 
									sf_gyr_x, sf_gyr_y, sf_gyr_z, sf_gyr,
									sf_rssi_x, sf_rssi,
									sf_lqi_x, sf_lqi,
									sf_light_x, sf_light,
									sf_temp_x, sf_temp),
								   interval=100)
	
	# show plot
	plt.show()

	# clean up
	redirector.stop()
	print('exiting.')


# call main
if __name__ == '__main__':
    main()
