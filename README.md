onewire-wireless-network
========================

Building my wireless OneWire network

(Loosely based on work from Gérard Chevalier and others

## Description

This code runs on Arduino compatibles (including JeeLinks, JeeNodes, JeeNode Micros (JNµ)). Connect one or multiple DS18B20
sensors (or any other 1-Wire sensor) to the nodes, and they should send their data over
RF to a listening device (eg. a JeeLink running the w1receiver). Run the 'processing'(https://github.com/jovandeginste/onewire-wireless-network-processing) script on the computer connected to the JeeLink

JeeNodes can be programmed over USB, JeeNode micro's over ISP.

## Configure the Arduino IDE

_tested and working with Arduino IDE 1.5.3_
(compilation issues with IDE's 1.5.8 or 1.6.0 for JeeNode micro)

Patch for Arduino IDE needed: http://forum.arduino.cc/index.php/topic,116674.0.html

Copy the Arduino dependencies from [this repository](https://github.com/jovandeginste/onewire-wireless-network-dependencies) to your Sketchbook directory (or find them yourself)
* hardware (for JNµ):
	* jeelabs: https://github.com/jcw/ide-hardware
* libraries:
	* JeeLib: https://github.com/jcw/jeelib
	* OneWire: http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip

## References

### Arduino als ISP:

http://jeelabs.org/2010/07/02/fixing-a-faulty-atmega-arduino/

### ISP pins:

http://jeelabs.net/projects/hardware/wiki/Flash_Board

### Programming JNµ:

http://jeelabs.org/2013/03/21/programming-the-jn%C2%B5-at-last/

### Tiny ino DS18B20:

https://github.com/nathanchantrell/TinyTX/blob/master/TinyTX_DS18B20/TinyTX_DS18B20.ino

### Tiny pins:

http://forum.jeelabs.net/node/697.html
