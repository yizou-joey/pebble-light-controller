# ESP32 programming activity

## Goals

Program a WS2812B LED strand using an ESP32 and FastLED. Explore RGB color, pixel indexes, conditions, loops, variables, and elapsed time, then build a moving-light **Chase** pattern.

Work with a partner. Predict what each example will show, run it, and explain what you observe before changing it. Take turns at the keyboard: one person edits while the other observes and discusses, then swap. Everyone should have time to think and type. Ask for help whenever a step is unclear.

## Overview and preparation

Your setup has a computer, a USB-C data cable, a Seeed XIAO ESP32-C3, and a WS2812B strand. The computer compiles your Arduino C++ program and uploads it over USB. The ESP32 then runs that program and sends color data to the strand.

Disconnect power before wiring. Use the supplied workshop hardware and check the strand's labels and input direction:

| XIAO ESP32-C3 | LED strand |
| --- | --- |
| D10 (GPIO10) | DIN / data input |
| 5V | 5V |
| GND | GND |

Open the `programming-workshop` folder in PlatformIO. PlatformIO, the configured ESP32 toolchain, and FastLED must already be installed on the workshop computer; initial installation may need internet access. The configured versions are `espressif32 7.1.3` and `FastLED 3.10.3`. Connect the board over USB.

Each example uses `NUM_LEDS` for the actual number of pixels on your strand, initially **60**. `BRIGHTNESS` starts at **20**, and FastLED is configured with a **5 V / 900 mA estimated power budget**. Keep those brightness and power settings during the workshop. The power setting limits estimated LED output; it is not a measurement of the USB supply. A different strand length requires changing `NUM_LEDS` before uploading.

### How to run every exercise

1. Open the linked example file in `examples/` and copy **all** its contents.
2. Open `src/main.cpp`, select everything, and replace it with the copied program. Save any earlier edits separately if you want to keep them.
3. Check `NUM_LEDS` against your strand, then save `src/main.cpp`.
4. Use PlatformIO **Build**, then **Upload**. Alternatively, in a terminal opened in the `programming-workshop` folder, run `pio run` followed by `pio run -t upload`.
5. Observe the strand. Edit the requested values or logic in `src/main.cpp`, save, and upload again after each change.

Only `src/main.cpp` is the student entry point. Merely opening or changing a file in `examples/` does not select it for upload. Each upload replaces the program on the board; code changes take effect after compiling and uploading. If the build fails, read the first compiler error and check punctuation and names. If upload fails, check that you opened the correct project and connected the board with a data cable. If upload succeeds but LEDs remain dark, disconnect power and check DIN, common ground, and `NUM_LEDS` with your instructor.

### The programming model

Skim this now and revisit it after trying the examples.

- `setup()` runs once when the board starts or resets. It configures FastLED, brightness, and the power budget. Static examples also draw their pattern here.
- `loop()` runs repeatedly. It can remain empty for a static image, or check time and update pixels for animation.
- `CRGB leds[NUM_LEDS]` stores one RGB color per pixel **in the ESP32's memory**. Changing `leds[index]` changes that buffer.
- `FastLED.show()` sends the buffer to the physical strand. A buffer edit alone does not change the lights.
- A `for` loop explicitly visits pixels from index `0` while `index < NUM_LEDS`. `NUM_LEDS` itself is outside the array.

The original Pixelblaze activity used `render(index)` for each pixel and `beforeRender(delta)` for frame updates. These examples use Arduino C++: write your own pixel loops, update variables in `loop()`, and call `FastLED.show()` after preparing the colors. Pixelblaze's `export` syntax and its `rgb()` and `time()` functions are not used here. FastLED RGB channel values are **integers from 0 to 255**.

## Getting started

### Light one LED

Replace all of `src/main.cpp` with [01_single_led.cpp](examples/01_single_led.cpp), then build and upload. The initial project already contains this example.

Before uploading, predict which LED will light and its color. Find the line that changes `leds[0]`, and find the line that sends that color to the strip. What would happen if you changed a color in the buffer but never sent it?

### Light a red block

Replace all of `src/main.cpp` with [02_red_block.cpp](examples/02_red_block.cpp), then build and upload. Find `START_INDEX`, `BLOCK_LENGTH`, and the loop that fills the block. Describe the first and last indexes that receive a color.

Take turns making these changes. Predict the result each time, then upload and compare:

- Make the red block shorter. Make it longer.
- Make the block green instead of red.
- Make the block purple.
- Move the block to the other end of the strand.

Keep your chosen block within the strand. What happens to the visible block if it extends beyond the last pixel? Explain the example's boundary check.

## Set all the pixels to a single color

Replace all of `src/main.cpp` with [03_single_color.cpp](examples/03_single_color.cpp), then build and upload.

Find `CRGB(255, 0, 0)`. Its arguments are red, green, and blue, each an integer from **0 to 255**. Change only this color expression and upload after each experiment:

| Try this expression | Observe |
| --- | --- |
| `CRGB(0, 255, 0)` | Which channel is active? |
| `CRGB(0, 0, 255)` | Which channel is active? |
| `CRGB(0, 0, 0)` | What does zero mean? |
| `CRGB(255, 255, 255)` | What does mixing all three channels produce? |
| `CRGB(77, 102, 128)` | Compare the channel proportions. |
| `CRGB(3, 77, 3)` | Which channel dominates? |
| `CRGB(54, 227, 235)` | Predict the mixture before uploading. |

Now make your own color. Can you make orange? Super challenging: can you make brown? Compare the light with surrounding objects and discuss how brightness and the viewing environment affect the perceived color. Swap who is typing between challenges.

## Lighting a single pixel

Replace all of `src/main.cpp` with [04_single_pixel.cpp](examples/04_single_pixel.cpp), then build and upload. Find `PIXEL_INDEX` and the condition that compares it with the current loop index. The loop only visits valid indexes, so a selection outside the strand lights nothing.

The example selects index `7`. Counting from the beginning of the strip, which LED is lit, and why? An index starts at zero, so index `7` is the **eighth** LED; everyday phrases such as “the seventh LED” count from one. Keep these two counting systems distinct.

Try these:

- Display blue instead of red.
- Light the last pixel. Bonus: make your code work for different values of `NUM_LEDS` without rewriting the selected index.
- Light both the seventh and the twelfth LED red.
- Light the seventh LED red and the twelfth LED blue.

The last two challenges need a strand with at least 12 pixels. Keep every selected index within the array and prepare all desired pixel colors before calling `FastLED.show()`.

## Overlapping blocks

Replace all of `src/main.cpp` with [05_overlapping_blocks.cpp](examples/05_overlapping_blocks.cpp), then build and upload.

For every pixel, the example starts with black, sets the red channel when `index < 5`, sets the green channel when `index > 3`, and gives the blue channel a spatial gradient. Each channel is assigned independently: a green assignment does not erase an earlier red assignment. Compare this with replacing a pixel's entire `CRGB` color.

Take turns experimenting. In C++, put `//` at the beginning of a statement's line to comment it out. Disable the red assignment, green assignment, and blue assignment **one at a time**, restoring the previous statement before the next experiment. If a condition uses braces, comment out its assignment inside the braces and leave the braces intact.

- How does each change affect the display? Explain which channels remain active.
- With only the blue-gradient assignment disabled, the active channels are **red and green**. How many visible colors do you expect? Identify red, green, and yellow on the default strand and explain why yellow appears.
- Make the red area longer or shorter.
- Make the green area longer or shorter.
- Change the amount of overlap between the red and green areas.

The gradient uses `255.0f * index / NUM_LEDS` to scale each pixel's position to an RGB channel value. The `255.0f` makes this floating-point arithmetic before conversion to an integer. Why is floating-point division useful here? Why does that fraction approach, but not reach, `1.0` at the last pixel? No change is needed just because the final pixel does not reach full blue.

## Using a variable: Chase

Replace all of `src/main.cpp` with [06_chase.cpp](examples/06_chase.cpp), then build and upload. Watch the light reach the end and return to the beginning.

`position` remembers which pixel is lit. `STEP_INTERVAL_MS` starts at **100 milliseconds** between movements. `millis()` reports elapsed milliseconds since the board started; `previousStepMs` remembers the preceding update time. The unsigned difference between those times determines when to move, without pausing the program with `delay()`.

Trace one update: wait until enough time has elapsed, advance and wrap `position`, clear the previous frame, set the selected pixel, and send the new frame with `FastLED.show()`. The setup code displays the initial position before updates begin.

Discuss with your partner:

- Where is `position` initialized? What is its initial value?
- Where is it updated or modified?
- Where is it read to decide which pixel to light?
- Which expression brings the position back to zero after the last pixel?
- Why must the old frame be cleared for a single moving point?

Try making the light move twice as fast. Challenge: make it move half as fast. Change the interval, predict its effect, then upload. Keep the interval greater than zero. Switch partners before the second modification.

## Optional extra animation challenges

### Animation model: a frame is a buffer of colors

A frame contains the color of every LED. The program can compute a color from time, pixel position, or both, then send the whole buffer. Unlike Pixelblaze, FastLED does not automatically call a render function for each index: the loops in your program do that work.

In the following examples, `loop()` redraws the buffer repeatedly, while `millis()` determines each color's phase. The color period specifies how long a complete ramp takes; it does not count loop iterations. The time phases restart when the millisecond counter eventually wraps, while Chase uses unsigned time differences for its step timing.

### Change color with time

Replace all of `src/main.cpp` with [07_time_color.cpp](examples/07_time_color.cpp), then build and upload.

Read the expressions for `redPhase` and `greenPhase`. For a period of 1,000 ms, taking elapsed time modulo 1,000 and dividing by `1000.0f` produces a fraction from `0.0` toward `1.0`, then resets it to zero. Multiplying a phase by 255 and converting to an integer produces a channel value.

The example uses a **1,000 ms red period** and a **2,000 ms green period**, with blue at zero. All pixels share the same values at a given moment.

- Temporarily set green to zero. Describe the red animation.
- Restore green. When do the two channels reset together?
- Make the red cycle take ten seconds. What needs to change in the phase calculation?
- Add your own blue cycle.

These channels **ramp upward and then reset abruptly**. This is not a smooth fade up and down or a breathing animation. What would you need to change to make a channel fade downward as well?

### Combine time and position

Replace all of `src/main.cpp` with [08_time_and_position.cpp](examples/08_time_and_position.cpp), then build and upload.

Red varies with time and is the same across the strand. Green uses `spatialPhase`, calculated from the pixel index divided by `float(NUM_LEDS)`, and is different along the strand. Blue stays zero.

- Which differences persist from one frame to the next?
- Set the time-dependent red contribution to zero to study the static green gradient, then restore it.
- Swap which channel uses time and which uses position.
- Change the time period and explain what changes along the strand and what stays fixed.

Finally, replace all of `src/main.cpp` with [09_combined_animation.cpp](examples/09_combined_animation.cpp), then build and upload.

Each RGB channel now uses the absolute difference between its own time phase and the spatial phase. The red, green, and blue periods are **1,000**, **2,000**, and **3,000 ms** respectively. Each pixel can have a different color, and its color can change over time.

Predict where a channel will become darkest as its time phase moves along the strand. Change one period at a time and describe the result. Work with your partner to make a variation you can explain in terms of time, position, and color.

## Source and adaptation

Adapted from [Spring 26 Pixelblaze Programming Activity](https://etextiles-sp22.notion.site/Spring-26-Pixelblaze-Programming-Activity-31257bee071b813b9578f4e24ed60284), using the local course clipping as the source for the activity sequence and questions. This version implements the code exercises in Arduino C++ with FastLED; the original Pixelblaze clipping remains unchanged.
