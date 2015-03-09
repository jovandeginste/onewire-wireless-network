/// @dir RF12demo
/// Configure some values in EEPROM for easy config of the RF12 later on.
// 2009-05-06 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <OneWire.h>
#include <RF12.h>
#include <Ports.h>
#include <avr/sleep.h>

#define LED_PIN     9   // activity LED, comment out to disable

#define RF_RECEIVER_ID	1   // RF id of the receiver
#define RF_NETWORK_ID	33  // RF id of the network (should be same on receiver and senders)

static void activityLed (byte on) {
#ifdef LED_PIN
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, !on);
#endif
}

void setup() {
	Serial.begin(57600);

	rf12_initialize(RF_RECEIVER_ID, RF12_868MHZ, RF_NETWORK_ID);
	Serial.println("OK ");
	activityLed(0);
}

void loop() {
	if (rf12_recvDone()) {
		byte n = rf12_len;
		if (rf12_crc == 0) {
			activityLed(1);
			Serial.print("OK ");
			Serial.print(millis());
			Serial.print((int) rf12_hdr);
			for (byte i = 0; i < n; ++i) {
				Serial.print(' ');
				Serial.print((int) rf12_data[i]);
			}
			Serial.println();

			activityLed(0);
		}
	}
}
