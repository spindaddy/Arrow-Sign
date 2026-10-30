#!/usr/bin/env python3
"""Serve the live sign dashboard and stream Nano status lines."""

import glob
import json
import queue
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import serial

ROOT = Path(__file__).resolve().parent
BAUD = 115200
HTTP_PORT = 8765

state_lock = threading.Lock()
clients = []
commands = queue.Queue()
state = {
    "connected": False,
    "error": None,
    "pat": None,
    "ms": 0,
    "d": [0, 0, 0, 0],
    "bri": 100,
    "sign": 0,
    "port": None,
    "raw": "",
}


def find_port():
    preferred = "/dev/cu.usbserial-1110"
    ports = sorted(glob.glob("/dev/cu.usbserial*"))
    if preferred in ports:
        return preferred
    return ports[0] if ports else None


def snapshot():
    with state_lock:
        return dict(state)


def publish():
    payload = ("data: " + json.dumps(snapshot()) + "\n\n").encode()
    with state_lock:
        dead = []
        for client in clients:
            try:
                client.put_nowait(payload)
            except queue.Full:
                try:
                    client.get_nowait()
                    client.put_nowait(payload)
                except queue.Empty:
                    dead.append(client)
        for client in dead:
            if client in clients:
                clients.remove(client)


def set_fields(**kwargs):
    with state_lock:
        state.update(kwargs)
    publish()


def handle_line(line):
    if not line.startswith("AS "):
        return
    fields = {}
    for part in line[3:].split():
        if "=" not in part:
            continue
        key, value = part.split("=", 1)
        fields[key] = value
    try:
        pat = int(fields["pat"])
        ms = int(fields["ms"])
        dips = [int(fields["d1"]), int(fields["d2"]), int(fields["d3"]), int(fields["d4"])]
        bri = int(fields["bri"])
        sign = int(fields["sign"])
    except (KeyError, ValueError):
        return
    set_fields(pat=pat, ms=ms, d=dips, bri=bri, sign=sign, raw=line, error=None)


def serial_loop():
    while True:
        port = find_port()
        if not port:
            set_fields(connected=False, port=None, pat=None, error="No USB serial port")
            time.sleep(1)
            continue
        try:
            with serial.Serial(port, BAUD, timeout=0.05) as ser:
                set_fields(connected=True, port=port, error=None, pat=None, raw="")
                buf = b""
                while find_port() == port:
                    while True:
                        try:
                            ser.write(commands.get_nowait())
                        except queue.Empty:
                            break
                    chunk = ser.read(256)
                    if not chunk:
                        continue
                    buf += chunk
                    while b"\n" in buf:
                        raw, buf = buf.split(b"\n", 1)
                        handle_line(raw.decode("ascii", "ignore").strip())
        except Exception as exc:
            set_fields(connected=False, port=port, pat=None, error=str(exc))
            time.sleep(1)


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        return

    def do_GET(self):
        path = self.path.split("?", 1)[0]
        if path in ("/", "/dashboard.html"):
            body = (ROOT / "dashboard.html").read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/state":
            body = json.dumps(snapshot()).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/events":
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Connection", "keep-alive")
            self.end_headers()
            client = queue.Queue(maxsize=4)
            with state_lock:
                clients.append(client)
            try:
                self.wfile.write(("data: " + json.dumps(snapshot()) + "\n\n").encode())
                self.wfile.flush()
                while True:
                    self.wfile.write(client.get())
                    self.wfile.flush()
            except Exception:
                pass
            finally:
                with state_lock:
                    if client in clients:
                        clients.remove(client)
            return
        self.send_error(404)

    def do_POST(self):
        path = self.path.split("?", 1)[0]
        if path != "/dip":
            self.send_error(404)
            return
        length = int(self.headers.get("Content-Length", "0"))
        try:
            body = json.loads(self.rfile.read(length).decode("utf-8"))
            switch = int(body["n"])
            on = 1 if int(body["on"]) else 0
        except (KeyError, TypeError, ValueError, json.JSONDecodeError):
            self.send_error(400)
            return
        if switch < 1 or switch > 4:
            self.send_error(400)
            return
        commands.put(f"DIP{switch}={on}\n".encode())
        self.send_response(204)
        self.end_headers()


def main():
    threading.Thread(target=serial_loop, daemon=True).start()
    server = ThreadingHTTPServer(("127.0.0.1", HTTP_PORT), Handler)
    print(f"dashboard http://127.0.0.1:{HTTP_PORT}/", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
