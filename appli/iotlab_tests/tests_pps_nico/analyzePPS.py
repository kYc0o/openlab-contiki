#!/usr/bin/python
import sys
import pylab as pl
import numpy as np

InputFileName= sys.argv[1]

data=np.genfromtxt(InputFileName)

print "Got " , data.size , " measures, avg= " , data.mean() , " std dev=", data.std()

fig = pl.figure()
ax = fig.add_subplot(111)

ax.hist(data,10,color='green')
pl.show()



