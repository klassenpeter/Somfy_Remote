# Somfy Remote
An Arduino program able to emulate a Somfy remote control.

Forked from https://github.com/Nickduino/Somfy_Remote.

If you want to learn more about the Somfy RTS protocol, check out [Pushtack](https://pushstack.wordpress.com/somfy-rts-protocol/).


**How the hardware works:**
Connect a *433.42 Mhz* RF transmitter to Pin 23 of the ESP32 (or change the pin in the config file). I ordered 433.42 MHz crystals to replace the one on a 433.92MHz transmitter.
A RTL-SDR comes in handy to check the frequency and make sure the transmitter is working.


**How the software works:**
Edit [config.h](https://github.com/marmotton/Somfy_Remote/blob/master/src/config_EXAMPLE.h) to adapt to your location. You can add or remove remotes to your liking.

The ESP32 will subscribe to the configured MQTT topics. Watch what is happening on the serial port to make sure it is working.

Programming the blinds:
  1) Press the program button on your actual remote. The blinds will move slightly.
  2) Publish 'p' message on the corresponding MQTT topic. The blinds will move slightly.
  3) Done !

Simply publish these messages on the corresponding topics to control your blinds:
  - u (up)
  - d (down)
  - s (stop, my)

The rolling code value is stored in the EEPROM, so that you don't loose count of your rolling code after a reset. In case you'd like to replace the ESP32, write down the current rolling codes which can be read using the serial terminal (and use them as default rolling codes in config.h).
