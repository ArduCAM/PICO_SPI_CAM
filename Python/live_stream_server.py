import argparse
import logging
import threading
import time
from typing import Optional

import serial
from flask import Flask, Response, make_response

DEFAULT_COMMAND = b"\x10"  # single capture
DEFAULT_BAUD = 115200 * 4

HTML_PAGE = """<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8" />
    <title>Arducam Live Stream</title>
    <style>
        body { font-family: system-ui, sans-serif; text-align: center; background: #0b1a2b; color: #f0f4ff; }
        img { max-width: 90vw; border: 4px solid #2cb1ff; border-radius: 8px; box-shadow: 0 0 18px rgba(44,177,255,.45); }
        .wrapper { margin-top: 4vh; }
    </style>
</head>
<body>
    <div class="wrapper">
        <h1>Arducam Live Stream</h1>
        <img src="/stream.mjpg" alt="Live stream" />
        <p>Refresh page if stream stops.</p>
    </div>
</body>
</html>
"""


def extract_jpeg(blob: bytearray) -> Optional[bytes]:
    """Return the first complete JPEG (if any) found inside blob."""
    start = blob.find(b"\xff\xd8")
    end = blob.find(b"\xff\xd9", start + 2)
    if start != -1 and end != -1:
        jpeg = bytes(blob[start : end + 2])
        del blob[: end + 2]  # drop consumed bytes so buffer does not balloon
        return jpeg
    return None


class ArducamSerial:
    """Handle serial capture requests in a threadsafe way."""

    def __init__(self, port: str, baud: int = DEFAULT_BAUD, timeout: float = 2.0):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self._lock = threading.Lock()
        self._serial = serial.Serial(port, baud, timeout=timeout)
        logging.info("Opened %s at %d baud", port, baud)

    def capture(self, timeout: float = 5.0) -> bytes:
        """Trigger a capture and return the JPEG bytes."""
        with self._lock:
            self._serial.reset_input_buffer()
            self._serial.write(DEFAULT_COMMAND)
            logging.debug("Capture command sent")
            buffer = bytearray()
            deadline = time.time() + timeout
            while time.time() < deadline:
                chunk = self._serial.read(2048)
                if chunk:
                    buffer.extend(chunk)
                    frame = extract_jpeg(buffer)
                    if frame:
                        logging.debug("Frame captured (%d bytes)", len(frame))
                        return frame
            raise TimeoutError("Timed out waiting for JPEG data from camera.")


def create_app(camera: ArducamSerial) -> Flask:
    app = Flask(__name__)

    @app.get("/")
    def index():
        return make_response(HTML_PAGE, 200, {"Content-Type": "text/html"})

    @app.get("/stream.mjpg")
    def stream():
        def frames():
            while True:
                try:
                    jpeg = camera.capture()
                    yield (
                        b"--frame\r\n"
                        b"Content-Type: image/jpeg\r\n"
                        + f"Content-Length: {len(jpeg)}\r\n\r\n".encode()
                        + jpeg
                        + b"\r\n"
                    )
                except TimeoutError as exc:
                    logging.warning("%s", exc)
                    time.sleep(0.5)
                except serial.SerialException as exc:
                    logging.error("Serial error: %s", exc)
                    break

        return Response(frames(), mimetype="multipart/x-mixed-replace; boundary=frame")

    return app


def parse_args():
    parser = argparse.ArgumentParser(description="Serve Arducam captures over MJPEG.")
    parser.add_argument("--serial-port", "-p", default="COM3", help="Serial port connected to Pico.")
    parser.add_argument("--baud", "-b", type=int, default=DEFAULT_BAUD, help="Serial baud rate.")
    parser.add_argument("--listen", default="0.0.0.0", help="IP/interface to bind HTTP server.")
    parser.add_argument("--port", type=int, default=8080, help="HTTP port.")
    parser.add_argument("--log-level", default="INFO", help="Logging level.")
    return parser.parse_args()


def main():
    args = parse_args()
    logging.basicConfig(
        level=getattr(logging, args.log_level.upper(), logging.INFO),
        format="%(asctime)s %(levelname)s: %(message)s",
    )
    camera = ArducamSerial(args.serial_port, args.baud)
    app = create_app(camera)
    logging.info("Serving MJPEG on http://%s:%d/", args.listen, args.port)
    app.run(host=args.listen, port=args.port, threaded=True)


if __name__ == "__main__":
    main()
