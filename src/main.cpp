#include <Arduino.h>
#include <vector>

/* Adapted to run on ESP32 from original code at https://github.com/Nickduino/Somfy_Remote

This program allows you to emulate a Somfy RTS or Simu HZ remote.
If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/

The rolling code will be stored in non-volatile storage (Preferences), so that you can power the Arduino off.

Serial communication of the original code is replaced by MQTT over WiFi.

Modifications should only be needed in config.h.

*/

// Store the rolling codes in NVS
#include <Preferences.h>
Preferences preferences;

// Configuration of the remotes that will be emulated
struct REMOTE {
    unsigned int id;
    char const* mqtt_topic;
    unsigned int default_rolling_code;
};

#include "config.h"

// Wifi and MQTT
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8

byte frame[7];

void BuildFrame(byte *frame, byte button, REMOTE remote);
void SendCommand(byte *frame, byte sync);
void receivedCallback(char* topic, byte* payload, unsigned int length);
void mqttconnect();

void setup() {
    // USB serial port
    Serial.begin(115200);

    // Output to 433MHz transmitter
    pinMode(PORT_TX, OUTPUT);
    GPIO.out_w1tc = 1 << PORT_TX; // Output = 0

    // Connect to WiFi
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Configure MQTT
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(receivedCallback);

    // Open storage for storing the rolling codes
    preferences.begin("somfy-remote", false);

    // Clear all the stored rolling codes (not used during normal operation)
    #ifdef RESET_ROLLING_CODES
        preferences.clear();
    #endif

    for ( REMOTE remote : remotes ) {
        Serial.print("Simulated remote number : ");
        Serial.println(remote.id, HEX);
        Serial.print("Current rolling code    : ");
        Serial.println( preferences.getUInt( (String(remote.id) + "rolling").c_str(), remote.default_rolling_code) );
    }
    Serial.println();
}

void loop() {

    if ( !mqtt.connected() ) {
        mqttconnect();
    }

    mqtt.loop();
}

void mqttconnect() {
    // Loop until reconnected
    while ( !mqtt.connected() ) {
        Serial.print("MQTT connecting ...");

        if (mqtt.connect(mqtt_id, mqtt_user, mqtt_password)) {
            Serial.println("connected");
            // Subscribe topics with QoS 1
            for ( REMOTE remote : remotes ) {
                mqtt.subscribe(remote.mqtt_topic, 1);
                Serial.print("Subscribed to topic: ");
                Serial.println(remote.mqtt_topic);
            }
        }
        else {
            Serial.print("failed, status code =");
            Serial.print(mqtt.state());
            Serial.println("try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void receivedCallback(char* topic, byte* payload, unsigned int length) {
    char command = *payload; // 1st byte of payload
    bool commandIsValid = false;
    REMOTE currentRemote;

    Serial.print("MQTT message received: ");
    Serial.println(topic);

    Serial.print("Payload: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Command is valid if the payload contains one of the chars below AND the topic corresponds to one of the remotes
    if ( length == 1 && ( command == 'u' || command == 's' || command == 'd' || command == 'p' ) ) {
        for ( REMOTE remote : remotes ) {
            if ( strcmp(remote.mqtt_topic, topic) == 0 ){
                currentRemote = remote;
                commandIsValid = true;
            }
        }
    }

    if ( commandIsValid ) {
        if ( command == 'u' ) {
            Serial.println("Monte"); // Somfy is a French company, after all.
            BuildFrame(frame, HAUT, currentRemote);
        }
        else if ( command == 's' ) {
            Serial.println("Stop");
            BuildFrame(frame, STOP, currentRemote);
        }
        else if ( command == 'd' ) {
            Serial.println("Descend");
            BuildFrame(frame, BAS, currentRemote);
        }
        else if ( command == 'p' ) {
            Serial.println("Prog");
            BuildFrame(frame, PROG, currentRemote);
        }

        Serial.println("");

        SendCommand(frame, 2);
        for ( int i = 0; i<2; i++ ) {
            SendCommand(frame, 7);
        }
    }
}

void BuildFrame(byte *frame, byte button, REMOTE remote) {
    unsigned int code = preferences.getUInt( (String(remote.id) + "rolling").c_str(), remote.default_rolling_code);

    frame[0] = 0xA7;            // Encryption key. Doesn't matter much
    frame[1] = button << 4;     // Which button did  you press? The 4 LSB will be the checksum
    frame[2] = code >> 8;       // Rolling code (big endian)
    frame[3] = code;            // Rolling code
    frame[4] = remote.id >> 16; // Remote address
    frame[5] = remote.id >>  8; // Remote address
    frame[6] = remote.id;       // Remote address

    Serial.print("Frame         : ");
    for(byte i = 0; i < 7; i++) {
        if(frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant nibble is a 0.
            Serial.print("0");
        }
        Serial.print(frame[i],HEX); Serial.print(" ");
    }

    // Checksum calculation: a XOR of all the nibbles
    byte checksum = 0;
    for(byte i = 0; i < 7; i++) {
        checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
    }
    checksum &= 0b1111; // We keep the last 4 bits only


    // Checksum integration
    frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will consider the checksum ok.

    Serial.println(""); Serial.print("With checksum : ");
    for(byte i = 0; i < 7; i++) {
        if(frame[i] >> 4 == 0) {
            Serial.print("0");
        }
        Serial.print(frame[i],HEX); Serial.print(" ");
    }


    // Obfuscation: a XOR of all the bytes
    for(byte i = 1; i < 7; i++) {
        frame[i] ^= frame[i-1];
    }

    Serial.println(""); Serial.print("Obfuscated    : ");
    for(byte i = 0; i < 7; i++) {
        if(frame[i] >> 4 == 0) {
            Serial.print("0");
        }
        Serial.print(frame[i],HEX); Serial.print(" ");
    }
    Serial.println("");
    Serial.print("Rolling Code  : ");
    Serial.println(code);

    preferences.putUInt( (String(remote.id) + "rolling").c_str(), code + 1); // Increment and store the rolling code
}

void SendCommand(byte *frame, byte sync) {
    if(sync == 2) { // Only with the first frame.
        //Wake-up pulse & Silence
        GPIO.out_w1ts = 1 << PORT_TX;
        delayMicroseconds(9415);
        GPIO.out_w1tc = 1 << PORT_TX;
        delayMicroseconds(89565);
    }

    // Hardware sync: two sync for the first frame, seven for the following ones.
    for (int i = 0; i < sync; i++) {
        GPIO.out_w1ts = 1 << PORT_TX;
        delayMicroseconds(4*SYMBOL);
        GPIO.out_w1tc = 1 << PORT_TX;
        delayMicroseconds(4*SYMBOL);
    }

    // Software sync
    GPIO.out_w1ts = 1 << PORT_TX;
    delayMicroseconds(4550);
    GPIO.out_w1tc = 1 << PORT_TX;
    delayMicroseconds(SYMBOL);


    //Data: bits are sent one by one, starting with the MSB.
    for(byte i = 0; i < 56; i++) {
        if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
            GPIO.out_w1tc = 1 << PORT_TX; // PORTD &= !(1<<PORT_TX);
            delayMicroseconds(SYMBOL);
            GPIO.out_w1ts = 1 << PORT_TX; // PORTD ^= 1<<5;
            delayMicroseconds(SYMBOL);
        }
        else {
            GPIO.out_w1ts = 1 << PORT_TX; // PORTD |= (1<<PORT_TX);
            delayMicroseconds(SYMBOL);
            GPIO.out_w1tc = 1 << PORT_TX; // PORTD ^= 1<<5;
            delayMicroseconds(SYMBOL);
        }
    }

    GPIO.out_w1tc = 1 << PORT_TX;
    delayMicroseconds(30415); // Inter-frame silence
}
