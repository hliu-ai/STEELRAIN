import socket
import json
import struct
import time

class UE5SocketClient:
    def __init__(self, host='127.0.0.1', port=7777, timeout=5.0, retry_delay=1.0):
        """Keep retrying until the UE5 server is listening."""
        self.sock = None
        while self.sock is None:
            try:
                print(f"[UE5SocketClient] Attempting to connect to {host}:{port}...")
                self.sock = socket.create_connection((host, port), timeout)
            except Exception as e:
                print(f"[UE5SocketClient] Connection failed ({e}), retrying in {retry_delay}s")
                time.sleep(retry_delay)
        self.sock.settimeout(None)
        print("[UE5SocketClient] TCP connection acquired. Initializing RL networks...")

    def _recv_n_bytes(self, n):
        buf = b''
        while len(buf) < n:
            chunk = self.sock.recv(n - len(buf))
            if not chunk:
                raise ConnectionError("Socket closed")
            buf += chunk
        return buf

    def _send(self, msg: dict) -> dict:
        # send the request
        data = json.dumps(msg).encode('utf-8')
        self.sock.sendall(struct.pack('<I', len(data)) + data)

        # read response length + payload
        raw_len = self._recv_n_bytes(4)
        resp_len = struct.unpack('<I', raw_len)[0]
        resp_bytes = self._recv_n_bytes(resp_len)
        resp = json.loads(resp_bytes.decode('utf-8'))

        return resp

    def reset(self):
        r = self._send({"cmd": "reset"})
        return (
            r.get("obs"),
            r.get("reward"),
            r.get("done"),
            r.get("delta_time", 0.0)
        )

    def step(self, pitch, yaw, fire_flag):
        r = self._send({"cmd": "step", "action": [pitch, yaw, fire_flag]})
        return (
            r.get("obs"),
            r.get("reward"),
            r.get("done"),
            r.get("delta_time", 0.0)
        )

    def pause(self):
        """Pause the UE5 simulation."""
        r = self._send({"cmd": "pause"})
        return r.get("status") == "paused"

    def resume(self):
        """Resume the UE5 simulation."""
        r = self._send({"cmd": "resume"})
        return r.get("status") == "resumed"

    def close(self):
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except:
            pass
        self.sock.close()
