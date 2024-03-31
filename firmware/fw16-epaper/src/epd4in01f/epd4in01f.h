/*****************************************************************************
* | File      	:   EPD_4in01f.h
* | Author      :   Waveshare team
* | Function    :   4.01inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-12-25
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

#ifndef __EPD_4IN01F_H__
#define __EPD_4IN01F_H__

#include <stdint.h>
#include "epdif.h"

// Display resolution
#define EPD_WIDTH       640
#define EPD_HEIGHT      400

#define UWORD   unsigned int
#define UBYTE   unsigned char
#define UDOUBLE  unsigned long

/**********************************
Color Index
**********************************/
enum EpdColor : UBYTE {
    EPD_4IN01F_BLACK   = 0x0,	/// 000
    EPD_4IN01F_WHITE   = 0x1,	///	001
    EPD_4IN01F_GREEN   = 0x2,	///	010
    EPD_4IN01F_BLUE    = 0x3,	///	011
    EPD_4IN01F_RED     = 0x4,	///	100
    EPD_4IN01F_YELLOW  = 0x5,	///	101
    EPD_4IN01F_ORANGE  = 0x6,	///	110
    EPD_4IN01F_CLEAN   = 0x7,	///	111   unavailable  Afterimage
};

class EpdUpdate;

class Epd : EpdIf {
public:
    Epd();
    ~Epd();
    int  Init(void);
	void EPD_4IN01F_BusyHigh(void);
	void EPD_4IN01F_BusyLow(void);
    void Reset(void);
    void EPD_4IN01F_Display(const UBYTE *image);
    void EPD_4IN01F_Display_part(const UBYTE *image, UWORD xstart, UWORD ystart, 
                                 UWORD image_width, UWORD image_height);
    template<typename F>
    void EPD_4IN01F_DisplayF(const F& func);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void SendData16(uint16_t data);
    void SetResolution(uint16_t width = EPD_WIDTH, uint16_t height = EPD_HEIGHT);
    void Sleep(void);
    void Clear(UBYTE color);

    //NOTE only write to pixels that have been cleared; if in doubt, call Clear(EPD_4IN01F_CLEAN)
    EpdUpdate start_full_update();
    EpdUpdate start_partial_update(uint16_t xstart, uint16_t ystart,
                                 uint16_t image_width, uint16_t image_heigh);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
    unsigned long width;
    unsigned long height;
};

class EpdUpdate {
    friend class Epd;

    Epd& epd;
    uint16_t startx, width, starty, height;
    int32_t remaining;
    EpdUpdate(Epd& epd, uint16_t startx, uint16_t starty, uint16_t width, uint16_t height);
    EpdUpdate(const EpdUpdate&) = delete;
public:
    uint16_t get_startx() { return startx; }
    uint16_t get_width()  { return width; }
    uint16_t get_starty() { return starty; }
    uint16_t get_height() { return height; }
    uint32_t get_remaining() { return remaining; }

    void put2(uint8_t two_pixels);
    template<typename F>
    void put_all(const F& func);
    template<typename F>
    void put2_all(const F& func);
    void finish();
};

enum EPD_Command {
    CMD_POWER_OFF = 0x02,
    CMD_POWER_ON  = 0x04,
    CMD_BOOSTER_SOFT_START = 0x06,
    CMD_DEEP_SLEEP = 0x07,
    CMD_WINM = 0x14,
    CMD_WHRES = 0x15,
    CMD_WVRES = 0x16,
    CMD_CCSET = 0xe0,
};

template<typename F>
void EpdUpdate::put_all(const F& func) {
    for(uint16_t y=starty; y<starty+height; y++) {
        for(uint16_t x=startx; x<startx+width; x+=2) {
            //FIXME which is the upper one?
            put2(((func(x, y) & 0xf) << 4) | (func(x+1, y) & 0xf));
        }
    }
}

template<typename F>
void EpdUpdate::put2_all(const F& func) {
    for(uint16_t y=starty; y<starty+height; y++) {
        for(uint16_t x=startx; x<startx+width; x+=2) {
            put2(func(x, y));
        }
    }
}

template<typename F>
void Epd::EPD_4IN01F_DisplayF(const F& func) {
    auto update = start_full_update();
    update.put_all(func);
    update.finish();
    DelayMs(200);
}

#endif /* EPD5IN83B_HD_H */

/* END OF FILE */
