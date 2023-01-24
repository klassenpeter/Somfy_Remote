#pragma once



#ifdef ESP32
    #include <remote_ESP32.h>
    typedef  REMOTE_ESP32 REMOTE;
#elif ESP8266
    #include <remote_ESP8266.h>
    typedef REMOTE_ESP8266 REMOTE;

#endif