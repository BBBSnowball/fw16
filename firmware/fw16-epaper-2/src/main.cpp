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
#include <bitset>

#include <SPI.h>
#include "epd3in7/epd3in7.h"
#include "epd3in7/imagedata.h"
#include "epd3in7/epdpaint.h"

std::bitset<EPD_WIDTH*EPD_HEIGHT> pixelIsClean;
Epd epd;


#define NUM_LEDS 1
#define WS2812_PIN 16

CRGB leds[NUM_LEDS];

#if 1
#include "pico/printf.h"

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
#endif

typedef uint8_t EpdColor;

EpdColor char_to_color(char c) {
  switch (c) {
    case ' ': return 0;
    case '-': return 1;
    case '+': return 2;
    case '#': return 3;
    default:  return 0;
  }
}

struct EpdUpdate {
  uint8_t data[EPD_WIDTH*EPD_HEIGHT/8];
  uint32_t cnt;

  void begin() {
    cnt = 0;
  }

  int get_remaining() {
    return EPD_WIDTH*EPD_HEIGHT - cnt;
  }

  void put(EpdColor a) {
    if (cnt >= EPD_WIDTH*EPD_HEIGHT)
      return;
    int i = cnt/8;
    int shift = 7 - cnt%8;
    data[i] = (data[i] & ~(1<<shift)) | ((a?1:0)<<shift);
    cnt++;
  }

  void put2(EpdColor a, EpdColor b) {
    put(a);
    put(b);
  }

  void finish() {
    if (cnt != EPD_WIDTH*EPD_HEIGHT) {
      Serial.print("E: not enough data, got");
      Serial.print(cnt);
      Serial.print(", but we need ");
      Serial.println(EPD_WIDTH*EPD_HEIGHT);
      return;
    }
    if (epd.Init() != 0) {
      Serial.println("E: e-Paper init failed");
      return;
    }
    epd.Clear(1);  
    epd.DisplayFrame(data, true);
    epd.Sleep();
  }
};
EpdUpdate epdUpdate;

uint16_t win_startx = 0, win_starty = 0, win_endx = EPD_WIDTH, win_endy = EPD_HEIGHT;
bool win_active = false;

void handle_line(const char* buf, int len) {
  switch (buf[0]) {
    case 0:
      return;

    case 'C': {
      EpdColor color = 0;
      if (buf[1] == 0) {
      } else if (buf[1] == ':' && buf[3] == 0) {
        color = char_to_color(buf[2]);
      } else {
        Serial.println("E:INVALID");
        return;
      }

      if (epd.Init() != 0) {
        Serial.println("E: e-Paper init failed");
        break;
      }
      epd.Clear(color);
      epd.Sleep();

      //if (win_active) {
      //  for (uint16_t y = win_starty; y < win_endy; y++) {
      //    for (uint16_t x = win_startx; x < win_endx; x++) {
      //      pixelIsClean.set(x + y*EPD_WIDTH, color == EPD_4IN01F_CLEAN);
      //    }
      //  }
      //} else if (color == EPD_4IN01F_CLEAN) {
      //  pixelIsClean.set();
      //} else {
      //  pixelIsClean.reset();
      //}
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
      //TODO
      //auto update = win_active
      //  ? epd.start_partial_update(win_startx, win_starty, win_endx-win_startx, win_endy-win_starty)
      //  : epd.start_full_update();
      auto& update = epdUpdate;
      update.begin();
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
          //if (a > EPD_4IN01F_CLEAN || b > EPD_4IN01F_CLEAN) {
          //  Serial.println("E:INVALID");
          //  return;
          //}
          //FIXME which one should be the upper?
          //FIXME check and update pixelIsClean!
          //update.put2((a << 4) | b);
          update.put(a);
          update.put(b);
          n = 0;
        }
        prev = c;
      }
    }
    break;

    case 'P': {
      /*
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
      */

      if (epd.Init() != 0) {
        Serial.println("E: e-Paper init failed");
        break;
      }
      Serial.print("I:3.7inch e-paper demo\r\n ");
      Serial.print("I:e-Paper Clear\r\n ");
      epd.Clear(1);  
      Serial.print("I:draw image\r\n ");
      epd.DisplayFrame(IMAGE_DATA, true);
      epd.Sleep();

      Serial.println("OK");
    }
    break;

    case 'I':
      //if (epd.Init() == 0)
      //  Serial.println("OK");
      //else
      //  Serial.println("E:FAILED");
      Serial.println("OK");
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

int numReceived = 0;

void poll_serial() {
  static int prev = 0;
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
      if (c == '\r' && prev == '\r')
        Serial.println("I: hint: use ctrl+j to send line feed");
    } else {
      numReceived++;
      if (numReceived == sizeof(buf)-1) {
        Serial.println("E:OVFL1");
      } else if (numReceived < sizeof(buf)-1) {
        buf[numReceived-1] = c;
      }
    }

    if (c > 0)
      prev = c;
  }
}


#define COLORED     0
#define UNCOLORED   1

UBYTE image[700];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
UDOUBLE time_start_ms;
UDOUBLE time_now_s;

ATECCX08A atecc;

void testEpaper(bool full);
void testSecurityChip();

void setup() {
    Serial.begin(115200);
    Serial.println("I:init");

    FastLED.addLeds<WS2812, WS2812_PIN>(leds, NUM_LEDS);

    leds[0] = CRGB(0, 0, 3);
    FastLED.show();

    testEpaper(false);

    testSecurityChip();

    Serial.print("I:done\r\n ");
}

const int TOUCH_Y0 = 26;
const int TOUCH_X0 = 27;
const int TOUCH_X1 = 28;
const int TOUCH_Y1 = 29;

static bool printTouch = false;

void loop() {
  unsigned long prevBlink = 0;
  if (millis()-prevBlink > 500) {
    prevBlink = millis();
    leds[0] = leds[0].red ? CRGB(0, 3, 0) : CRGB(3, 3, 0);
    FastLED.show();
  }

  pinMode(TOUCH_Y0, INPUT);
  pinMode(TOUCH_Y1, INPUT);
  pinMode(TOUCH_X0, OUTPUT);
  digitalWrite(TOUCH_X0, 1);
  pinMode(TOUCH_X1, OUTPUT);
  digitalWrite(TOUCH_X1, 0);
  int a = analogRead(TOUCH_Y0);
  int b = analogRead(TOUCH_Y1);

  pinMode(TOUCH_X0, INPUT);
  pinMode(TOUCH_X1, INPUT);
  pinMode(TOUCH_Y0, OUTPUT);
  digitalWrite(TOUCH_Y0, 1);
  pinMode(TOUCH_Y1, OUTPUT);
  digitalWrite(TOUCH_Y1, 0);
  int c = analogRead(TOUCH_X0);
  int d = analogRead(TOUCH_X1);
  if (printTouch) {
    Serial.print("I:Touch: ");
    Serial.print(a);
    Serial.print(", ");
    Serial.print(b);
    Serial.print(", ");
    Serial.print(c);
    Serial.print(", ");
    Serial.println(d);
  }

  if (0) {
    int ch = Serial.read();
    switch (ch) {
      case 'T':
        printTouch = true;
        break;
      case '\r':
      case '\n':
        Serial.print((char)c);
        if (printTouch) {
          printTouch = false;
        }
        break;
      case 'e':
        testEpaper(false);
        break;
      case 'E':
        testEpaper(true);
        break;
      case 'c':
        if (epd.Init() != 0) {
            Serial.println("E: e-Paper init failed");
        } else {
          epd.Clear(1);
          epd.Sleep();
        }
        break;
      case 'S':
        testSecurityChip();
        break;
    }
  } else {
    poll_serial();
  }
}

void testEpaper(bool full) {
  if (epd.Init() != 0) {
      Serial.println("E: e-Paper init failed");
      return;
  }
  Serial.print("I:3.7inch e-paper demo\r\n ");
  Serial.print("I:e-Paper Clear\r\n ");
  epd.Clear(1);  
  Serial.print("I:draw image\r\n ");
  epd.DisplayFrame(IMAGE_DATA, true);   // Set base image

  if (!full) {
    epd.Sleep();
    return;
  }

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
      Serial.print("I:refresh------\r\n ");
      // epd.DisplayFrame_Partial(paint.GetImage(), 20, 100, 40, 120); // Width must be a multiple of 8
      /* Writes new data to RAM */
      epd.DisplayFrame_Part(paint.GetImage(), 40+i*40, 30, 80+i*40, 140, false);   // Xstart must be a multiple of 8
      /* Displays and toggles the RAM currently in use */
      epd.TurnOnDisplay();
      /* Writes the last data to another RAM */
      epd.DisplayFrame_Part(paint.GetImage(), 40+i*40, 30, 80+i*40, 140, false);   // Xstart must be a multiple of 8
      delay(500);
  }
  
  Serial.print("I:clear and sleep......\r\n ");
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

  Serial.print("I:Serial Number: \t");
  for (int i = 0 ; i < 9 ; i++)
  {
    if ((atecc.serialNumber[i] >> 4) == 0) Serial.print("0"); // print preceeding high nibble if it's zero
    Serial.print(atecc.serialNumber[i], HEX);
  }
  Serial.println();

  Serial.print("I:Rev Number: \t");
  for (int i = 0 ; i < 4 ; i++)
  {
    if ((atecc.revisionNumber[i] >> 4) == 0) Serial.print("0"); // print preceeding high nibble if it's zero
    Serial.print(atecc.revisionNumber[i], HEX);
  }
  Serial.println();

  Serial.print("I:Config Zone: \t");
  if (atecc.configLockStatus) Serial.println("Locked");
  else Serial.println("NOT Locked");

  Serial.print("I:Data/OTP Zone: \t");
  if (atecc.dataOTPLockStatus) Serial.println("Locked");
  else Serial.println("NOT Locked");

  Serial.print("I:Data Slot 0: \t");
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
    Serial.println("I:Successful wakeUp(). I2C connections are good.");
  }
  else
  {
    Serial.println("E:Device not found. Check wiring.");
    return;
  }

  printInfo(); // see function below for library calls and data handling

  // check for configuration
  if (!(atecc.configLockStatus && atecc.dataOTPLockStatus && atecc.slot0LockStatus))
  {
    Serial.println("W:Device not configured. Please use the configuration sketch.");
    return;
  }

  long myRandomNumber = atecc.random(100);
  Serial.print("I:Random number: ");
  Serial.println(myRandomNumber);
}

