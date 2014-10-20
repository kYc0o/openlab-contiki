#!/bin/sh
# user@devgrenoble:~$ nc m3-381 20000 >robot.txt

scp dev-gre:robot.txt .
grep Acc robot.txt | sed s/Acc\;// > acc.txt
grep Mag robot.txt | sed s/Mag\;// > mag.txt
grep Gyr robot.txt | sed s/Gyr\;// > gyr.txt
grep Ang robot.txt | sed s/Ang\;// > ang.txt
python plot_imu.py

