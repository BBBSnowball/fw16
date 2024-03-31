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
#define EPD_4IN01F_BLACK   0x0	/// 000
#define EPD_4IN01F_WHITE   0x1	///	001
#define EPD_4IN01F_GREEN   0x2	///	010
#define EPD_4IN01F_BLUE    0x3	///	011
#define EPD_4IN01F_RED     0x4	///	100
#define EPD_4IN01F_YELLOW  0x5	///	101
#define EPD_4IN01F_ORANGE  0x6	///	110
#define EPD_4IN01F_CLEAN   0x7	///	111   unavailable  Afterimage

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
                                 UWORD image_width, UWORD image_heigh);
    void EPD_4IN01F_Display_part2(const UBYTE *image, UWORD xstart, UWORD ystart, 
                                 UWORD image_width, UWORD image_heigh);
    template<typename F>
    void EPD_4IN01F_DisplayF(const F& func);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void SendData16(uint16_t data);
    void SetResolution(uint16_t width = EPD_WIDTH, uint16_t height = EPD_HEIGHT);
    void Sleep(void);
    void Clear(UBYTE color);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
    unsigned long width;
    unsigned long height;
};

enum EPD_Command {
    CMD_POWER_OFF = 0x02,
    CMD_POWER_ON  = 0x04,
    CMD_BOOSTER_SOFT_START = 0x06,
    CMD_DEEP_SLEEP = 0x07,
template<typename F>
void Epd::EPD_4IN01F_DisplayF(const F& func) {
    unsigned long i,j;
    SetResolution();
    SendCommand(0x10);
    for(i=0; i<height; i++) {
        for(j=0; j<width; j+=2) {
            //FIXME which is the upper one?
			SendData(((func(j, i) & 0xf) << 4) | (func(j+1, i) & 0xf));
		}
    }
    SendCommand(CMD_POWER_ON);
    EPD_4IN01F_BusyHigh();
    SendCommand(0x12);  // display refresh?
    EPD_4IN01F_BusyHigh();
    SendCommand(CMD_POWER_OFF);
    EPD_4IN01F_BusyLow();
    DelayMs(200);
}

#endif /* EPD5IN83B_HD_H */

/* END OF FILE */
