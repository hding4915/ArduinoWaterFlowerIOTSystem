#ifndef _CONNECTION_TOOL_H_
#define _CONNECTION_TOOL_H_

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiUdp.h>
  #include "Seeed_mbedtls.h"
  extern "C" {
    #include "user_interface.h"
    #include "wpa2_enterprise.h"
  }
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <WiFiUdp.h>
  #include "mbedtls/md.h"
  extern "C" {
    #include "esp_eap_client.h"
  }
#endif

#include <WiFiClientSecure.h>

#include "Arduino.h"


extern WiFiUDP Udp;
extern const char* secret_key;

#pragma once
extern const char* root_CA;


void connectWPA2(const char *ssid, const char *username, const char *password);
void sendAuthentication(IPAddress rpi_ip, uint16_t rpi_port, String device_id);
String generateHMAC(const String& message, const String& key);
void showHttpData(String url);
void sendHttpData(String payload, String targetUrl);
void sendHumiHttpData(String deviceID, String targetUrl, int humi);
void handleHttpPostResponse(HTTPClient &http, int httpCode);
void handleHttpGetResponse(HTTPClient &http, int httpCode);

#endif
