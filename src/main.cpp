#include <Arduino.h>
#include <vector>

// Wifi and MQTT
#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#include "RFM69.h"
#include "remote.h"
#include "wifi.hpp"
#include "mqtt_client.h"

/* Adapted to run on ESP32 from original code at https://github.com/Nickduino/Somfy_Remote

This program allows you to emulate a Somfy RTS or Simu HZ remote.
If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/

The rolling code will be stored in non-volatile storage (Preferences), so that you can power the Arduino off.

Serial communication of the original code is replaced by MQTT over WiFi.

Modifications should only be needed in config.h.

*/

#include "config.h"

RFM69 rfm69(RFM_CHIP_SELECT, RFM_RESET_PIN);

// Buttons
#define SYMBOL 640
#define BUTTON_UP 0x2
#define BUTTON_STOP 0x1
#define BUTTON_DOWN 0x4
#define BUTTON_PROG 0x8

byte frame[7];

void BuildFrame(byte *frame, byte button, REMOTE *remote);
void SendCommand(byte *frame, byte sync);
void receivedCallback(char *topic, byte *payload, unsigned int length);

void setup()
{
    // USB serial port
    Serial.begin(115200);

    // Clear all the stored rolling codes (not used during normal operation). Only ESP32 here (ESP8266 further below).

    // Print out all the configured remotes.
    // Also reset the rolling codes for ESP8266 if needed.
    for (REMOTE *remote : remotes)
    {
        if (reset_rolling_codes)
        {
            remote->resetRollingCode();
        }
        Serial.print("Simulated remote number : ");
        Serial.println(remote->getId(), HEX);
        Serial.print("Current rolling code    : ");
        Serial.println(remote->getRollingCode());
    }
    Serial.println();

    connectWiFi();

    // Configure MQTT
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(receivedCallback);

    rfm69.init();
    Serial.println("RFM69 Radio initialized");
}

void loop()
{
    // Reconnect MQTT if needed
    if (!mqtt.connected())
    {
        mqttconnect();
    }

    mqtt.loop();

    delay(100);
}

void receivedCallback(char *topic, byte *payload, unsigned int length)
{
    char command = *payload; // 1st byte of payload
    bool commandIsValid = false;
    REMOTE *currentRemote = nullptr;

    Serial.print("MQTT message received: ");
    Serial.println(topic);

    Serial.print("Payload: ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Command is valid if the payload contains one of the chars below AND the topic corresponds to one of the remotes
    if (length == 1 && (command == 'u' || command == 's' || command == 'd' || command == 'p'))
    {
        for (REMOTE *remote : remotes)
        {
            if (strcmp(remote->getMqttTopic(), topic) == 0)
            {
                currentRemote = remote;
                commandIsValid = true;
            }
        }
    }

    if (commandIsValid)
    {
        if (command == 'u')
        {
            Serial.println("Monte"); // Somfy is a French company, after all.
            BuildFrame(frame, BUTTON_UP, currentRemote);
        }
        else if (command == 's')
        {
            Serial.println("Stop");
            BuildFrame(frame, BUTTON_STOP, currentRemote);
        }
        else if (command == 'd')
        {
            Serial.println("Descend");
            BuildFrame(frame, BUTTON_DOWN, currentRemote);
        }
        else if (command == 'p')
        {
            Serial.println("Prog");
            BuildFrame(frame, BUTTON_PROG, currentRemote);
        }

        Serial.println("");

        SendCommand(frame, 2);
        for (int i = 0; i < 2; i++)
        {
            SendCommand(frame, 7);
        }

        // Send the MQTT ack message
        String ackString = "id: 0x";
        ackString.concat(String(currentRemote->getId(), HEX));
        ackString.concat(", cmd: ");
        ackString.concat(command);
        mqtt.publish(ack_topic, ackString.c_str());
    }
}

void BuildFrame(byte *frame, byte button, REMOTE *remote)
{
    unsigned int code = remote->getRollingCode();

    frame[0] = 0xA7;                  // Encryption key. Doesn't matter much
    frame[1] = button << 4;           // Which button did  you press? The 4 LSB will be the checksum
    frame[2] = code >> 8;             // Rolling code (big endian)
    frame[3] = code;                  // Rolling code
    frame[4] = remote->getId() >> 16; // Remote address
    frame[5] = remote->getId() >> 8;  // Remote address
    frame[6] = remote->getId();       // Remote address

    Serial.print("Frame         : ");
    for (byte i = 0; i < 7; i++)
    {
        Serial.printf(" %02X", frame[i]);
    }

    // Checksum calculation: a XOR of all the nibbles
    byte checksum = 0;
    for (byte i = 0; i < 7; i++)
    {
        checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
    }
    checksum &= 0b1111; // We keep the last 4 bits only

    // Checksum integration
    frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will consider the checksum ok.

    Serial.println("");
    Serial.print("With checksum : ");
    for (byte i = 0; i < 7; i++)
    {
        Serial.printf(" %02X", frame[i]);
    }

    // Obfuscation: a XOR of all the bytes
    for (byte i = 1; i < 7; i++)
    {
        frame[i] ^= frame[i - 1];
    }

    Serial.println("");
    Serial.print("Obfuscated    : ");
    for (byte i = 0; i < 7; i++)
    {
        Serial.printf(" %02X", frame[i]);
    }
    Serial.println("");
    Serial.print("Rolling Code  : ");
    Serial.println(code);

    remote->setRollingCode(code + 1);
}

void SendCommand(byte *frame, byte sync)
{
    rfm69.enterTxMode();

    if (sync == 2)
    { // Only with the first frame.
        // Wake-up pulse & Silence
        digitalWrite(RFM_PORT_TX, HIGH);
        delayMicroseconds(9415);
        digitalWrite(RFM_PORT_TX, LOW);
        delayMicroseconds(89565);
    }

    // Hardware sync: two sync for the first frame, seven for the following ones.
    for (int i = 0; i < sync; i++)
    {
        digitalWrite(RFM_PORT_TX, HIGH);
        delayMicroseconds(4 * SYMBOL);
        digitalWrite(RFM_PORT_TX, LOW);
        delayMicroseconds(4 * SYMBOL);
    }

    // Software sync
    digitalWrite(RFM_PORT_TX, HIGH);
    delayMicroseconds(4550);
    digitalWrite(RFM_PORT_TX, LOW);
    delayMicroseconds(SYMBOL);

    // Data: bits are sent one by one, starting with the MSB.
    for (byte i = 0; i < 56; i++)
    {
        if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1)
        {
            digitalWrite(RFM_PORT_TX, LOW);
            delayMicroseconds(SYMBOL);
            digitalWrite(RFM_PORT_TX, HIGH);
            delayMicroseconds(SYMBOL);
        }
        else
        {
            digitalWrite(RFM_PORT_TX, HIGH);
            delayMicroseconds(SYMBOL);
            digitalWrite(RFM_PORT_TX, LOW);
            delayMicroseconds(SYMBOL);
        }
    }

    digitalWrite(RFM_PORT_TX, LOW);
    delayMicroseconds(30415); // Inter-frame silence

    rfm69.enterStandbyMode();
}
