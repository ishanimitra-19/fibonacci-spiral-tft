/**************************************************************************
  Generative Fibonacci spiral fractal for the TTGO T-Display (TFT_eSPI)

  Each cycle:
    - a golden rectangle is subdivided into squares (Fibonacci tiling)
    - an anti-aliased quarter arc is drawn in each square -> golden spiral
    - random sub-squares get their own smaller spiral (fractal recursion)
    - palette, orientation, line width and recursion depth are random
 **************************************************************************/
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

const float PHI = 1.6180339887f;

// ---- per-cycle random parameters ----
float baseHue;        // 0..360
float hueStep;        // hue change per square
bool  mirrorX, mirrorY;
int   maxDepth;       // fractal recursion depth (1 = plain spiral)
int   spawnChance;    // % chance a square spawns a child spiral
float lineW;          // arc width in pixels
bool  showSquares;    // outline the tiling squares?
int   stepDelay;      // ms between squares (animation speed)

// ---- helpers ----
uint16_t hsv565(float h, float s, float v) {
  while (h < 0) h += 360; while (h >= 360) h -= 360;
  float c = v * s, x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1)), m = v - c;
  float r, g, b;
  if      (h <  60) { r = c; g = x; b = 0; }
  else if (h < 120) { r = x; g = c; b = 0; }
  else if (h < 180) { r = 0; g = c; b = x; }
  else if (h < 240) { r = 0; g = x; b = c; }
  else if (h < 300) { r = x; g = 0; b = c; }
  else              { r = c; g = 0; b = x; }
  return tft.color565((r + m) * 255, (g + m) * 255, (b + m) * 255);
}

// map "design" coords to screen coords, applying the random mirror
inline float mx(float x) { return mirrorX ? tft.width()  - x : x; }
inline float my(float y) { return mirrorY ? tft.height() - y : y; }

void arcSeg(float cx, float cy, float r, float t0, float t1, float w, uint16_t col) {
  int n = max(6, (int)(r / 3));
  float px = cx + r * cosf(t0), py = cy + r * sinf(t0);
  for (int i = 1; i <= n; i++) {
    float t = t0 + (t1 - t0) * i / n;
    float x = cx + r * cosf(t), y = cy + r * sinf(t);
    tft.drawWideLine(mx(px), my(py), mx(x), my(y), w, col, TFT_BLACK);
    px = x; py = y;
  }
}

// Subdivide rect (x,y,w,h) into squares, drawing the spiral arc in each.
// dir cycles: 0 = square on left, 1 = top, 2 = right, 3 = bottom.
void spiral(float x, float y, float w, float h, int dir, float hue, int depth) {
  int guard = 0;
  while (min(w, h) >= 4 && guard++ < 24) {
    float s, sx, sy;               // square side + position
    if (w >= h) { s = h; sx = (dir == 0) ? x : x + w - s; sy = y; }
    else        { s = w; sx = x;   sy = (dir == 1) ? y : y + h - s; }

    // arc centre = square corner that keeps the curve tangent-continuous
    float cx, cy;
    switch (dir) {
      case 0: cx = sx + s; cy = sy + s; break;
      case 1: cx = sx;     cy = sy + s; break;
      case 2: cx = sx;     cy = sy;     break;
      default:cx = sx + s; cy = sy;     break;
    }
    float t0 = PI + dir * PI / 2;

    uint16_t col = hsv565(hue, 0.85f, depth == 1 ? 1.0f : 0.75f);
    if (showSquares && depth == 1)
      tft.drawRect(min(mx(sx), mx(sx + s)), min(my(sy), my(sy + s)), s, s, hsv565(hue, 0.4f, 0.25f));
    arcSeg(cx, cy, s, t0, t0 + PI / 2, depth == 1 ? lineW : max(1.0f, lineW - 1), col);

    // fractal: sometimes grow a child spiral inside this square
    if (depth < maxDepth && s > 24 && random(100) < spawnChance) {
      float pad = s * 0.12f;
      float cw = s - 2 * pad, ch = cw / PHI;
      spiral(sx + pad, sy + pad + (cw - ch) / 2, cw, ch, random(4), hue + 120, depth + 1);
    }

    // shrink to the remaining rectangle
    if (w >= h) { if (dir == 0) x += s; w -= s; }
    else        { if (dir == 1) y += s; h -= s; }
    dir = (dir + 1) % 4;
    hue += hueStep;
    if (depth == 1) delay(stepDelay);
  }
}

void newCycle() {
  baseHue     = random(360);
  hueStep     = random(8, 40);
  mirrorX     = random(2);
  mirrorY     = random(2);
  maxDepth    = random(1, 4);          // 1..3
  spawnChance = random(35, 90);
  lineW       = random(2, 5);          // 2..4 px
  showSquares = random(3) == 0;
  stepDelay   = random(60, 200);

  tft.fillScreen(TFT_BLACK);

  // golden rectangle that fills the screen height, centred horizontally
  float h = tft.height() - 6;
  float w = h * PHI;
  float x = (tft.width() - w) / 2, y = 3;
  spiral(x, y, w, h, 0, baseHue, 1);
}

void setup() {
  tft.init();
  tft.setRotation(1);
  randomSeed(esp_random() ^ analogRead(34));
  newCycle();
}

void loop() {
  // hold, then shimmer: redraw the same spiral with a shifting hue a few times
  delay(random(1500, 3000));
  for (int i = 0; i < 3; i++) {
    baseHue += 40;
    stepDelay = 0;
    spiral((tft.width() - (tft.height() - 6) * PHI) / 2, 3,
           (tft.height() - 6) * PHI, tft.height() - 6, 0, baseHue, 1);
    delay(400);
  }
  delay(random(1000, 2500));
  newCycle();
}
