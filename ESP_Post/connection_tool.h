#ifndef _CONNECTION_TOOL_H_
#define _CONNECTION_TOOL_H_

#include <ESP8266WiFi.h>
extern "C" {
    #include "user_interface.h"
    #include "wpa2_enterprise.h"
}


void connectWPA2(const char* ssid, const char* username,
                 const char* password);


#endif