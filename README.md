Arduino-SpectrumAnalyzer
========================

A low-cost spectrum analyzer for Arduino based on a CYWM6935 Wireless USB Radio.
The Arduino source code and the CYWM6935 driver are based on sample code from https://github.com/wa5znu/CYWM6935/.

For schematics and details see http://frack.nl/wiki/Arduino_Spectrum_Analyzer (Dutch)

Usage
=====

- Upload the program in 'ArduinoSA/' to your arduino
- Run 'python arduino-sa.py' from your terminal.

Serial Output
=============

The Arduino program is configured to output JSON over Serial. The format is as follows:
{ "ArduinoSA": { "freq": frequency, "rssi": RSSI } }

