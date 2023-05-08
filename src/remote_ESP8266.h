#pragma once

#include <inttypes.h>
#include <remote_base.h>

// Store the rolling codes in NVS
#include <EEPROM.h>

class REMOTE_ESP8266 : public REMOTE_BASE
{
public:
    REMOTE_ESP8266(unsigned int _id,
                   char const *_prefix,
                   char const *_mqtt_name,
                   unsigned int _default_rolling_code,
                   uint32_t _eeprom_address) : REMOTE_BASE(_id, *_prefix, _mqtt_name, _default_rolling_code, _eeprom_address){};

    void setRollingCode(unsigned int code)
    {
        EEPROM.put(eeprom_address, code);
        EEPROM.commit();
    };

    unsigned int getRollingCode()
    {
        unsigned int code;
        EEPROM.get(eeprom_address, code);
        return code;
    };

    // private:
};

EEPROM.begin(1024);
