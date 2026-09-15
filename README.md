# Pebble Light Controller

Firmware for a Seeed XIAO ESP32-C3 that drives a strand of Pebble LEDs and serves its own
Wi-Fi network with a phone-friendly color wheel. No internet, no app install: join the
board's network, open a page, drag the wheel.

Written by Joey Zou (yizou-joey) for the E-Textiles course (CMAA 5036, HKUST-GZ),
with additions by Margaret Minsky.

## Hardware

- Seeed XIAO ESP32-C3. **The small U.FL antenna must be clipped on** or the Wi-Fi network
  will not appear.
- Pebble LED strand (WS2812B), data line on pin **D10** (GPIO10).
- LED power from the XIAO's **5V** pin, ground to **GND**.

## Flash it

Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI), plug the board
in over USB-C, then from this folder:

```
pio run -t upload
```

## Use it

1. Power the board. It creates an open Wi-Fi network named **Pebble-XXXX**;
   the four characters are unique to each board (from its chip ID), so ten boards
   in one room stay distinguishable. No password.
2. Join that network with a phone or laptop.
3. Open **http://192.168.4.1** in a browser. The page header shows the board's name,
   so you can confirm you are on *your* board.
4. Drag the color wheel; all LEDs follow. The four buttons are shortcuts.
   全部熄灭 turns everything off.

### Set your LED count

Strand lengths differ. Enter the number of LEDs on *your* strand in the
灯珠数量 (LED count) field, 1-100. The board remembers it across power cycles.

### Brightness and power

The brightness slider goes to 255, but the firmware holds total LED current inside a
budget (`LED_MILLIAMP_BUDGET`, 900 mA) because the strand is powered from USB through
the board. At high brightness on white you will see the output stop getting brighter:
that is the power governor doing its job, not a bug. If every board runs from a 2 A
wall charger, the budget in `src/main.cpp` can be raised.

## Provisioning the class set (one-time per board)

Each class board gets a human letter appended to its network name, e.g.
**Pebble-A354-A**: the hex part comes from the chip (always unique), the letter
is what students look for and what's written on the board's tape. To set up the
boards, with [uv](https://docs.astral.sh/uv/) and PlatformIO installed:

1. Plug in **one** board.
2. Run `uv run scripts/provision.py`. It flashes the firmware, assigns the next
   free letter, verifies the board announces its full name, and records the
   board in `scripts/board_registry.json`.
3. Write the letter it shows on tape on the board (and the header and battery,
   so kits stay together). Unplug, insert the next board.

Running it on an already-provisioned board just reports its existing letter, so
it never hands out duplicates. The letter lives in the board's flash settings
and survives future reflashes. Commit `board_registry.json` after a provisioning
session; it is the inventory of the class set.

## Things to try changing first

All the firmware is one file, `src/main.cpp`.

- The four quick colors (search for `'#ff0000'` in the page section).
- The default color at power-on (`CRGB color = CRGB::Red;`).
- `examples/led_test_60.cpp` is a standalone strand test (moving dot, then blinks)
  useful when wiring a new strand.

## Layout

- `src/main.cpp` — everything: LED control, Wi-Fi access point, the web page, the API.
- `src/iro_asset.h` — the color-wheel library (iro.js) compressed for serving from flash;
  regenerate with `scripts/embed_iro.py` after updating `vendor/iro/iro.min.js`.
- `vendor/iro/` — iro.js 5.5.2 and its MPL-2.0 license.
