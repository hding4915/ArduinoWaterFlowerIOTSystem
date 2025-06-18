import time
import hashlib
import hmac
import socket
import secrets
from jwt_token import *
from threading import Event


SECRET_KEY = b"my_secret_key"
esp1_confirmed = False
esp2_confirmed = False
esp1_confirmed_event = Event()
esp2_confirmed_event = Event()
valid_tokens = {}


def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

def generate_hmac(message):
    return hmac.new(SECRET_KEY, message.encode(), hashlib.sha256).hexdigest()

def verify_hmac(device_id, timestamp, received_hmac):
    message = f"{device_id}|{timestamp}"
    expected_hmac = hmac.new(SECRET_KEY, message.encode(), hashlib.sha256).hexdigest()
    return hmac.compare_digest(expected_hmac, received_hmac)

def udp_listen_for_confirmation():
    global esp1_confirmed, esp2_confirmed
    udp_port = 4211

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', udp_port))
    print(f"[RPi] 等待 ESP 認證（UDP port {udp_port}）...")

    while not esp1_confirmed or not esp2_confirmed:
        try:
            data, addr = sock.recvfrom(1024)
            message = data.decode('utf-8').strip()
            print(f"[RPi] 收到來自 {addr} 的訊息: {message}")

            # 預期格式：ESP_CONFIRM|device_id|timestamp|hmac
            parts = message.split('|')
            if len(parts) == 4 and parts[0] == "ESP_CONFIRM":
                device_id = parts[1]
                timestamp = parts[2]
                received_hmac = parts[3]

                if verify_hmac(device_id, timestamp, received_hmac):
                    token = generate_jwt(device_id)
                    response = f"JWT|{device_id}|{token}"
                    sock.sendto(response.encode(), addr)

                    if device_id == "esp8266_001":
                        esp1_confirmed = True
                        esp1_confirmed_event.set()
                    elif device_id == "esp8266_002":
                        esp2_confirmed = True
                        esp2_confirmed_event.set()
                    print(f"[RPi] ✅ ESP 裝置 {device_id} 已通過認證")
                else:
                    print(f"[RPi] ❌ HMAC 驗證失敗（裝置 {device_id}）")
            else:
                print("[RPi] ❌ 格式錯誤，忽略訊息")
        except Exception as e:
            print(f"[RPi] 接收失敗: {e}")
    print("[RPi] 停止監聽 Udp ✅")

def broadcast_ip_loop():
    ip = get_local_ip()
    message = f"FLASK_SERVER_IP:{ip}"
    signature = generate_hmac(message)
    full_message = f"{message}|{signature}".encode('utf-8')
    udp_port = 4210

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(0.2)

    while not esp1_confirmed or not esp2_confirmed:
        try:
            sock.sendto(full_message, ('<broadcast>', udp_port))
            print(f"[RPi] 廣播自己的 IP: {ip}")
        except Exception as e:
            print(f"廣播失敗: {e}")
        time.sleep(1)

    print("[RPi] 停止廣播 ✅")

def resetConfirmation():
    global esp1_confirmed, esp2_confirmed, esp1_confirmed_event, esp2_confirmed_event
    esp1_confirmed = False
    esp2_confirmed = False
    esp1_confirmed_event = Event()
    esp2_confirmed_event = Event()