import jwt
import time

JWT_SECRET = "your_jwt_secret_key"  # 建議與 SECRET_KEY 不同
JWT_ALGORITHM = "HS256"

def generate_jwt(device_id):
    payload = {
        "device_id": device_id,
        "iat": int(time.time()),
        "exp": int(time.time()) + 3600  # 過期時間：1小時
    }
    token = jwt.encode(payload, JWT_SECRET, algorithm=JWT_ALGORITHM)
    return token
