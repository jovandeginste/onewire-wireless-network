You can use this to send data to graphite (https://github.com/graphite-project)

Steps needed:

1. complete the mapping.yaml
   
   An example is included, but basically you need to map any unique hex id to a graphite (sub)path

   The unit will send a heartbeat every cycle, which will have '0000XX0000000001' as id (XX is it's
   unique id programmed via the firmware)

   Every OneWire sensor will be transmitted over via the OneWire unique id; for DS18B20 temperature
   sensors, the id starts with '28'.

   The data should sent over as is to minimize power consumption on the sensors, and is processed
   by the ruby script here.

2. install the ruby dependencies - either or not using rvm
   
   ```bash
   bundle install
   ```

3. try the script
   
   ```bash
   ruby receive.rb
   ```

4. send the output to graphite
   
   ```bash
   ruby receive.rb | nc your.graphite.server 2003
   ```
