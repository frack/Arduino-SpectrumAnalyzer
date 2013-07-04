/*
 * ArduinoSA -- Arduino based Spectrum Analyzer based on the Cypress CYWM6935 Library
 *
 * Based on code written by Miguel A. Vallejo, Jason Hecker, and Richard Ulrich <richi@paraeasy.ch>
 * Author       :  Leigh L. Klotz, Jr. WA5ZNU
 * License      :  GPL 3.0 http://opensource.org/licenses/gpl-3.0
 * Modified by  :  Rudi Daemen <fludizz@gmail.com>
 */


/*
 * Arduino Pin usage
 * 13 SPI SCLK
 * 12 SPI MISO -> data from devices to Arduino 
 * 11 SPI MOSI -> data from Arduino to devices
 * 10 SPI SS   -> must be output
 *  9 
 *  8 
 *  7 
 *  6 RADIO SS
 *  5 RADIO RESET
 *  4 
 *  3
 *  2 
 *  1 TX
 *  0 TX
 */

/**
 * Cypress RADIO module 12-pin connector pins
 * Note that VCC=logic=3.3v
 * Name:    GND VCC  IRQ  RESET MOSI SS SCK MISO GND PD
 * Pin:     1   2    3    4     5    6  7   8    9   10
 * Arduino: GND 3.3v NC   D5    D11  D6 D13 D12  GND PULLUP
 */

#include "CYWM6935.h"
#include <SPI.h>

// Set the radio-reset to pin 5
#define RADIO_RESET 5
// Set the radio Slave-Select to pin 6
#define RADIO_SS 6
#define SPI_SS 10
#define CLOCK_RADIO SPI_CLOCK_DIV64
// Scan all 100 channels from 2400 to 2500 MHz 
#define CHANNELS 100

CYWM6935 radio(RADIO_RESET, RADIO_SS);

int ledPin = 2;

void setup()   
{
  Serial.begin(57600);
  SPI.begin();
  SPI.setClockDivider(CLOCK_RADIO);
  // Send most significant bit first when transferring a byte.
  SPI.setBitOrder(MSBFIRST);
  // Base value of clock is 0, data is captured on clock’s rising edge.
  SPI.setDataMode(SPI_MODE0);
  radio.init();
  pinMode(ledPin,OUTPUT);
}

void loop() {
  Serial.print("{\"ArduinoSA\":[");
  digitalWrite(ledPin,HIGH);
  // Loop through all 1MHz channels.
  for (byte chan=0; chan<CHANNELS; chan++) {
    byte rssi = 0;
    int freq = int(chan) + 2400;
    rssi = radio.RSSI_avg(chan, 10);
    Serial.print("{\"freq\":");
    Serial.print(freq, DEC);
    Serial.print(",\"rssi\":");
    Serial.print(rssi, DEC);
    Serial.print("}");
    if ( chan < 99 ) {
      Serial.print(",");
    }
    delay(20); // Keep write speed under Serial line speed.
    digitalWrite(ledPin,LOW);
  }
  Serial.println("]}");
}
