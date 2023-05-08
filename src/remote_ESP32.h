#pragma once

#include <inttypes.h>
#include <remote_base.h>

// Store the rolling codes in NVS
#include <Preferences.h>

class REMOTE_ESP32 : public REMOTE_BASE
{
public:
    REMOTE_ESP32(unsigned int _id,
                 char const *_prefix,
                 char const *_mqtt_name,
                 unsigned int _default_rolling_code,
                 uint32_t _eeprom_address) : REMOTE_BASE(_id, _prefix, _mqtt_name, _default_rolling_code, _eeprom_address)
    {
        myPrefName = (String(_id) + "_remote").c_str();
    }

    virtual void setRollingCode(unsigned int code)
    {
        preferences.begin(myPrefName.c_str(), false);
        preferences.putUInt("rolling_code", code);
    }

    virtual unsigned int getRollingCode()
    {
        preferences.begin(myPrefName.c_str(), false);
        return preferences.getUInt("rolling_code", default_rolling_code);
    }

private:
    Preferences preferences;

    String myPrefName;
};