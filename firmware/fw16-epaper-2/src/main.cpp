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
#include <Wire.h>
#include <SparkFun_ATECCX08a_Arduino_Library.h>
#include "pico/printf.h"

#define NUM_LEDS 1
#define WS2812_PIN 16

CRGB leds[NUM_LEDS];
//Epd epd;

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

#if 0

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
  //epd.Clear(EPD_4IN01F_WHITE);
  // reference PDF says to clear with 0x77
  epd.Clear(EPD_4IN01F_CLEAN);
  
  Serial.print("draw image\r\n ");
  if (0) {
    epd.EPD_4IN01F_Display_part(gImage_4in01f, 204, 153, 192, 143);
  } else if (1) {
    //epd.EPD_4IN01F_Display_part(gImage_4in01f, 208, 0, 192, 143);
    //epd.EPD_4IN01F_Display_part(gImage_4in01f, 204, 152, 192, 142);
    epd.EPD_4IN01F_Display_part(gImage_4in01f, 24, 20, 192, 142);
    epd.EPD_4IN01F_Display_part(gImage_4in01f, 48, 220, 192, 142);
  } else {
    epd.EPD_4IN01F_DisplayF([](int x, int y) {
      return (x/4 + y/16) % 8;
    });
  }
}

void loop() {
  leds[0] = CRGB(0, 10, 0);
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}

#elif 0

#include <bitset>

std::bitset<EPD_WIDTH*EPD_HEIGHT> pixelIsClean;

void setup() {
  Serial.begin(115200);
  Serial.println("I:init");

  FastLED.addLeds<WS2812, WS2812_PIN>(leds, NUM_LEDS);

  leds[0] = CRGB(0, 0, 3);
  FastLED.show();

  if (epd.Init() != 0) {
    Serial.println("E: e-Paper init failed");
    return;
  }

  // reference PDF says to clear with 0x77
  //epd.Clear(EPD_4IN01F_CLEAN);

  pixelIsClean.reset();
}

EpdColor char_to_color(char c) {
  switch (c) {
    case 'k': return EPD_4IN01F_BLACK;
    case 'K': return EPD_4IN01F_BLACK;
    case 'w': return EPD_4IN01F_WHITE;
    case 'W': return EPD_4IN01F_WHITE;
    case 'g': return EPD_4IN01F_GREEN;
    case 'G': return EPD_4IN01F_GREEN;
    case 'b': return EPD_4IN01F_BLUE;
    case 'B': return EPD_4IN01F_BLUE;
    case 'r': return EPD_4IN01F_RED;
    case 'R': return EPD_4IN01F_RED;
    case 'y': return EPD_4IN01F_YELLOW;
    case 'Y': return EPD_4IN01F_YELLOW;
    case 'o': return EPD_4IN01F_ORANGE;
    case 'O': return EPD_4IN01F_ORANGE;
    case 'c': return EPD_4IN01F_CLEAN;
    case 'C': return EPD_4IN01F_CLEAN;
    case ' ': return EPD_4IN01F_CLEAN;
    default:  return (EpdColor)8;
  }
}

uint16_t win_startx = 0, win_starty = 0, win_endx = EPD_WIDTH, win_endy = EPD_HEIGHT;
bool win_active = false;

void handle_line(const char* buf, int len) {
  switch (buf[0]) {
    case 0:
      return;

    case 'C': {
      auto color = EPD_4IN01F_CLEAN;
      if (buf[1] == 0) {
      } else if (buf[1] == ':' && buf[3] == 0) {
        color = char_to_color(buf[2]);
      } else {
        Serial.println("E:INVALID");
        return;
      }

      epd.Clear(color);

      if (win_active) {
        for (uint16_t y = win_starty; y < win_endy; y++) {
          for (uint16_t x = win_startx; x < win_endx; x++) {
            pixelIsClean.set(x + y*EPD_WIDTH, color == EPD_4IN01F_CLEAN);
          }
        }
      } else if (color == EPD_4IN01F_CLEAN) {
        pixelIsClean.set();
      } else {
        pixelIsClean.reset();
      }
      Serial.println("OK");
    }
    break;

    case 'W':
      if (buf[1] == 0) {
        win_startx = 0;
        win_starty = 0;
        win_endx = EPD_WIDTH;
        win_endy = EPD_HEIGHT;
        win_active = false;
      } else if (buf[1] != ':') {
        Serial.println("E:INVALID");
        return;
      } else {
        bool ok = true;
        const char* p = buf+2;
        char* p2 = NULL;
        if (ok) {
          long a = strtoul(p, &p2, 10);
          if (!p2 || a % 8 || a < 0 || a > EPD_WIDTH)
            ok = false;
          else
            win_startx = a;
        }
        if (p2 && p2[0] == ',')
          ++p2;
        if (ok) {
          long a = strtoul(p, &p2, 10);
          if (!p2 || a < 0 || a > EPD_HEIGHT)
            ok = false;
          else
            win_starty = a;
        }
        if (p2 && p2[0] == ',')
          ++p2;
        if (ok) {
          long a = strtoul(p, &p2, 10);
          if (!p2 || a <= 0 || a % 8 || win_startx + a > EPD_WIDTH)
            ok = false;
          else
            win_endx = win_startx + a;
        }
        if (p2 && p2[0] == ',')
          ++p2;
        if (ok) {
          long a = strtoul(p, &p2, 10);
          if (!p2 || a <= 0 || win_starty + a > EPD_HEIGHT)
            ok = false;
          else
            win_endy = win_starty + a;
        }
        if (!p2 || *p2)
          ok = false;

        win_active = ok;
        if (ok) {
          Serial.println("OK");
        } else {
          win_startx = 0;
          win_starty = 0;
          win_endx = EPD_WIDTH;
          win_endy = EPD_HEIGHT;
          win_active = false;
        }
      }
      break;

    case 'D': {
      if (buf[1]) {
        Serial.println("E:INVALID");
        return;
      }
      auto update = win_active
        ? epd.start_partial_update(win_startx, win_starty, win_endx-win_startx, win_endy-win_starty)
        : epd.start_full_update();
      char prev = 0;
      UBYTE color = 0;
      int n = 0;
      while (1) {
        while (!Serial.available())
          ;
        int c = Serial.read();
        if (c <= 0 || c == '\r')
          continue;
        if (c == '\n' && prev == '\n' || c == 3) {
          Serial.println("E:ABORT");
          return;
        } else if (c == '\n') {
          n = 0;
          if (!update.get_remaining()) {
            update.finish();
            Serial.println("OK");
            return;
          }
        } else if (c == '?') {
          Serial.print("I:n=");
          Serial.print(n);
          Serial.print(",r=");
          Serial.println(update.get_remaining());
          continue;
        } else if (n == 0) {
          n++;
        } else {
          auto a = char_to_color(prev);
          auto b = char_to_color(c);
          if (a > EPD_4IN01F_CLEAN || b > EPD_4IN01F_CLEAN) {
            Serial.println("E:INVALID");
            return;
          }
          //FIXME which one should be the upper?
          //FIXME check and update pixelIsClean!
          update.put2((a << 4) | b);
          n = 0;
        }
        prev = c;
      }
    }
    break;

    case 'P': {
      int width = 192;
      int height = 143;
      bool ok = true;
      const char* p = buf+2;
      char* p2 = NULL;
      long x, y;
      if (ok) {
        x = strtoul(p, &p2, 10);
        if (!p2 || x < 0 || x + width > EPD_WIDTH)
          ok = false;
      }
      if (p2 && p2[0] == ',')
        ++p2;
      if (ok) {
        y = strtoul(p, &p2, 10);
        if (!p2 || y < 0 || y + height > EPD_HEIGHT)
          ok = false;
      }
      if (!ok) {
        Serial.println("E:INVALID");
        return;
      }
      epd.EPD_4IN01F_Display_part(gImage_4in01f, x, y, width, height);
      Serial.println("OK");
    }
    break;

    case 'I':
      if (epd.Init() == 0)
        Serial.println("OK");
      else
        Serial.println("E:FAILED");
      break;

    case 'h':
    case 'H':
    case '?':
      Serial.println("# End commands with newline. Reply is OK. Send two newlines or ctrl-c to abort most commands.");
      Serial.println("# C                  clear");
      Serial.println("# C:<c>              fill with color");
      Serial.println("# I                  re-initialize");
      Serial.println("# W                  disable window, i.e. update everything");
      Serial.println("# W:<x>,<y>,<w>,<h>  set window");
      Serial.println("# D\\n<c>*\\n          draw pixels");
      Serial.println("#     <c> is: blacK, White, Green, Blue, Red, Yellow, Orange, Clean");
      Serial.println("#     (capital letter is code for that color)");
      break;

    default:
      Serial.println("E:INVALID");
      break;
  }
}

void loop() {
  int numReceived = 0;
  while (1) {
    if (Serial.available()) {
      int c = Serial.read();
      if (c == '\n') {
        if (numReceived < sizeof(buf)-1) {
          buf[numReceived] = 0;
          handle_line(buf, numReceived);
        } else {
          Serial.println("E:OVFL2");
        }
        numReceived = 0;
      } else if (c == '\r' || c < 32 || c >= 128) {
        // ignore
      } else {
        numReceived++;
        if (numReceived == sizeof(buf)-1) {
          Serial.println("E:OVFL1");
        } else if (numReceived < sizeof(buf)-1) {
          buf[numReceived-1] = c;
        }
      }
    }
  }
}

#else


#include <SPI.h>
#include "epd3in7/epd3in7.h"
#include "epd3in7/imagedata.h"
#include "epd3in7/epdpaint.h"

#define COLORED     0
#define UNCOLORED   1

UBYTE image[700];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
UDOUBLE time_start_ms;
UDOUBLE time_now_s;

ATECCX08A atecc;

void testEpaper();
void testSecurityChip();

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    FastLED.addLeds<WS2812, WS2812_PIN>(leds, NUM_LEDS);

    leds[0] = CRGB(0, 0, 3);
    FastLED.show();

    testEpaper();

    testSecurityChip();

    Serial.print("done\r\n ");
}

void loop() {
  leds[0] = CRGB(3, 3, 0);
  FastLED.show();
  delay(500);
  leds[0] = CRGB(0, 3, 0);
  FastLED.show();
  delay(500);
}

void testEpaper() {
  Epd epd;
  if (epd.Init() != 0) {
      Serial.print("e-Paper init failed");
      return;
  }
  Serial.print("3.7inch e-paper demo\r\n ");
  Serial.print("e-Paper Clear\r\n ");
  epd.Clear(1);  
  Serial.print("draw image\r\n ");
  epd.DisplayFrame(IMAGE_DATA, true);   // Set base image
  delay(3000);
  
  paint.SetWidth(40);
  paint.SetHeight(120);
  paint.SetRotate(ROTATE_270);
  paint.Clear(UNCOLORED);

  leds[0] = CRGB(0, 3, 0);
  FastLED.show();

  UBYTE i;
  time_start_ms = millis();
  for(i=0; i<5; i++) {
      time_now_s = (millis() - time_start_ms) / 1000;
      char time_string[] = {'0', '0', ':', '0', '0', '\0'};
      time_string[0] = time_now_s / 60 / 10 + '0';
      time_string[1] = time_now_s / 60 % 10 + '0';
      time_string[3] = time_now_s % 60 / 10 + '0';
      time_string[4] = time_now_s % 60 % 10 + '0';

      paint.Clear(UNCOLORED);
      paint.DrawStringAt(20, 10, time_string, &Font16, COLORED);
      Serial.print("refresh------\r\n ");
      // epd.DisplayFrame_Partial(paint.GetImage(), 20, 100, 40, 120); // Width must be a multiple of 8
      /* Writes new data to RAM */
      epd.DisplayFrame_Part(paint.GetImage(), 40+i*40, 30, 80+i*40, 140, false);   // Xstart must be a multiple of 8
      /* Displays and toggles the RAM currently in use */
      epd.TurnOnDisplay();
      /* Writes the last data to another RAM */
      epd.DisplayFrame_Part(paint.GetImage(), 40+i*40, 30, 80+i*40, 140, false);   // Xstart must be a multiple of 8
      delay(500);
  }
  
  Serial.print("clear and sleep......\r\n ");
  epd.Init();
  epd.Clear(1);
  epd.Sleep();
}

// copied from https://github.com/sparkfun/SparkFun_ATECCX08a_Arduino_Library/blob/master/examples/Example5_Random/Example5_Random.ino
void printInfo()
{
  // Read all 128 bytes of Configuration Zone
  // These will be stored in an array within the instance named: atecc.configZone[128]
  atecc.readConfigZone(false); // Debug argument false (OFF)

  // Print useful information from configuration zone data
  Serial.println();

  Serial.print("Serial Number: \t");
  for (int i = 0 ; i < 9 ; i++)
  {
    if ((atecc.serialNumber[i] >> 4) == 0) Serial.print("0"); // print preceeding high nibble if it's zero
    Serial.print(atecc.serialNumber[i], HEX);
  }
  Serial.println();

  Serial.print("Rev Number: \t");
  for (int i = 0 ; i < 4 ; i++)
  {
    if ((atecc.revisionNumber[i] >> 4) == 0) Serial.print("0"); // print preceeding high nibble if it's zero
    Serial.print(atecc.revisionNumber[i], HEX);
  }
  Serial.println();

  Serial.print("Config Zone: \t");
  if (atecc.configLockStatus) Serial.println("Locked");
  else Serial.println("NOT Locked");

  Serial.print("Data/OTP Zone: \t");
  if (atecc.dataOTPLockStatus) Serial.println("Locked");
  else Serial.println("NOT Locked");

  Serial.print("Data Slot 0: \t");
  if (atecc.slot0LockStatus) Serial.println("Locked");
  else Serial.println("NOT Locked");

  Serial.println();

  // omitted printing public key, to keep this example simple and focused on just random numbers.
}

void testSecurityChip() {
  Wire.setSDA(12);
  Wire.setSCL(13);
  Wire.begin();

  // copied from https://github.com/sparkfun/SparkFun_ATECCX08a_Arduino_Library/blob/master/examples/Example5_Random/Example5_Random.ino

  if (atecc.begin() == true)
  {
    Serial.println("Successful wakeUp(). I2C connections are good.");
  }
  else
  {
    Serial.println("Device not found. Check wiring.");
    return;
  }

  printInfo(); // see function below for library calls and data handling

  // check for configuration
  if (!(atecc.configLockStatus && atecc.dataOTPLockStatus && atecc.slot0LockStatus))
  {
    Serial.print("Device not configured. Please use the configuration sketch.");
    return;
  }

  long myRandomNumber = atecc.random(100);
  Serial.print("Random number: ");
  Serial.println(myRandomNumber);
}

#endif
