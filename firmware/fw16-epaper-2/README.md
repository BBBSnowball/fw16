Pins of RP2040-Tiny - only for initial prototype:
- left:
  - 29: SPI1 CSn
  - 28: (SPI1 RX), DC
  - 27: SPI1 TX
  - 26: SPI1 SCK
  - 15: RST
  - 14: BUSY
- bottom:
  - 13
  - 12
  - 11
  - 10
- right:
  - 8
  - 7
  - 6
  - 5
  - 4
  - 3
  - 2
  - 1

APDS-9960:
We have [this module from Berrybase](https://www.berrybase.de/apds-9960-rgb-infrarot-gestensensor),
which doesn't seem to be an original chip. Its ID is 0xA8 instead of 0xAB, so we have to patch the
library. See [here](https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor/pull/25) for the
same issue with a sensor baught on AliExpress.

