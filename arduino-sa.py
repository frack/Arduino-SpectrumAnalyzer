#!/usr/bin/python2.7
""" Client utility for the Arduino Spectrum Analyzer as found on github:
https://github.com/frack/Arduino-SpectrumAnalyzer
More information on how to build this device:
http://frack.nl/wiki/Arduino_Spectrum_Analyzer """
_author__ = 'Rudi Daemen <fludizz@gmail.com>'
__version__ = '0.2'

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
import sys
import optparse
import matplotlib.pyplot as plt

def ReadSingleSweep(device='ttyUSB0', baud=57600):
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

def ReadForever(device='ttyUSB0', baud=57600):
  """ Prints the Frequency and it's RSSI and loops forever. """
  arsa = serial.Serial(device, baud, timeout=1)
  arsa.write('l')
  while True:
    try:
      freq, avg = arsa.readline().split(' ')
      print "Frequency: %sMHz, RSSI: %s" % (int(freq) + 2400, int(avg))
    except ValueError:
      pass

def PlotSomeStuff(device='/dev/ttyUSB0', baud=57600):
  """ Uses ReadSingleSweep to build up individual graph data and plots
  it to the screen. It leaves a ghost graph for previous values. """
  plt.ion()
  x2 = y2 = None
  while True:
    data = ReadSingleSweep(device, baud)
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
  usage = "usage: %prog [options]"
  parser = optparse.OptionParser(usage=usage)
  parser.add_option("-d", "--device", metavar="DEVICE", default="/dev/ttyUSB0",
                    help="The serial device the Arduino is connected to.")
  parser.add_option("-b", "--baud", metavar="BAUD", default=57600, type="int",
                    help="Baudrate at which the Arduino is communicating.")
  (opts, args) = parser.parse_args()
  try:
    print "%s: Unrecognized argument \'%s\'" % (sys.argv[0], args[0])
    print "Try \'%s --help\' for more information." % sys.argv[0]
    sys.exit(1)
  except IndexError:
    pass
  print "Using device %s at %i baud..." % (opts.device, opts.baud)
  PlotSomeStuff(opts.device, opts.baud)
