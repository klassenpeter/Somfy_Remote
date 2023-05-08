#pragma once

#include "config.h"
#include <PubSubClient.h>
#include "wifi.hpp"

PubSubClient mqtt(wifiClient);

bool mqttPublishConfig(REMOTE *r)
{
    char payLoad[500];
    snprintf(payLoad, sizeof(payLoad),
             "{"
             "\"~\":\"%s\","
             "\"name\":\"MQTT Cover %s\","
             "\"uniq_id\":\"%X\","
             "\"cmd_t\":\"~/set\","
             "\"dev_cla\":\"shutter\","
             "\"qos\":0,"
             "\"ret\":true,"
             "\"pl_open\":\"u\","
             "\"pl_cls\":\"d\","
             "\"pl_stop\":\"s\","
             "\"avty_t\":\"%s\","
             "\"pl_avail\":\"online\","
             "\"pl_not_avail\":\"offline\","
             "\"opt\":true"
             "}",
             r->getMQTT_topic(""),
             r->getMQTT_name(),
             r->getId(),
             status_topic);
    return mqtt.publish(r->getMQTT_topic("config"), payLoad);
}

void mqttconnect()
{
    // Loop until reconnected
    while (!mqtt.connected())
    {
        Serial.print("MQTT connecting ...");

        if (mqtt.setBufferSize(500))
            Serial.print("buffer size increased");

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

                if (!mqttPublishConfig(remote))
                    Serial.println("pub failed, mqtt buffer too small?");
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
