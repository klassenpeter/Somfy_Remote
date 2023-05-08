#pragma once

// You can add as many remote control emulators as you want by adding elements to the "remotes" vector
// The id and mqttName can have any value but must be unique
// default_rolling_code can be any unsigned int, usually leave it at 1
// eeprom_address must be incremented by 4 for each remote

// Once the programe is uploaded on the ESP32:
// - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
// - send 'p' using MQTT on the corresponding topic
// - You can use the same remote control emulator for multiple blinds, just repeat these steps.
//
// Then:
// - u will make it go up
// - s make it stop
// - d will make it go down

#include <vector>
#include "remote.h"

#define HA_PREFIX "homeassistant"
//             id
//             |          we need to pass the HA_PREFIX here
//             |          |          mqttName
//             |          |          |            default_rolling_code
//             |          |          |            |  eeprom_address
//             |          |          |            |  |
std::vector<REMOTE *> const remotes = {
    new REMOTE(0x184623, HA_PREFIX, "livingRoom", 1, 0),
    new REMOTE(0x971547, HA_PREFIX, "office",     1, 4),
    new REMOTE(0x336124, HA_PREFIX, "balcony",    1, 8),
    new REMOTE(0x187542, HA_PREFIX, "kitchen",    1, 12),
    new REMOTE(0x244412, HA_PREFIX, "room1",      1, 16) //
};

// Change reset_rolling_codes to true to clear the rolling codes stored in the non-volatile storage
// The default_rolling_code will be used

const bool reset_rolling_codes = false;

const char *wifi_ssid = "myWIFISSID";
const char *wifi_password = "superSecretPassword1234";

const char *mqtt_server = "serverIpOrName";
const unsigned int mqtt_port = 1883;
const char *mqtt_user = "username";
const char *mqtt_password = "secretPassword5678";
const char *mqtt_id = "esp32-somfy-remote";

const char *status_topic = HA_PREFIX "/cover/somfy-remote/status"; // Online / offline
const char *ack_topic = HA_PREFIX "/cover/somfy-remote/ack";       // Commands ack "id: 0x184623, cmd: u"

#define RFM_CHIP_SELECT 33  // this is the pin used for SPI control.  MUST be connected to the SPI Chip Select pin on the RFM69
#define RFM_RESET_PIN 27    // this is the pin used to reset the RFM.  MUST be connected to the RESET pin on the RFM69
#define RF_FREQUENCY 433.42 // RF frequency (in MHz) for Somfy-RTS system
#define RFM_PORT_TX 4       // Output data on pin 4 (can range from 0 to 31). Check pin numbering on ESP8266.
