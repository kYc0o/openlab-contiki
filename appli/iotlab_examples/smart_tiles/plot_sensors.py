#!/usr/bin/python
# -*- coding: utf-8 -*-

""" plot_sensors.py <filename> <node_id> ...
plot sensors  values from <node_id> printed by smart_tiles firmware 
saved in filename (by serial_aggregator)

Example of use :

After firmware deployement on m3-29 to m3-32
mypc> aggr.sh 29 30 31 32 > data.txt
mypc> python myplot.py data.txt 29 30 31 32

"""
import sys
import os
import numpy as np
import matplotlib.pyplot as plt

FIELDS = {'time': 0, 'name': 1, 'type': 2, 'X': 3, 'Y': 4, 'Z': 5}

def imu_load(filename):
    """ Load iot-lab imu file

    Parameters:
    ------------
    filename: string
              imu filename saved from smart_tiles firmware

    Returns:
    -------
    data : numpy array
    [timestamp node_name sensor_type X Y Z]
    """

    try:
        mytype = [('time', '<f8'), ('name', '|S11'), ('type', '|S11'),
                  ('X', '<f8'), ('Y', '<f8'), ('Z', '<f8')]
        # pylint:disable=I0011,E1101
        data = np.genfromtxt(filename, skip_header=1, invalid_raise=False,
                             delimiter=";", dtype=mytype)
    except IOError as err:
        sys.stderr.write("Error opening oml file:\n{0}\n".format(err))
        sys.exit(2)
    except (ValueError, StopIteration) as err:
        sys.stderr.write("Error reading oml file:\n{0}\n".format(err))
        sys.exit(3)

    # Start time to 0 sec
    data['time'] = data['time'] - data['time'][0]

    return data


def imu_extract(data, node_name='', sensor_type='Acc'):
    """ Extract iot-lab imu data for node_name, sensor_type

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    node_name: string
       name of the iot-lab name to be extracted
    sensor_type: string
       type of the sensor to be extracted 'Acc' or 'Mag'

    """
    if node_name != '':
        condition = data['name'] == node_name
        # pylint:disable=I0011,E1101
        filtname_data = np.extract(condition, data)
    else:
        filtname_data = data
    condition = filtname_data['type'] == sensor_type
    # pylint:disable=I0011,E1101
    filt_data = np.extract(condition, filtname_data)
    return filt_data


def imu_plot(data, title):
    """ Plot iot-lab imu data

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    title: string
       title of the plot
    """
    plt.figure()
    plt.grid()
    plt.title(title)
    plt.plot(data['time'], data['X'])
    plt.plot(data['time'], data['Y'])
    plt.plot(data['time'], data['Z'])
    plt.xlabel('Sample Time (sec)')

    return


def imu_all_plot(data, title, ylabel, nodes, sensor_type):
    """ Plot iot-lab imu data

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    title: string
       title of the plot
    ylabel: stringx
       ylabel of the plot
    nodes: tab of string
       list of nodes_names
    """
    nbplots = len(nodes)

    if nbplots > 0:
        plt.figure()
        i = 0
        for node in nodes:
            i = i + 1
            node_plot = plt.subplot(nbplots, 1, i)
            node_plot.grid()
            plt.title(title + nodes[i-1])
            datanode = imu_extract(data, nodes[i-1], sensor_type)
            peaknode = imu_extract(data, nodes[i-1], sensor_type+'Peak')
            print nodes[i-1], len(datanode)
            norm = np.sqrt(datanode['X']**2 + datanode['Y']**2
                           + datanode['Z']**2)
            node_plot.plot(datanode['time'], norm)
            node_plot.plot(peaknode['time'], peaknode['X'], 'ro')
            plt.ylabel(ylabel)
        plt.xlabel('Sample Time (sec)')

    return


def usage():
    """Usage command print
    """
    print "Usage"
    print __doc__


if __name__ == "__main__":

    if len(sys.argv) <= 2:
        usage()
    else:
        filename = sys.argv[1]
        # Verif the file existence
        if not os.path.isfile(filename):
            usage()
            sys.exit(1)
        # Nodes list
        nodes =[]
        for arg in sys.argv[2:]: 
            nodes.append('m3-'+arg)
    
        # extract data from file
        data = imu_load(filename)
        # Plot all sensors acc sensors
        for node in nodes:
            #datanode = imu_extract(data, node, sensor_type='Acc')
            #imu_plot(datanode, "Accelerometers " + node)
            datanode = imu_extract(data, node, sensor_type='Mag')
            imu_plot(datanode, "Magnetometers " + node)
        # Plot all norm accelerometers on a same windows
        #imu_all_plot(data, "Accelerometers ", "Norm Acceleration (G)", nodes, 'Acc')  
        imu_all_plot(data, "Magnetometers ", "Norm ", nodes, 'Mag') 
        plt.show()

