/*****************************************************************************
* | File      	:   EPD_4in01f.c
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

#include <stdlib.h>
#include "epd4in01f.h"
#include "imagedata.h"

Epd::~Epd() {
};

Epd::Epd() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
};

void Epd::SetResolution(uint16_t width, uint16_t height) {
	SendCommand(0x61);
	SendData16(width);
	SendData16(height);
}

/******************************************************************************
function :  Initialize the e-Paper register
parameter:
******************************************************************************/
int Epd::Init(void) {
	if (IfInit() != 0) {
	    return -1;
	}
	Reset();
	EPD_4IN01F_BusyHigh();
	SendCommand(0x00);  // panel setting?
	SendData(0x2f);
	SendData(0x00);
	SendCommand(0x01);  // power setting?
	SendData(0x37);
	SendData(0x00);
	SendData(0x05);
	SendData(0x05);
	SendCommand(0x03);
	SendData(0x00);
	SendCommand(CMD_BOOSTER_SOFT_START);
	SendData(0xC7);
	SendData(0xC7);
	SendData(0x1D);
	SendCommand(0x41);
	SendData(0x00);
	SendCommand(0x50);
	SendData(0x37);
	SendCommand(0x60);
	SendData(0x22);
    SetResolution();
	SendCommand(0xE3);
	SendData(0xAA);
	return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

void Epd::SendData16(uint16_t data) {
    SendData(data >> 8);
    SendData(data & 0xff);
}

void Epd::EPD_4IN01F_BusyHigh(void)// If BUSYN=0 then waiting
{
    while(!(DigitalRead(BUSY_PIN)));
}

void Epd::EPD_4IN01F_BusyLow(void)// If BUSYN=1 then waiting
{
    while(DigitalRead(BUSY_PIN));
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void Epd::Reset(void) {
    DigitalWrite(reset_pin, HIGH);
    DelayMs(200);   
    DigitalWrite(reset_pin, LOW);                //module reset    
    DelayMs(1);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(200);    
}

/******************************************************************************
function :  Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void Epd::EPD_4IN01F_Display(const UBYTE *image) {
    auto update = start_full_update();
    update.put2_all([=](uint16_t x, uint16_t y) {
			return image[x/2+((width/2)*y)];
    });
    update.finish();
	DelayMs(200);
}

/******************************************************************************
function : 
      Clear screen
******************************************************************************/
void Epd::Clear(UBYTE color) {
    auto update = start_full_update();
    update.put2_all([=](uint16_t x, uint16_t y) {
        return EPD_4IN01F_CLEAN | (EPD_4IN01F_CLEAN << 4);
    });
    update.finish();
	DelayMs(500);
}

/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          The only one parameter is a check code, the command would be
 *          You can use EPD_Reset() to awaken
 */
void Epd::Sleep(void) {
    DelayMs(100);
    SendCommand(CMD_DEEP_SLEEP);
    SendData(0xA5);
    DelayMs(100);
	DigitalWrite(RST_PIN, 0); // Reset
}

/******************************************************************************
function :  Sends the part image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void Epd::EPD_4IN01F_Display_part(const UBYTE *image, UWORD xstart, UWORD ystart, 
                                        UWORD image_width, UWORD image_height)
{
    auto update = start_partial_update(xstart, ystart, image_width, image_height);
    update.put2_all([=](uint16_t x, uint16_t y) {
        if (x < xstart || x-xstart >= image_width)
            return (EpdColor)(EPD_4IN01F_WHITE | (EPD_4IN01F_WHITE << 4));
        else
            return (EpdColor)image[(x-xstart)/2 + (y-ystart)*image_width/2];
    });
    update.finish();
	DelayMs(200);
}

EpdUpdate Epd::start_full_update() {
    SetResolution();

    // disable partial mode
    SendCommand(CMD_WINM);
    SendData(0);
    SendCommand(CMD_CCSET);
    SendData(0x00);

    SendCommand(0x10);
    return EpdUpdate(*this, 0, 0, EPD_WIDTH, EPD_HEIGHT);
}

EpdUpdate Epd::start_partial_update(uint16_t xstart, uint16_t ystart,
                                 uint16_t width, uint16_t height) {
    // widen columns if they aren't already divisible by 8.
    //NOTE We have two pixels per byte but datasheet still says it should be a multiple of 8.
    //FIXME Can we make the window a multiple of 8 but keep the exact bounds when writing data?
    if (xstart % 8) {
        width += (xstart % 8);
        xstart -= (xstart % 8);
    }
    if (width % 8) {
        width = width/8*8 + 8;
    }
    uint16_t xend = xstart + width;

    SetResolution();

    // Based on this: https://www.waveshare.com/w/upload/b/bf/SPD1656_1.1.pdf
    // (found via https://github.com/adafruit/Adafruit_CircuitPython_SPD1656)
    SendCommand(CMD_WHRES);
    SendData16(xstart);
    SendData16(xstart + width - 1);
    SendCommand(CMD_WVRES);
    SendData16(ystart);
    SendData16(ystart + height - 1);
    SendCommand(CMD_WINM);
    SendData(1);  // enable window mode

    SendCommand(CMD_CCSET);
    SendData(0x80);  // PartialRAM mode

    // description for CCSET mentions 0xa1, 0xd4, 0xd5, 0xde, and 0xdf; so let's see what they do
    SendCommand(0xd4);  // set active columns
    SendData16(xstart);
    SendData16(xstart + width - 1);
    SendCommand(0xdf);  // set start row
    SendData16(ystart);
    //SendData16(yend-1);
    SendCommand(0xde);  // set start offset
    SendData16(xstart); // (columns will be shifted/rotated without that)
    //SendCommand(0xa1);  // no idea; breaks things
    //SendData16(0);

    SendCommand(0x10);
    return EpdUpdate { *this, xstart, ystart, width, height };
}

EpdUpdate::EpdUpdate(Epd& epd, uint16_t startx, uint16_t starty, uint16_t width, uint16_t height) : epd(epd) {
    this->startx = startx;
    this->width = width;
    this->starty = starty;
    this->height = height;
    this->remaining = width * height;
}

void EpdUpdate::put2(uint8_t two_pixels) {
    if (!remaining)
        return;
    epd.SendData(two_pixels);
    remaining -= 2;
}

void EpdUpdate::finish() {
    if (remaining < 0)
        return;
    remaining = -1;

    epd.SendCommand(CMD_POWER_ON);
    epd.EPD_4IN01F_BusyHigh();
    epd.SendCommand(0x12);  // display refresh?
    epd.EPD_4IN01F_BusyHigh();
    epd.SendCommand(CMD_POWER_OFF);
    epd.EPD_4IN01F_BusyLow();
}

/* END OF FILE */
