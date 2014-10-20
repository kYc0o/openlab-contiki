# pissardg@devgrenoble:~$ nc m3-381 20000 >robot.txt
# mypc$ offline.sh

import numpy as np
import matplotlib.pyplot as plt

acc = np.loadtxt("acc.txt",delimiter=';')
mag = np.loadtxt("mag.txt",delimiter=';')
gyr = np.loadtxt("gyr.txt",delimiter=';')
ang = np.loadtxt("ang.txt",delimiter=';')

plt.figure()
plt.title("Accelerometers")
plt.plot(acc)

plt.figure()
plt.title("Magnetometers")
plt.plot(mag)

plt.figure()
plt.title("Gyrometers")
plt.plot(gyr)

plt.figure()
plt.title("Robot orientation")
plt.plot(ang)
plt.show()


