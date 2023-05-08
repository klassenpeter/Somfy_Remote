#pragma once

#include "config.h"
#include <PubSubClient.h>
#include "wifi.hpp"

PubSubClient mqtt(wifiClient);


void mqttconnect()
{
    // Loop until reconnected
    while (!mqtt.connected())
    {
        Serial.print("MQTT connecting ...");

        // Connect to MQTT, with retained last will message "offline"
        if (mqtt.connect(mqtt_id, mqtt_user, mqtt_password, status_topic, 1, 1, "offline"))
        {
            Serial.println("connected");

            mqtt.publish(status_topic, "online", true);
            // Subscribe to the topic of each remote with QoS 1
            for (REMOTE *remote : remotes)
            {
                mqtt.subscribe(remote->getMQTT_topic("set"), 1);
                Serial.print("Subscribed to topic: ");
                Serial.println(remote->getMQTT_topic("set"));
            }

            // Update status, message is retained
        }
        else
        {
            Serial.print("failed, status code =");
            Serial.print(mqtt.state());
            Serial.println("try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    Serial.println("mqtt connected");
}
