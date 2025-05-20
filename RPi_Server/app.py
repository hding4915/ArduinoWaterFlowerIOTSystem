import threading
from flask import Flask, request, jsonify, render_template_string
from ip_broadcast import *

app = Flask(__name__)
latest_data = {}

@app.route('/')
def index():
    html = '''
    <meta http-equiv="refresh" content="5">
    <h1>Hello, World!</h1>
    <h2>Latest POST Data:</h2>
    <pre>{{ data }}</pre>
    '''
    return render_template_string(html, data=latest_data)

@app.route('/api/data', methods=['POST'])
def receive_data():
    global latest_data
    latest_data = request.get_json()
    print("Received data:", latest_data)
    return jsonify({"status": "OK", "received": latest_data})

@app.route('/api/last_data', methods=['GET'])
def get_last_data():
    return jsonify(latest_data)


if __name__ == "__main__":
    # 開始廣播
    broadcast_thread = threading.Thread(target=broadcast_ip_loop)
    broadcast_thread.daemon = True
    broadcast_thread.start()

    udp_thread = threading.Thread(target=udp_listen_for_confirmation)
    udp_thread.daemon = True
    udp_thread.start()

    esp1_confirmed_event.wait()
    esp2_confirmed_event.wait()

    app.run(host='0.0.0.0', port=8000, debug=False)

