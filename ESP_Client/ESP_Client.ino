

#include "connection_tool.h"

const char* ssid = "HDING49_2.4G";
const char* password = "0413wifi";
const char* deviceID = "esp8266_002";

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
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n已連上WiFi，IP位址：" + WiFi.localIP().toString());

  Udp.begin(udpPort);
  Serial.println("開始監聽UDP廣播");
}


char recv;
bool is_set = false;

void loop() {
  if (!recvServerIP) {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      int len = Udp.read(incomingPacket, 255);
      if (len > 0) incomingPacket[len] = 0;

      Serial.print("收到UDP廣播：");
      Serial.println(incomingPacket);

      String packetStr = String(incomingPacket);


      // 解析格式：FLASK_SERVER_IP:192.168.0.x|hmac_hash
      int sepIndex = packetStr.indexOf('|');
      if (sepIndex > 0) {
        String message = packetStr.substring(0, sepIndex);
        String received_hmac = packetStr.substring(sepIndex + 1);
        String calculated_hmac = generateHMAC(message, secret_key);

        Serial.println("[ESP] 計算 HMAC: " + calculated_hmac);

        if (calculated_hmac.equalsIgnoreCase(received_hmac)) {
          Serial.println("[ESP] ✅ HMAC 驗證成功");

          // 取得 RPi IP
          serverIP = message.substring(message.indexOf(':') + 1);
          IPAddress serverIPAddr;
          serverIPAddr.fromString(serverIP);

          Serial.println("抓到Server IP：" + serverIP);

          sendAuthentication(serverIPAddr, confirmPort, deviceID);
          //   sendAuthentication(serverIPAddr, confirmPort, "esp8266_002");


          Serial.println("[ESP] 🔁 傳送確認訊息到 RPi");

          recvServerIP = true;

          targetUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/data";
          recvUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/last_data";


          delay(1000);  // 可選擇結束 loop 或進入主流程
        } else {
          Serial.println("[ESP] ❌ HMAC 驗證失敗，丟棄封包");
        }
      }
    }

  } else {
    if (!is_set) {
      targetUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/data";
      recvUrl = "https://" + serverIP + ":" + String(serverPort) + "/api/last_data";
      Serial.println(targetUrl);
      Serial.println(recvUrl);
      is_set = true;
    }
    int sensorVal = analogRead(A0);
    Serial.println("humidity: " + String(sensorVal));
    if (Serial.available() > 0) {
      recv = Serial.read();
      if (recv == '5') {
        sendHumiHttpData(deviceID, targetUrl, sensorVal);
      } else if (recv == '6') {
        showHttpData(recvUrl);
      }
    }
    yield();
    delay(500);
  }
}
