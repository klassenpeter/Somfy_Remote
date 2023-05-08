#pragma once

#include <inttypes.h>

class REMOTE_BASE
{
public:
    REMOTE_BASE(unsigned int _id,
                char const *_prefix,
                char const *_mqtt_name,
                unsigned int _default_rolling_code,
                uint32_t _eeprom_address) : id(_id),
                                            mqttName(_mqtt_name),
                                            mqttPrefix(_prefix),
                                            default_rolling_code(_default_rolling_code),
                                            eeprom_address(_eeprom_address){};

    void resetRollingCode()
    {
        setRollingCode(default_rolling_code);
    }

    virtual void setRollingCode(unsigned int code) = 0;
    virtual unsigned int getRollingCode() { return default_rolling_code; };

    unsigned int getId() { return id; }
    char const *getMQTT_name()
    {
        return mqttName;
    }
    char *getMQTT_topic(const char *append = "")
    {

        if (strcmp("", append) == 0)
        {
            snprintf(
                mqttTopic, sizeof(mqttTopic),
                "%s/cover/%s", mqttPrefix, mqttName);
        }
        else
        {
            snprintf(
                mqttTopic, sizeof(mqttTopic),
                "%s/cover/%s/%s", mqttPrefix, mqttName, append);
        }

        return mqttTopic;
    }

protected:
    unsigned int id;
    char const *mqttPrefix;
    char const *mqttName;
    char mqttTopic[100];
    unsigned int default_rolling_code;
    uint32_t eeprom_address;
};

// Open storage for storing the rolling codes
