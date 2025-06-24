# ESP8266 Data Sender and Receiver

本專案是基於 ESP8266 的裝置程式，負責：

1. 透過 UDP 廣播尋找並驗證區域網路中的 Flask Server。
2. 與驗證成功的 Server 進行 HTTPS 資料傳送與接收。
3. 透過 Serial2 傳送感測資料給其他模組。

## 硬體需求

* ESP8266 or ESP32......
* 感測器接至 A0 腳位（類比輸入）
* 使用 GPIO16 與 GPIO17 作為 Serial2 通訊腳位

## 功能說明

### 1. WiFi 連線

裝置啟動後會連接到指定的 WiFi 網路。

```cpp
WiFi.begin(ssid, password);
```

### 2. Server IP 驗證與切換

變數 `recvServerIP` 控制裝置是否需進行伺服器驗證：

* `false`：自動尋找 Server IP（適用於同一內網但不知 IP 的情況）
* `true`：使用預先定義好的 `serverIP`，略過驗證流程

### 3. 自動尋找 Server IP（UDP 廣播 + HMAC 驗證）

裝置透過 UDP 接收格式為 `FLASK_SERVER_IP:<ip>|<hmac_hash>` 的封包。
收到後使用預設密鑰 `secret_key` 計算 HMAC，若比對成功則儲存 Server IP。

```cpp
String calculated_hmac = generateHMAC(message, secret_key);
```

### 4. 傳送確認訊息（UDP）

驗證 Server IP 後會透過另一個 UDP port 傳送確認訊息（包含 deviceID）給 Server。

```cpp
sendAuthentication(serverIPAddr, confirmPort, deviceID);
```

### 5. 傳送與接收資料（HTTPS）

* 定時從 A0 讀取感測值，透過 HTTPS 傳送至 Flask Server 的 `/api/data`
* 從 `/api/last_data` 接收回傳的濕度數值

```cpp
sendHumiHttpData(deviceID, targetUrl, sensorVal);
int recvHumi = getHumidityFromServer(recvUrl);
```

### 6. Serial2 通訊

將接收到的濕度值透過 Serial2 以格式 `<H:xx>` 傳送給其他裝置。

```cpp
Serial2.println("<H:" + String(recvHumi) + ">");
```

## Sender / Receiver 模式說明

* **Sender**：讀取類比感測值並上傳至 Server
* **Receiver**：從 Server 端讀取資料並透過 Serial2 發送至其他裝置

## 參數設定

| 參數             | 說明                                    |
| -------------- | ------------------------------------- |
| `ssid`         | WiFi SSID 名稱                          |
| `password`     | WiFi 密碼                               |
| `deviceID`     | 裝置 ID，用於辨識身份                          |
| `serverPort`   | Flask Server 的 HTTPS Port             |
| `udpPort`      | Server 廣播使用的 Port                     |
| `confirmPort`  | ESP 傳送確認訊息的 Port                      |
| `recvServerIP` | 是否使用預設 IP（`true` 為直接連線，`false` 為廣播尋找） |

## 注意事項

* 此程式需搭配具備 HMAC 驗證與 HTTPS 接口的 Flask Server 使用。

---

作者：@hding4915
