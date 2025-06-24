

#include "connection_tool.h"

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* deviceID = "esp_sender";

const int serverPort = 443;


char incomingPacket[256];
const int udpPort = 4210;
const int confirmPort = 4211;
boolean recvServerIP = true;



String serverIP = "esprun.hding49.uk";
String targetUrl = "";
String recvUrl = "";
String confirmUrl = "";

void setup() {
    pinMode(2, INPUT);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.begin(115200);
    Serial.println("Trying to connect to " + String(ssid));
    Serial.println("Password: " + String(password));
    WiFi.begin(ssid, password);

    Serial.print("\nConnecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected, IP：" + WiFi.localIP().toString());

    delay(1000);
}


char recv;
bool is_set = false;

void loop() {
    if (!recvServerIP) {
        findLocalServer();
    } else {
        // For sender
        int sensorVal = analogRead(A0);
        sendHumiHttpData(deviceID, targetUrl, sensorVal);

        if (!is_set) {
            targetUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/data";
            recvUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/last_data";
            Serial.println(targetUrl);
            Serial.println(recvUrl);
            is_set = true;
        }

        // For receiver
        int recvHumi = getHumidityFromServer(recvUrl);
        yield();

        Serial.println("Humi: " + String(recvHumi));

        if (recvHumi != -1) {
            Serial2.print("<H:");
            Serial2.print(String(recvHumi));
            Serial2.println(">");
        }
        delay(5000);
    }
}


void findLocalServer() {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        int len = Udp.read(incomingPacket, 255);
        if (len > 0) incomingPacket[len] = 0;

        Serial.print("Receive UDP broadcast：");
        Serial.println(incomingPacket);

        String packetStr = String(incomingPacket);


        // FLASK_SERVER_IP:192.168.0.x|hmac_hash
        int sepIndex = packetStr.indexOf('|');
        if (sepIndex > 0) {
            String message = packetStr.substring(0, sepIndex);
            String received_hmac = packetStr.substring(sepIndex + 1);
            String calculated_hmac = generateHMAC(message, secret_key);

            Serial.println("[ESP] HMAC: " + calculated_hmac);

            if (calculated_hmac.equalsIgnoreCase(received_hmac)) {
                Serial.println("[ESP] ✅ HMAC verification succeeded");

                // Get RPi IP
                serverIP = message.substring(message.indexOf(':') + 1);
                IPAddress serverIPAddr;
                serverIPAddr.fromString(serverIP);

                Serial.println("Server IP：" + serverIP);

                sendAuthentication(serverIPAddr, confirmPort, deviceID);
                //   sendAuthentication(serverIPAddr, confirmPort, "esp8266_002");


                Serial.println("[ESP] Send confirmation messages to RPi");

                recvServerIP = true;

                targetUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/data";
                recvUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/last_data";


                delay(1000);
            } else {
                Serial.println("[ESP] ❌ HMAC verification failed. Dropped the packet");
            }
        }
    }
}
