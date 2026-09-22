# ESP32 programming workshop

Code-based LED activities for a Seeed XIAO ESP32-C3 and a WS2812B strand, using Arduino C++ and FastLED.

This README covers project setup and how to run the code. Follow [ACTIVITY.md](ACTIVITY.md) for the complete classroom sequence, explanations, questions, and challenges.

## Quick start

1. With power disconnected, connect strand **DIN → D10 (GPIO10)**, **5V → 5V**, and **GND → GND**. Check the strand's input direction and use the supplied workshop hardware.
2. Open **this folder** (`programming-workshop`) as the PlatformIO project in VS Code. Install PlatformIO and the configured toolchain/dependencies beforehand; initial installation may require internet access.
3. Connect the board with a USB-C data cable. In [src/main.cpp](src/main.cpp), set `NUM_LEDS` to your strand's actual count (default **60**). Keep brightness at **20** and the FastLED estimated power budget at **5 V / 900 mA**.
4. Use PlatformIO **Build**, then **Upload**, or run these commands from this folder:

   ```sh
   pio run
   pio run -t upload
   ```

5. Begin with [Getting started in the activity guide](ACTIVITY.md#getting-started).

## Switching examples

The starting [src/main.cpp](src/main.cpp) contains the [single-LED example](examples/01_single_led.cpp). For each activity, copy the entire linked example into `src/main.cpp`, replacing its contents. Recheck `NUM_LEDS`, save, then build and upload.

Files in [examples/](examples/) are complete reference programs and are not built automatically. Save your previous edits separately if you want to keep them.

## File reference

- [ACTIVITY.md](ACTIVITY.md): the classroom guide; activity instructions are maintained here.
- [src/main.cpp](src/main.cpp): the student entry point to edit and upload.
- [examples/](examples/): complete starting programs for each activity.
- [platformio.ini](platformio.ini): board and dependency configuration, using Arduino, `espressif32 7.1.3`, and `FastLED 3.10.3`.

For the original Wi-Fi controller, open the separate [pebble-controller project](../pebble-controller/README.md).
