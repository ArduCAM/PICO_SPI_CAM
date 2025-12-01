"""Listen to Pico USB serial and dump every CAMERA_USB JPEG to disk."""

import os
import pathlib
import serial
import time
from datetime import datetime

PORT = "COM3"  # Update if needed
BAUD = 115200 * 4
OUTPUT_DIR = pathlib.Path(r"C:\Misc\SIT")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

HEADER_PREFIX = b"[CAMERA_USB] BEGIN_HEX"
FOOTER_PREFIX = b"[CAMERA_USB] END"


def read_exact(ser: serial.Serial, size: int, timeout: float = 5.0) -> bytes:
    """Read an exact number of bytes or return empty on timeout."""
    deadline = time.time() + timeout
    chunks = bytearray()
    while len(chunks) < size and time.time() < deadline:
        chunk = ser.read(size - len(chunks))
        if chunk:
            chunks.extend(chunk)
    return bytes(chunks)


def save_jpeg(payload: bytes) -> pathlib.Path:
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = OUTPUT_DIR / f"pico_capture_{timestamp}.jpg"
    with open(filename, "wb") as fp:
        fp.write(payload)
    return filename


def main() -> None:
    with serial.Serial(PORT, BAUD, timeout=1) as ser:
        print(f"Listening on {PORT} @ {BAUD} baud for CAMERA_USB JPEG frames...")
        while True:
            line = ser.readline()
            if not line:
                continue

            stripped = line.strip()
            if not stripped:
                continue

            if not stripped.startswith(HEADER_PREFIX):
                try:
                    print(stripped.decode("utf-8", errors="ignore"))
                except UnicodeDecodeError:
                    pass
                continue

            try:
                size = int(stripped.split()[-1])
            except (ValueError, IndexError):
                print(f"[HOST] Invalid header: {line!r}")
                continue

            payload = bytearray()
            while True:
                line2 = ser.readline()
                if not line2:
                    continue

                stripped2 = line2.strip()
                if not stripped2:
                    continue

                if stripped2.startswith(FOOTER_PREFIX):
                    break

                try:
                    payload.extend(bytes.fromhex(stripped2.decode("ascii")))
                except ValueError:
                    print(f"[HOST] Invalid HEX chunk: {stripped2!r}")
                    payload.clear()
                    break

            if len(payload) != size:
                print(f"[HOST] Size mismatch (expected {size}, got {len(payload)})")
                continue

            output_path = save_jpeg(payload)
            print(f"[HOST] Saved {output_path}")
            print("[HOST] Transfer complete\n")


if __name__ == "__main__":
    main()
