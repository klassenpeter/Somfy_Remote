#pragma once

#include <inttypes.h>

class REMOTE_BASE
{
public:
    REMOTE_BASE(unsigned int _id,
                char const *_mqtt_topic,
                unsigned int _default_rolling_code,
                uint32_t _eeprom_address) : id(_id),
                                            mqtt_topic(_mqtt_topic),
                                            default_rolling_code(_default_rolling_code),
                                            eeprom_address(_eeprom_address){};

    void resetRollingCode()
    {
        setRollingCode(default_rolling_code);
    }

    virtual void setRollingCode(unsigned int code) = 0;
    virtual unsigned int getRollingCode() { return default_rolling_code; };

    unsigned int getId() { return id; }
    char const *getMqttTopic() { return mqtt_topic; }

protected:
    unsigned int id;
    char const *mqtt_topic;
    unsigned int default_rolling_code;
    uint32_t eeprom_address;
};

// Open storage for storing the rolling codes
