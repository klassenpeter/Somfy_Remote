#pragma once

#include <inttypes.h>


struct REMOTE
{
    unsigned int id;
    char const *mqtt_topic;
    unsigned int default_rolling_code;
    uint32_t eeprom_address;
};