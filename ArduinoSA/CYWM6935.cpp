/*
 * Name         :  CYWM6935.cpp
 * Description  :  This is a partial driver for the CYWM6935 wireless usb chip.
 *                 It is only concerned with reading the RSSI value for different frequencies.
 *                 Based on the code written by Miguel A. Vallejo and Jason Hecker
 * Author       :  Richard Ulrich <richi@paraeasy.ch>
 * License      :  GPL v. 3
 * Updates      :  Ported to Arduino 1.0.1
 *              :  Changed init to follow Cypress application note.
 *              :  Changed RSSI to follow Cypress application note.
 *              :  Added RSSI_peak and RSSI_avg.
 *		   Leigh L. Klotz, Jr. WA5ZNU
*/

#include "CYWM6935.h"
#include <Arduino.h>

#define RSSI_MASK 0x1F

CYWM6935::CYWM6935(const uint8_t pinReset, const uint8_t pinChipSel)
  : pinReset_(pinReset), pinChipSel_(pinChipSel)
{

}

CYWM6935::~CYWM6935()
{
  
}

void CYWM6935::init()
{
    pinMode(pinReset_,    OUTPUT); // active low reset   
    pinMode(pinChipSel_,  OUTPUT); // chip select  
    
    // reset the radio
    digitalWrite(pinReset_,   HIGH);
    digitalWrite(pinChipSel_, HIGH);
    delay(1);
    
    // initialize the radio module
    Write(REG_ANALOG_CTL,   0x01);	
    delayMicroseconds(50);
    Write(REG_CLOCK_MANUAL, 0x41);
    Write(REG_CLOCK_ENABLE, 0x41);
    Write(REG_ANALOG_CTL,   0x44);	
    Write(REG_CRYSTAL_ADJ,  0x40);
    Write(REG_VCO_CAL,      0xC0);
    Write(REG_SYN_LOCK_CNT, 0xFF);
}

const uint8_t CYWM6935::Read(const RADIO_REGISTERS address) const
{    
    digitalWrite(pinChipSel_, LOW);         // Enable module    
    SPI.transfer(address);                  // Send address    
    const uint8_t val = SPI.transfer(0x00); // Receive data    
    digitalWrite(pinChipSel_, HIGH);        // Disable module
    
    return val;
}

void CYWM6935::Write(const RADIO_REGISTERS address, const uint8_t value) const
{    
    digitalWrite(pinChipSel_, LOW);  // Enable module    
    SPI.transfer(0x80 | address);    // Send data
    SPI.transfer(value);             // Send data
    digitalWrite(pinChipSel_, HIGH); // Disable module
}

// From http://www.cypress.com/?docID=24401
//
// To check for a quiet channel before transmitting:
// 1. First set up receive mode properly and read the RSSI register (Reg 0x22). 
// 2. If the valid bit is zero, then force the Carrier Detect register
//    (Reg // 0x2F, bit 7=1) to initiate an ADC conversion.
// 3. Then, wait greater than 50 μs and read the RSSI register again. 
// 4. Next, clear the Carrier Detect Register (Reg 0x2F, bit 7=0) and turn the receiver OFF. 
//
// Measuring the noise floor of a quiet channel is inherently a
// 'noisy' process so, for best results, this procedure should be
// repeated several times (~20) to compute an average noise floor
// level.
//
// A RSSI register value of 0-10 indicates a channel that is
// relatively quiet. A RSSI register value greater than 10 indicates
// the channel is probably being used. A RSSI register value greater
// than 28 indicates the presence of a strong signal.  
//

// This function returns an instantaneous reading. 
const uint8_t CYWM6935::RSSI(const uint8_t channel) const
{    
    Write(REG_CHANNEL, channel);       // Set channel    
    Write(REG_CONTROL, 0x80);          // Turn receiver on
    // Wait for receiver to start-up
    // SYNTH_SETTLE (200) + RECEIVER_READY (35) + RSSI_ADC_CONVERSION (50)
    delayMicroseconds(285);
    Write(REG_CARRIER_DETECT, 0x00);  // clear override
    
    uint8_t value = 0;
    while(1)
    {
      uint8_t v = Read(REG_RSSI);  // Read RSSI
      if ((v & 0x20) == 0) {
	Write(REG_CARRIER_DETECT, 0x80);  // set override
	delayMicroseconds(50);   // RSSI_ADC_CONVERSION (50)
	v = Read(REG_RSSI);  // Read RSSI
	Write(REG_CARRIER_DETECT, 0x00);  // clear override 
      } else {
	value = v & RSSI_MASK;
	break;
      }
    }

    Write(REG_CONTROL,0x00);     // Turn receiver off    
    
    return value; // Return lower n bits
}

// This function returns the peak of n readings.
// Returns RSSI (0..15) for given channel
const uint8_t CYWM6935::RSSI_peak(const uint8_t channel, const uint8_t count) const
{    
  Write(REG_CHANNEL, channel);       // Set channel    
  Write(REG_CONTROL, 0x80);          // Turn receiver on
  // Wait for receiver to start-up
  // SYNTH_SETTLE (200) + RECEIVER_READY (35) + RSSI_ADC_CONVERSION (50)
  delayMicroseconds(285);
  Write(REG_CARRIER_DETECT, 0x00);  // clear override

  uint8_t value = 0;
  for (uint8_t i = 0; i < count; i++) {
    while (1) {
      uint8_t v = Read(REG_RSSI);  // Read RSSI
      if ((v & 0x20) == 0) {
	Write(REG_CARRIER_DETECT, 0x80);  // set override
	delayMicroseconds(50);   // RSSI_ADC_CONVERSION (50)
	v = Read(REG_RSSI);  // Read RSSI
	Write(REG_CARRIER_DETECT, 0x00);  // clear override 
      } else {
	v = v & RSSI_MASK;
	if (v > value) value = v;
	break;
      }
    }
  }


  Write(REG_CONTROL,0x00);     // Turn receiver off    
    
  return value;
}


// This function returns the average of count readings.
const uint8_t CYWM6935::RSSI_avg(const uint8_t channel, const uint8_t count) const
{    
  Write(REG_CHANNEL, channel);       // Set channel    
  Write(REG_CONTROL, 0x80);          // Turn receiver on
  // Wait for receiver to start-up
  // SYNTH_SETTLE (200) + RECEIVER_READY (35) + RSSI_ADC_CONVERSION (50)
  delayMicroseconds(285);
  Write(REG_CARRIER_DETECT, 0x00);  // clear override

  uint16_t sum = 0;
  for (uint8_t i = 0; i < count; i++) {
    while (1) {
      uint8_t v = Read(REG_RSSI);  // Read RSSI
      if ((v & 0x20) == 0) {
	Write(REG_CARRIER_DETECT, 0x80);  // set override
	delayMicroseconds(50);   // RSSI_ADC_CONVERSION (50)
	v = Read(REG_RSSI);  // Read RSSI
	Write(REG_CARRIER_DETECT, 0x00);  // clear override 
      } else {
	sum += v & RSSI_MASK;
	break;
      }
    }
  }


  Write(REG_CONTROL,0x00);     // Turn receiver off    
    
  return (sum / count);
}
