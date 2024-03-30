/**
   Copyright (C) Waveshare     Dec 25 2020
   Copyright (C) snowball      2024

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documnetation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to  whom the Software is
   furished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <SPI.h>
#include <FastLED.h>
#include "epd4in01f/imagedata.h"
#include "epd4in01f/epd4in01f.h"
#include "pico/printf.h"

#define NUM_LEDS 1
#define WS2812_PIN 16

CRGB leds[NUM_LEDS];
Epd epd;

// Replace panic handler, so we can see panic on USB serial:
//   cd ~/.platformio/packages/framework-arduinopico/lib
//   ar x libpico.a runtime.c.obj
//   ../../toolchain-rp2040-earlephilhower/bin/arm-none-eabi-objcopy runtime.c.obj runtime2.c.obj -W panic
//   mv runtime2.c.obj runtime.c.obj
//   ar r libpico.a runtime.c.obj

char buf[256];
void __attribute__((noreturn)) panic(const char *fmt, ...) {
  leds[0] = CRGB(50, 0, 0);
  FastLED.show();

  while (!Serial)
    ;

  Serial.print("\n*** PANIC ***\n");
  if (fmt) {
#if LIB_PICO_PRINTF_NONE
      Serial.print(fmt);
#else
      va_list args;
      va_start(args, fmt);
      vsprintf(buf, fmt, args);
      va_end(args);
      Serial.println(buf);
#endif
  }

  while (1) {
    leds[0] = CRGB(50, 0, 0);
    FastLED.show();
    delay(200);
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("init");

  FastLED.addLeds<WS2812, WS2812_PIN>(leds, NUM_LEDS);

  leds[0] = CRGB(10, 0, 0);
  FastLED.show();

  while (!Serial)
    ;

  leds[0] = CRGB(10, 5, 0);
  FastLED.show();

  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return;
  }
  
  Serial.print("e-Paper Clear\r\n ");
  epd.Clear(EPD_4IN01F_WHITE);
  
  Serial.print("draw image\r\n ");
  epd.EPD_4IN01F_Display_part(gImage_4in01f, 204, 153, 192, 143);
}

void loop() {
  leds[0] = CRGB(0, 10, 0);
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}
