#!/usr/bin/python2.7


# Wifi|    Frequency (MHz)
# Chn | Center | Start | End      
#  1  | 2412   | 2402  | 2422
#  2  | 2417   | 2407  | 2427
#  3  | 2422   | 2412  | 2432
#  4  | 2427   | 2417  | 2437
#  5  | 2432   | 2422  | 2442
#  6  | 2437   | 2427  | 2447
#  7  | 2442   | 2432  | 2452
#  8  | 2447   | 2437  | 2457
#  9  | 2452   | 2442  | 2462
#  10 | 2457   | 2447  | 2467
#  11 | 2462   | 2452  | 2472
#  12 | 2467   | 2457  | 2477
#  13 | 2472   | 2462  | 2482
#  14 | 2484   | 2474  | 2494

import serial
import time
import matplotlib.pyplot as plt

def ReadSingleSweep(device, baud=57600):
  """ Returns a list of 100 tuples. Each tuple is a 1MHz channel paired with
  it's RSSI. """
  arsa = serial.Serial(device, baud, timeout=1)
  arsa.write('l')
  sweep = 0
  data = {} 
  while sweep < 100:
    try:
      rawdata = arsa.readline()
      freq, avg = rawdata.split(' ')
      data[int(freq) + 2400] = int(avg)
      sweep += 1
    except ValueError:
      pass
  arsa.write('s')
  arsa.close()
  return data

def ReadForever(device, baud=57600):
  """ Returns a list of 100 tuples. Each tuple is a 1MHz channel paired with
  it's RSSI. """
  arsa = serial.Serial(device, baud, timeout=1)
  arsa.write('l')
  while True:
    try:
      freq, avg = arsa.readline().split(' ')
      print "Frequency: %sMHz, RSSI: %s" % (int(freq) + 2400, int(avg))
    except ValueError:
      pass

def PlotSomeStuff():
  plt.ion()
  x2 = y2 = None
#  plt.show(block=False)
  while True:
    data = ReadSingleSweep('/dev/ttyUSB3')
    x = []
    y = []
    for freq, lvl in data.iteritems():
      x.append(freq)
      y.append(lvl)
    plt.clf()
    if x2:
      plt.plot(x,y,'r-', x2,y2,'r:')
    else:
      plt.plot(x,y, 'r-')
    plt.ylim((0, 35))
    plt.grid(True, which='major', axis='both')
    plt.ylabel('RSSI')
    plt.xlabel('Frequency')
    plt.draw()
    x2 = x
    y2 = y
    time.sleep(.5)
  

if __name__ == '__main__':
#  print ReadSingleSweep('/dev/ttyUSB3')
  PlotSomeStuff()
#  ReadForever('/dev/ttyUSB3')
