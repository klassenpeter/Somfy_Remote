#include <vector>
#include <inttypes.h>

// You can add as many remote control emulators as you want by adding elements to the "remotes" vector
// The id and mqtt_topic can have any value but must be unique
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

// Configuration of the remotes that will be emulated
struct REMOTE {
    unsigned int id;
    char const* mqtt_topic;
    unsigned int default_rolling_code;
    uint32_t eeprom_address;
};

//                                 id            mqtt_topic     default_rolling_code     eeprom_address
std::vector<REMOTE> const remotes = {{0x184623, "smartHome/livingRoom/blinds", 1,                0 }
                                    ,{0x971547, "smartHome/office/blinds",     1,                4 }
                                    ,{0x336124, "smartHome/balcony/awning",    1,                8 }
                                    ,{0x187542, "smartHome/kitchen/blinds",    1,               12 }
                                    ,{0x244412, "smartHome/room1/blinds",      1,               16 }
                                    };

// Change reset_rolling_codes to true to clear the rolling codes stored in the non-volatile storage
// The default_rolling_code will be used

const bool reset_rolling_codes = false;

const char*        wifi_ssid = "myWIFISSID";
const char*    wifi_password = "superSecretPassword1234";

const char*      mqtt_server = "serverIpOrName";
const unsigned int mqtt_port = 1883;
const char*        mqtt_user = "username";
const char*    mqtt_password = "secretPassword5678";
const char*          mqtt_id = "esp32-somfy-remote";

const char*     status_topic = "smartHome/somfy-remote/status"; // Online / offline
const char*        ack_topic = "smartHome/somfy-remote/ack"; // Commands ack "id: 0x184623, cmd: u"

#define RFM_CHIP_SELECT 33  // this is the pin used for SPI control.  MUST be connected to the SPI Chip Select pin on the RFM69
#define RFM_RESET_PIN 27    // this is the pin used to reset the RFM.  MUST be connected to the RESET pin on the RFM69
#define RF_FREQUENCY 433.42 // RF frequency (in MHz) for Somfy-RTS system
#define RFM_PORT_TX 4       // Output data on pin 4 (can range from 0 to 31). Check pin numbering on ESP8266.
