# /// script
# requires-python = ">=3.9"
# dependencies = ["pyserial"]
# ///
"""Class-set provisioning assembly line.

Plug in one XIAO board, run `uv run scripts/provision.py`, write the letter it
tells you on the board, unplug, next board. The script flashes the firmware,
assigns the next free letter (A, B, C...), verifies the board announces its
full name, and records everything in scripts/board_registry.json so the same
chip is never given two letters.
"""
import glob
import json
import re
import subprocess
import sys
import time
from datetime import date
from pathlib import Path

import serial

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "scripts" / "board_registry.json"


def fail(msg):
    print(f"\nERROR: {msg}")
    sys.exit(1)


def find_port():
    ports = glob.glob("/dev/cu.usbmodem*")
    if not ports:
        fail("no board found on USB (looked for /dev/cu.usbmodem*)")
    if len(ports) > 1:
        fail(f"more than one board plugged in ({', '.join(ports)}); provision one at a time")
    return ports[0]


def read_mac_hex(port):
    # Same suffix the firmware uses: last two bytes of the base MAC, printed big-endian.
    out = subprocess.run(
        ["pio", "pkg", "exec", "--", "esptool.py", "--port", port, "read_mac"],
        cwd=ROOT, capture_output=True, text=True, timeout=120,
    )
    match = re.search(r"MAC:\s*((?:[0-9a-f]{2}:){5}[0-9a-f]{2})", out.stdout, re.I)
    if not match:
        fail(f"could not read the chip MAC:\n{out.stdout}\n{out.stderr}")
    parts = match.group(1).split(":")
    return (parts[4] + parts[5]).upper()


def main():
    port = find_port()
    print(f"Board on {port}")
    mac_hex = read_mac_hex(port)
    print(f"Chip hex: {mac_hex}")

    registry = json.loads(REGISTRY.read_text()) if REGISTRY.exists() else []
    known = {entry["hex"]: entry for entry in registry}
    if mac_hex in known:
        entry = known[mac_hex]
        print(f'\nAlready provisioned on {entry["date"]}: this is board "{entry["letter"]}" (Pebble-{mac_hex}-{entry["letter"]}).')
        print("Nothing to do. If its tape is missing, just re-write the letter.")
        return

    used = {entry["letter"] for entry in registry}
    letter = next((c for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ" if c not in used), None)
    if letter is None:
        fail("all 26 letters used")

    print(f"Flashing firmware (this takes ~30 s)...")
    flash = subprocess.run(["pio", "run", "-t", "upload", "--upload-port", port],
                           cwd=ROOT, capture_output=True, text=True, timeout=600)
    if flash.returncode != 0:
        fail(f"flash failed:\n{flash.stdout[-2000:]}")

    print(f"Setting letter {letter} over serial...")
    time.sleep(2.5)  # let the board boot
    expected = f"Pebble-{mac_hex}-{letter}"
    with serial.Serial(port, 115200, timeout=0.3) as s:
        s.reset_input_buffer()
        s.write(f"letter {letter}\n".encode())
        deadline = time.time() + 15
        banner = b""
        while time.time() < deadline and expected.encode() not in banner:
            banner += s.read(4096)
    if expected.encode() not in banner:
        fail(f"board did not announce {expected} after reboot; check serial output and retry")

    registry.append({"hex": mac_hex, "letter": letter, "date": date.today().isoformat()})
    REGISTRY.write_text(json.dumps(registry, indent=2) + "\n")

    print(f"\nVerified: network is {expected}")
    print("=" * 46)
    print(f'||   WRITE THE LETTER  "{letter}"  ON THIS BOARD   ||')
    print("=" * 46)
    print(f"Registry now has {len(registry)} board(s). Unplug and insert the next one.")


if __name__ == "__main__":
    main()
