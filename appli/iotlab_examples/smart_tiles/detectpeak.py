# -*- coding: utf-8; -*-
# This file is a part of sensbiotk
# Contact : sensbio@inria.fr
# Copyright (C) 2014  INRIA (Contact: sensbiotk@inria.fr)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""
detectpeak.py <filename> <node_id> ...
detect_peak in python from <node_id> printed by smart_tiles firmware
saved in filename (by serial_aggregator)
allows to test/debug peak detection off-line

Example of use :

After firmware deployement on m3-29 to m3-32
mypc> aggr.sh 29 30 31 32 > data.txt
mypc> python detectpeack.py data.txt 29 30 31 32

"""
import sys
import os
import numpy as np
import matplotlib.pyplot as plt
from  sensbiotk.algorithms import basic as algo
import plot_imu

#TYP_SENS = 'Acc'
TYP_SENS = 'Mag'
WINDOW_SIZE = 50
PEAK_TEMPO = 0
THRESHOLD = 0.98


def test_detectpeak(filename, node):
    """ Test detectpeak implemented in Python

    Returns
    -------
    status: str
          "OK" or "ERROR"
    """
    data = plot_imu.imu_load(filename)
    datanode = plot_imu.imu_extract(data, node, sensor_type=TYP_SENS)
    timea = np.transpose([datanode['time'], datanode['time'], datanode['time']])
    sens = np.transpose([datanode['X'], datanode['Y'], datanode['Z']])
    print len(timea), len(sens), len(datanode['X'])

    sens_norm = algo.compute_norm(sens)
    sens_filt = algo.moving_average(sens_norm, WINDOW_SIZE)
    index_peak = algo.search_maxpeak(sens_filt)
    [time_peak, sens_peak] = algo.threshold_signal(timea[index_peak],
                                                   sens_filt[index_peak], THRESHOLD)
    print "Step numbers=", len(sens_peak)
        
    plt.figure()
    plt.plot(timea[:, 0], sens[:, 0:3])
    plt.title(TYP_SENS + node)
    plt.legend(('x', 'y', 'z'), bbox_to_anchor=(0, 1, 1, 0),
              ncol=2, loc=3, borderaxespad=0.)

    plt.figure()
    plt.title("Peak detection " + node)
    plt.plot(timea[:, 0], sens_norm)
    plt.plot(timea[:, 0], sens_filt)
    plt.plot(time_peak, sens_peak, "o")

     #plt.figure()
     #plt.plot(np.diff(sens_filt))
     #plt.plot(np.diff(np.sign(np.diff(sens_filt)) < 0))
     
    return "OK"


def test_detectpeak2(filename, node):
    """ Test detectpeak implemented in Python

    Returns
    -------
    status: str
          "OK" or "ERROR"
    """
    data = plot_imu.imu_load(filename)
    datanode = plot_imu.imu_extract(data, node, sensor_type=TYP_SENS)
    timea = np.transpose([datanode['time'], datanode['time'], datanode['time']])
    sens = np.transpose([datanode['X'], datanode['Y'], datanode['Z']])
    print len(timea), len(sens), len(datanode['X'])

    sens_norm = algo.compute_norm(sens)
    sens_filt = algo.moving_average(sens_norm, WINDOW_SIZE)
    #index_peak = algo.search_maxpeak(sens_filt)
    [time_peak, sens_peak] = algo.threshold_under_signal(timea,
                                                   sens_filt, THRESHOLD)
    print "Step numbers=", len(sens_peak)
        
    plt.figure()
    plt.plot(timea[:, 0], sens[:, 0:3])
    plt.title(TYP_SENS + node)
    plt.legend(('x', 'y', 'z'), bbox_to_anchor=(0, 1, 1, 0),
              ncol=2, loc=3, borderaxespad=0.)

    plt.figure()
    plt.title("Peak detection " + node)
    plt.plot(timea[:, 0], sens_norm)
    plt.plot(timea[:, 0], sens_filt)
    plt.plot(time_peak, sens_peak, "o")

   
    return "OK"


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
        # Plot all sensors 
        for node in nodes:
            if TYP_SENS == 'Acc':
                test_detectpeak(filename, node)
            else:
                test_detectpeak2(filename, node)

        plt.show()
