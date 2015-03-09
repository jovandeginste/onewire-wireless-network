/*
   Description
   This code runs on both JeeNodes and JeeNode Micros. Connect one
   or multiple DS18B20 sensors (or any other 1-Wire sensor) to it,
   and it should send it's data over RF to a listening device (eg.
   a JeeLink running the w1receiver)

   JeeNodes can be programmed over USB, JeeNode micro's over ISP.

References:
Arduino IDE libs:
 * http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
 * http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip
 * https://github.com/jcw/jeelib

 Arduino als ISP:
 * http://jeelabs.org/2010/07/02/fixing-a-faulty-atmega-arduino/
 ISP pins:
 * http://jeelabs.net/projects/hardware/wiki/Flash_Board

 Programming jnµ:
 * http://jeelabs.org/2013/03/21/programming-the-jn%C2%B5-at-last/

 Tiny ino DS18B20:
 * https://github.com/nathanchantrell/TinyTX/blob/master/TinyTX_DS18B20/TinyTX_DS18B20.ino
 Tiny pins:
 * http://forum.jeelabs.net/node/697.html

 Loosely based on work from Gérard Chevalier, Jan 2011
 */

#include <OneWire.h>
#include <JeeLib.h>
#include <avr/eeprom.h>

//#define BLINK 1
#define INTERVAL    50000  // Interval with which to send updates
#define LED_PIN     9      // activity LED, comment out to disable
#define RF_RECEIVER_ID	1   // RF id of the receiver
#define RF_NETWORK_ID	33  // RF id of the network (should be same on receiver and senders)
#define MY_SENDER_ID	14  // RF id of this unit - comment this if unit was already configured before

static void activityLed (byte on) {
#ifdef LED_PIN
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, !on);
#endif
}

#if defined(__AVR_ATtiny84__)

/*
   JeeNode micro and other ATtiny's

   Connect the sensor to the JNµ as follows:

 * DS18B20 gnd	->	JNµ gnd
 * DS18B20 DQ	->	JNµ DIO/PA0 (ATtiny pin 13)
 * DS18B20 Vdd	->	JNµ AIO/PA1 (ATtiny pin 12)

 */
#define ONE_WIRE_BUS 10
#define ONE_WIRE_POWER 9
#else

/*
   JeeNode and other ATmega's

   Connect the sensor to the JN as follows:

 * DS18B20 gnd	->	JN Port 1 G
 * DS18B20 DQ	->	JN Port 1 D (ATmega pin 4)
 * DS18B20 Vdd	->	JN Port 1 +

 */
#define ONE_WIRE_BUS 4
#endif


OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance

byte TempRead, CountRemain;
int stamp;
byte TempBinFormat[15];
byte addr[8];
byte *temp;
byte my_id;
unsigned long int longInt;
unsigned char byteArray[4];

void setup() {
	cli();
	CLKPR = bit(CLKPCE);
#if defined(__AVR_ATtiny84__)
	CLKPR = 0; // div 1, i.e. speed up to 8 MHz
#else
	CLKPR = 1; // div 2, i.e. slow down to 8 MHz
#endif
	sei();

#if defined(__AVR_ATtiny84__)
	// power up the radio on JMv3
	bitSet(DDRB, 0);
	bitClear(PORTB, 0);
#endif

	my_id = (byte)eeprom_read_byte(0);
#ifdef MY_SENDER_ID
	// If MY_SENDER_ID is defined and different, we will also save it in EEPROM
	if (my_id != MY_SENDER_ID) {
		eeprom_write_byte(0, MY_SENDER_ID);
	}
#endif

	rf12_initialize(my_id, RF12_868MHZ, RF_NETWORK_ID);
	activityLed(0);
	// see http://tools.jeelabs.org/rfm12b
	rf12_control(0xC040); // set low-battery level to 2.2V i.s.o. 3.1V
	rf12_sleep(0);                          // Put the RFM12 to sleep

#ifdef ONE_WIRE_POWER
	pinMode(ONE_WIRE_POWER, OUTPUT); // set power pin for DS18B20 to output
#endif
}

bool Next1820(byte *addr) {
	byte i;
	if ( !oneWire.search(addr)) {
		oneWire.reset_search();
		return false;
	}

	if ( OneWire::crc8( addr, 7) != addr[7]) {
		return false;
	}
	return true;
}

byte *Get1820Tmp(byte *addr) {
	byte i;
	byte present = 0;
	byte OneWData[12];

	oneWire.reset();
	oneWire.select(addr);
	oneWire.write(0x44,1);         // start conversion, with parasite power on at the end

	delay(1000);     // maybe 750ms is enough, maybe not

	present = oneWire.reset();
	oneWire.select(addr);
	oneWire.write(0xBE);         // Read Scratchpad

	for ( i = 0; i < 9; i++) {           // we need 9 bytes
		OneWData[i] = oneWire.read();
	}

	return OneWData;
}

void rf12_send(byte header, const void* data, byte length) {
	rf12_recvDone();
	if (rf12_canSend()) {
#ifdef BLINK
		activityLed(1);
#endif
		rf12_sendStart(header, data, length);
		rf12_sendWait(1);
		delay(50);
#ifdef BLINK
		activityLed(0);
#endif
		delay(50);
	}
}

ISR(WDT_vect) { 
	Sleepy::watchdogEvent(); 
}

void loop() {
	stamp = (int)millis();
	rf12_sleep(RF12_WAKEUP);
	sendLocalData();
	checkAllTemps();

	rf12_sendWait(2);
	rf12_sleep(RF12_SLEEP);
	Sleepy::loseSomeTime(INTERVAL + stamp - (int)millis());
}

void sendLocalData() {
	memcpy(&TempBinFormat[0], "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 15);
	memcpy(&TempBinFormat[2], &my_id, 1);
	memcpy(&TempBinFormat[7], "\1", 1);

	// Heartbeat :-)
	memcpy(&TempBinFormat[8], "\1", 1);
	longInt = millis()/1000;
	byteArray[3] = (int)((longInt >> 24) & 0xFF) ;
	byteArray[2] = (int)((longInt >> 16) & 0xFF) ;
	byteArray[1] = (int)((longInt >> 8) & 0XFF);
	byteArray[0] = (int)((longInt & 0XFF));
	memcpy(&TempBinFormat[9], &byteArray, 4);
	rf12_send(RF_RECEIVER_ID, TempBinFormat, 15);
}

void checkAllTemps() {
	// Note that even if you only want to send out packets, you still have to call rf12 recvDone periodically, because
	// it keeps the RF12 logic going. If you don't, rf12_canSend() will never return true.
	while (Next1820(addr)) {
		temp = Get1820Tmp(addr);

		memcpy(&TempBinFormat[0], addr, 8); 
		memcpy(&TempBinFormat[8], temp, 2); 
		memcpy(&TempBinFormat[10], "\0\0\0\0\0", 5); 

		rf12_send(RF_NETWORK_ID, TempBinFormat, 15);
	}
}
