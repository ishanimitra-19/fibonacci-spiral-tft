# Fibonacci Spiral Fractal for the TTGO T-Display

Generative art for the LilyGO TTGO T-Display (ESP32 + 1.14" ST7789, 135×240). Each cycle the sketch tiles a golden rectangle into Fibonacci squares, draws a smooth golden spiral through them, and randomly grows smaller spirals inside the squares so the piece becomes a self-similar fractal. Palette, orientation, line weight, recursion depth and animation speed are re-rolled every cycle, so it never repeats.

![Demo](docs/images/demo.gif)

Blog post with more background: _link coming soon_

## Design goals

- **Grow, don't loop.** The spiral is drawn square by square so you watch it form, rather than appearing as a finished frame.
- **Math you can see.** Every square is a true golden-ratio subdivision, and each quarter arc is centred on the corner that keeps the curve tangent-continuous, so the spiral is a real Fibonacci spiral and not an approximation.
- **Never the same twice.** Colour palette, mirror orientation, line width, fractal depth (1–3 levels), spawn probability and drawing speed are all drawn from `random()` each cycle. This satisfies the "more than a GIF" requirement for Module 1.
- **Clean lines on a tiny screen.** Arcs use TFT_eSPI's anti-aliased `drawWideLine`, which matters a lot at 135 px tall.

## How it works

1. `newCycle()` rolls the random parameters and clears the screen.
2. `spiral()` takes a rectangle and repeatedly splits off the largest square (left, top, right, bottom, in rotation), drawing one quarter arc per square. The remaining rectangle is itself golden, so the loop continues until squares are smaller than 4 px.
3. While drawing, each square has a `spawnChance` % chance of recursively calling `spiral()` on a smaller golden rectangle inside itself, with a hue shift of 120°. That recursion is what makes it fractal.
4. `loop()` holds the finished image, redraws it three times with a shifting hue for a shimmer effect, then starts a new cycle.

## Hardware

| Part | Notes |
|---|---|
| LilyGO TTGO T-Display | ESP32, ST7789 135×240 TFT, USB-C |
| USB-C cable | data-capable, for uploading |
| Optional: power bank / wall charger | the sketch runs standalone once flashed |

No wiring is needed; the display is built into the board.

## Replicate it

1. **Install the Arduino IDE** (2.x): https://www.arduino.cc/en/software
2. **Install the ESP32 boards.** In *Settings → Additional boards manager URLs* add
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, then in *Boards Manager* install **esp32 by Espressif**. Version 2.0.17 is what this was built with; 3.x also works.
3. **Install TFT_eSPI.** *Tools → Manage Libraries*, search `TFT_eSPI` by Bodmer, install.
4. **Point TFT_eSPI at the T-Display.** Open `Documents/Arduino/libraries/TFT_eSPI/User_Setup_Select.h`:
   - comment out `#include <User_Setup.h>`
   - uncomment `#include <User_Setups/Setup25_TTGO_T_Display.h>`
5. **USB driver.** Boards with a CH9102 chip need the WCH CH34x driver (see the [LilyGO repo](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)). Boards with a CP210x chip show up as `usbserial` with no extra driver on recent macOS.
6. **Open the sketch** `FibonacciSpiral/FibonacciSpiral.ino` and set:
   - *Tools → Board → esp32 → ESP32 Dev Module*
   - *Tools → Port* → your board's port
   - *Tools → Upload Speed → 115200*
   - *Tools → PSRAM → Disabled*, *Flash Size → 4MB*
7. **Upload.** The spiral starts as soon as the board resets, and runs from flash on any USB power source afterwards.

If you use `arduino-cli`, the included `sketch.yaml` already carries the board settings; edit `default_port` for your machine.

## Make it your own

All the knobs live in `newCycle()`:

| Variable | Effect |
|---|---|
| `baseHue`, `hueStep` | starting colour and how fast colour shifts per square |
| `maxDepth` | how many levels of nested spirals (1 = plain spiral) |
| `spawnChance` | % chance a square grows a child spiral |
| `lineW` | arc thickness in pixels |
| `showSquares` | outline the tiling squares |
| `stepDelay` | ms between squares, i.e. drawing speed |

Narrow or widen the `random()` ranges to bias the look, or fix a value to pin it.

## Repository layout

```
FibonacciSpiral/      Arduino sketch (folder name must match the .ino)
  FibonacciSpiral.ino
  sketch.yaml         board profile for arduino-cli
docs/images/          demo GIF and installation photos
```

## Credits

Built for COMS 3930 (Fall 2026) at Columbia. Uses [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) by Bodmer.
