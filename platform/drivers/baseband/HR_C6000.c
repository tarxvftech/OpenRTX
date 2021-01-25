/***************************************************************************
 *   Copyright (C) 2020 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN                             *
 *                         Frederik Saraci IU2NRO                          *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "HR_C6000.h"
#include "interfaces.h"
#include <hwconfig.h>
#include <interfaces/gpio.h>
#include <interfaces/delays.h>
#include <hwconfig.h>
#include <os.h>
#include <calibUtils.h>

#include <stdio.h>

static const uint8_t initSeq1[] = { 0x01, 0x04, 0xD5, 0xD7, 0xF7, 0x7F, 0xD7, 0x57 };
static const uint8_t initSeq2[] =
{
    0x01, 0x10, 0x69, 0x69, 0x96, 0x96, 0x96, 0x99, 0x99, 0x99, 0xA5, 0xA5, 0xAA,
    0xAA, 0xCC, 0xCC, 0x00, 0xF0, 0x01, 0xFF, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t initSeq3[] =
{
    0x01, 0x30, 0x00, 0x00, 0x14, 0x1E, 0x1A, 0xFF, 0x3D, 0x50, 0x07, 0x60, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t initSeq4[] = { 0x01, 0x40, 0x00, 0x03, 0x01, 0x02, 0x05, 0x1E, 0xF0 };
static const uint8_t initSeq5[] = { 0x01, 0x51, 0x00, 0x00, 0xEB, 0x78, 0x67 };
static const uint8_t initSeq6[] =
{
    0x01, 0x60, 0x32, 0xEF, 0x00, 0x31, 0xEF, 0x00, 0x12, 0xEF, 0x00, 0x13, 0xEF,
    0x00, 0x14, 0xEF, 0x00, 0x15, 0xEF, 0x00, 0x16, 0xEF, 0x00, 0x17, 0xEF, 0x00,
    0x18, 0xEF, 0x00, 0x19, 0xEF, 0x00, 0x1A, 0xEF, 0x00, 0x1B, 0xEF, 0x00, 0x1C,
    0xEF, 0x00, 0x1D, 0xEF, 0x00, 0x1E, 0xEF, 0x00, 0x1F, 0xEF, 0x00, 0x20, 0xEF,
    0x00, 0x21, 0xEF, 0x00, 0x22, 0xEF, 0x00, 0x23, 0xEF, 0x00, 0x24, 0xEF, 0x00,
    0x25, 0xEF, 0x00, 0x26, 0xEF, 0x00, 0x27, 0xEF, 0x00, 0x28, 0xEF, 0x00, 0x29,
    0xEF, 0x00, 0x2A, 0xEF, 0x00, 0x2B, 0xEF, 0x00, 0x2C, 0xEF, 0x00, 0x2D, 0xEF,
    0x00, 0x2E, 0xEF, 0x00, 0x2F, 0xEF, 0x00
};

void _writeReg(uint8_t type, uint8_t reg, uint8_t val)
{
    gpio_clearPin(DMR_CS);
    (void) uSpi_sendRecv(type);
    (void) uSpi_sendRecv(reg);
    (void) uSpi_sendRecv(val);
    delayUs(2);
    gpio_setPin(DMR_CS);
    delayUs(2);
}

uint8_t _readReg(uint8_t type, uint8_t reg)
{
    gpio_clearPin(DMR_CS);
    (void) uSpi_sendRecv(type);
    (void) uSpi_sendRecv(reg);
    uint8_t val = uSpi_sendRecv(0xFF);
    delayUs(2);
    gpio_setPin(DMR_CS);
    delayUs(2);

    return val;
}

void _sendSequence(const uint8_t *seq, uint8_t len)
{
    gpio_clearPin(DMR_CS);

    uint8_t i = 0;
    for(; i < len; i++)
    {
        (void) uSpi_sendRecv(seq[i]);
    }

   delayUs(2);
   gpio_setPin(DMR_CS);
   delayUs(2);
}

void C6000_init()
{

    uSpi_init();

    gpio_setMode(DMR_CS,    OUTPUT);
    gpio_setMode(DMR_SLEEP, OUTPUT);
    gpio_setMode(DMR_RESET, OUTPUT);

    gpio_setPin(DMR_RESET);
    gpio_setPin(DMR_SLEEP);

    delayMs(10);
    gpio_clearPin(DMR_SLEEP);         // Exit from sleep pulling down DMR_SLEEP
    delayMs(10);

    _writeReg(0x04, 0x0a, 0x81);    //Clock connected to crystal
    _writeReg(0x04, 0x0b, 0x40);    //Set PLL M Register
    _writeReg(0x04, 0x0c, 0x32);    //Set PLL Dividers
    _writeReg(0x04, 0xb9, 0x05);
    _writeReg(0x04, 0x0a, 0x01);    //Clock connected to PLL, set Clock Source Enable CLKOUT Pin

//*
    _sendSequence(initSeq1, sizeof(initSeq1));
    _sendSequence(initSeq2, sizeof(initSeq2));
    _sendSequence(initSeq3, sizeof(initSeq3));
    _sendSequence(initSeq4, sizeof(initSeq4));
    _sendSequence(initSeq5, sizeof(initSeq5));
    _sendSequence(initSeq6, sizeof(initSeq6));
 
    _writeReg(0x04, 0x00, 0x00);   //Clear all Reset Bits which forces a reset of all internal systems
    _writeReg(0x04, 0x10, 0x6E);   //Set DMR,Tier2,Timeslot Mode, Layer 2, Repeater, Aligned, Slot1
    _writeReg(0x04, 0x11, 0x80);   //Set LocalChanMode to Default Value 
    _writeReg(0x04, 0x13, 0x00);   //Zero Cend_Band Timing advance
    _writeReg(0x04, 0x1F, 0x10);   //Set LocalEMB  DMR Colour code in upper 4 bits - defaulted to 1, and is updated elsewhere in the code
    _writeReg(0x04, 0x20, 0x00);   //Set LocalAccessPolicy to Impolite
    _writeReg(0x04, 0x21, 0xA0);   //Set LocalAccessPolicy1 to Polite to Color Code  (unsure why there are two registers for this)   
    _writeReg(0x04, 0x22, 0x26);   //Start Vocoder Decode, I2S mode
    _writeReg(0x04, 0x22, 0x86);   //Start Vocoder Encode, I2S mode
    _writeReg(0x04, 0x25, 0x0E);   //Undocumented Register 
    _writeReg(0x04, 0x26, 0x7D);   //Undocumented Register 
    _writeReg(0x04, 0x27, 0x40);   //Undocumented Register 
    _writeReg(0x04, 0x28, 0x7D);   //Undocumented Register
    _writeReg(0x04, 0x29, 0x40);   //Undocumented Register
    _writeReg(0x04, 0x2A, 0x0B);   //Set spi_clk_cnt to default value
    _writeReg(0x04, 0x2B, 0x0B);   //According to Datasheet this is a Read only register For FM Squelch
    _writeReg(0x04, 0x2C, 0x17);   //According to Datasheet this is a Read only register For FM Squelch
    _writeReg(0x04, 0x2D, 0x05);   //Set FM Compression and Decompression points (?)
    _writeReg(0x04, 0x2E, 0x04);   //Set tx_pre_on (DMR Transmission advance) to 400us
    _writeReg(0x04, 0x2F, 0x0B);   //Set I2S Clock Frequency
    _writeReg(0x04, 0x32, 0x02);   //Set LRCK_CNT_H CODEC Operating Frequency to default value
    _writeReg(0x04, 0x33, 0xFF);   //Set LRCK_CNT_L CODEC Operating Frequency to default value
    _writeReg(0x04, 0x34, 0xF0);   //Set FM Filters on and bandwidth to 12.5Khz 
    _writeReg(0x04, 0x35, 0x28);   //Set FM Modulation Coefficient
    _writeReg(0x04, 0x3E, 0x28);   //Set FM Modulation Offset
    _writeReg(0x04, 0x3F, 0x10);   //Set FM Modulation Limiter
    _writeReg(0x04, 0x36, 0x00);   //Enable all clocks
    _writeReg(0x04, 0x37, 0x00);   //Set mcu_control_shift to default. (codec under HRC-6000 control)
    _writeReg(0x04, 0x4B, 0x1B);   //Set Data packet types to defaults
    _writeReg(0x04, 0x4C, 0x00);   //Set Data packet types to defaults
    _writeReg(0x04, 0x56, 0x00);   //Undocumented Register
    _writeReg(0x04, 0x5F, 0xC0);   //Enable Sync detection for MS or BS orignated signals
    _writeReg(0x04, 0x81, 0xFF);   //Enable all Interrupts
    _writeReg(0x04, 0xD1, 0xC4);   //According to Datasheet this register is for FM DTMF (?)


    _writeReg(0x04, 0x01, 0x70);     //set 2 point Mod, swap receive I and Q, receive mode IF (?)    (Presumably changed elsewhere)
    _writeReg(0x04, 0x03, 0x00);   //zero Receive I Offset
    _writeReg(0x04, 0x05, 0x00);   //Zero Receive Q Offset
    _writeReg(0x04, 0x12, 0x15);     //Set rf_pre_on Receive to transmit switching advance 
    _writeReg(0x04, 0xA1, 0x80);     //According to Datasheet this register is for FM Modulation Setting (?)
    _writeReg(0x04, 0xC0, 0x0A);   //Set RF Signal Advance to 1ms (10x100us)
    _writeReg(0x04, 0x06, 0x21);   //Use SPI vocoder under MCU control
    _writeReg(0x04, 0x07, 0x0B);   //Set IF Frequency H to default 450KHz
    _writeReg(0x04, 0x08, 0xB8);   //Set IF Frequency M to default 450KHz
    _writeReg(0x04, 0x09, 0x00);   //Set IF Frequency L to default 450KHz
    _writeReg(0x04, 0x0D, 0x10);   //Set Voice Superframe timeout value
    _writeReg(0x04, 0x0E, 0x8E);   //Register Documented as Reserved 
    _writeReg(0x04, 0x0F, 0xB8);   //FSK Error Count
    _writeReg(0x04, 0xC2, 0x00);   //Disable Mic Gain AGC
    _writeReg(0x04, 0xE0, 0x8B);   //CODEC under MCU Control, LineOut2 Enabled, Mic_p Enabled, I2S Slave Mode
    _writeReg(0x04, 0xE1, 0x0F);   //Undocumented Register (Probably associated with CODEC)
    _writeReg(0x04, 0xE2, 0x06);   //CODEC  Anti Pop Enabled, DAC Output Enabled
    _writeReg(0x04, 0xE3, 0x52);   //CODEC Default Settings 
    _writeReg(0x04, 0xE4, 0x4A);   //CODEC   LineOut Gain 2dB, Mic Stage 1 Gain 0dB, Mic Stage 2 Gain 30dB
    _writeReg(0x04, 0xE5, 0x1A);   //CODEC Default Setting


    _writeReg(0x04, 0x40, 0xC3);      //Enable DMR Tx, DMR Rx, Passive Timing, Normal mode
    _writeReg(0x04, 0x41, 0x40);   //Receive during next timeslot

    _writeReg(0x04, 0x06, 0x23); // SET OpenMusic bit (play Boot sound and Call Prompts)

    gpio_clearPin(DMR_CS);
    (void) uSpi_sendRecv(0x03);
    (void) uSpi_sendRecv(0x00);
    for(uint8_t i = 0; i < 128; i++) uSpi_sendRecv(0xAA);
    delayUs(2);
    gpio_setPin(DMR_CS);
    delayUs(2);

    _writeReg(0x04, 0x06, 0x21); // CLEAR OpenMusic bit (play Boot sound and Call Prompts)

    _writeReg(0x04, 0x37, 0x9E); // MCU take control of CODEC
    _writeReg(0x04, 0xE4, 0x0A);
//     SPI0SeClearPageRegByteWithMask(0x04, 0xE4, 0x3F, 0x00); // Set CODEC LineOut Gain to 0dB

//     dmrMonitorCapturedTimeout = nonVolatileSettings.dmrCaptureTimeout * 1000;

   _writeReg(0x04, 0x04, 0xE8);  //Set Mod2 output offset
   _writeReg(0x04, 0x46, 0x37);  //Set Mod1 Amplitude
   _writeReg(0x04, 0x48, 0x03);  //Set 2 Point Mod Bias
   _writeReg(0x04, 0x47, 0xE8);  //Set 2 Point Mod Bias

   _writeReg(0x04, 0x41, 0x20);  //set sync fail bit (reset?)
   _writeReg(0x04, 0x40, 0x03);  //Disable DMR Tx and Rx
   _writeReg(0x04, 0x41, 0x00);  //Reset all bits.
   _writeReg(0x04, 0x00, 0x3F);  //Reset DMR Protocol and Physical layer modules.
   _sendSequence(initSeq1, sizeof(initSeq1));
   _writeReg(0x04, 0x10, 0x6E);  //Set DMR, Tier2, Timeslot mode, Layer2, Repeater, Aligned, Slot 1
   _writeReg(0x04, 0x1F, 0x10);  // Set Local EMB. DMR Colour code in upper 4 bits - defaulted to 1, and is updated elsewhere in the code
   _writeReg(0x04, 0x26, 0x7D);  //Undocumented Register 
   _writeReg(0x04, 0x27, 0x40);  //Undocumented Register 
   _writeReg(0x04, 0x28, 0x7D);  //Undocumented Register 
   _writeReg(0x04, 0x29, 0x40);  //Undocumented Register 
   _writeReg(0x04, 0x2A, 0x0B);  //Set SPI Clock to default value
   _writeReg(0x04, 0x2B, 0x0B);  //According to Datasheet this is a Read only register For FM Squelch
   _writeReg(0x04, 0x2C, 0x17);  //According to Datasheet this is a Read only register For FM Squelch
   _writeReg(0x04, 0x2D, 0x05);  //Set FM Compression and Decompression points (?)
   _writeReg(0x04, 0x56, 0x00);  //Undocumented Register
   _writeReg(0x04, 0x5F, 0xC0);  //Enable Sync detection for MS or BS orignated signals
   _writeReg(0x04, 0x81, 0xFF);  //Enable all Interrupts
   _writeReg(0x04, 0x01, 0x70);  //Set 2 Point Mod, Swap Rx I and Q, Rx Mode IF
   _writeReg(0x04, 0x03, 0x00);  //Zero Receive I Offset
   _writeReg(0x04, 0x05, 0x00);  //Zero Receive Q Offset
   _writeReg(0x04, 0x12, 0x15);  //Set RF Switching Receive to Transmit Advance
   _writeReg(0x04, 0xA1, 0x80);  //According to Datasheet this register is for FM Modulation Setting (?)
   _writeReg(0x04, 0xC0, 0x0A);  //Set RF Signal Advance to 1ms (10x100us)
   _writeReg(0x04, 0x06, 0x21);  //Use SPI vocoder under MCU control
   _writeReg(0x04, 0x07, 0x0B);  //Set IF Frequency H to default 450KHz
   _writeReg(0x04, 0x08, 0xB8);  //Set IF Frequency M to default 450KHz
   _writeReg(0x04, 0x09, 0x00);  //Set IF Frequency l to default 450KHz
   _writeReg(0x04, 0x0D, 0x10);  //Set Voice Superframe timeout value
   _writeReg(0x04, 0x0E, 0x8E);  //Register Documented as Reserved 
   _writeReg(0x04, 0x0F, 0xB8);  //FSK Error Count
   _writeReg(0x04, 0xC2, 0x00);  //Disable Mic Gain AGC
   _writeReg(0x04, 0xE0, 0x8B);  //CODEC under MCU Control, LineOut2 Enabled, Mic_p Enabled, I2S Slave Mode
   _writeReg(0x04, 0xE1, 0x0F);  //Undocumented Register (Probably associated with CODEC)
   _writeReg(0x04, 0xE2, 0x06);  //CODEC  Anti Pop Enabled, DAC Output Enabled
   _writeReg(0x04, 0xE3, 0x52);  //CODEC Default Settings

   _writeReg(0x04, 0xE5, 0x1A);  //CODEC Default Setting
   _writeReg(0x04, 0x26, 0x7D);  //Undocumented Register
   _writeReg(0x04, 0x27, 0x40);  //Undocumented Register
   _writeReg(0x04, 0x28, 0x7D);  //Undocumented Register
   _writeReg(0x04, 0x29, 0x40);  //Undocumented Register
   _writeReg(0x04, 0x41, 0x20);  //Set Sync Fail Bit  (Reset?)
   _writeReg(0x04, 0x40, 0xC3);  //Enable DMR Tx and Rx, Passive Timing
   _writeReg(0x04, 0x41, 0x40);  //Set Receive During Next Slot Bit
   _writeReg(0x04, 0x01, 0x70);  //Set 2 Point Mod, Swap Rx I and Q, Rx Mode IF
   _writeReg(0x04, 0x10, 0x6E);  //Set DMR, Tier2, Timeslot mode, Layer2, Repeater, Aligned, Slot 1
   _writeReg(0x04, 0x00, 0x3F);  //Reset DMR Protocol and Physical layer modules.
   _writeReg(0x04, 0xE4, 0xCB); //CODEC   LineOut Gain 6dB, Mic Stage 1 Gain 0dB, Mic Stage 2 Gain default is 11 =  33dB
   
   _writeReg(0x04, 0x06, 0x23);

//*/
}

void C6000_terminate()
{
    gpio_setPin(DMR_SLEEP);
    gpio_setMode(DMR_CS, INPUT);
}

void C6000_setModOffset(uint16_t offset)
{
    uint8_t offUpper = (offset >> 8) & 0x03;
    uint8_t offLower = offset & 0xFF;

    _writeReg(0x04, 0x48, offUpper);
    _writeReg(0x04, 0x47, offLower);
}

void C6000_setMod1Amplitude(uint8_t amplitude)
{
    _writeReg(0x04, 0x46, amplitude);
}

void C6000_setMod2Bias(uint8_t bias)
{
    _writeReg(0x04, 0x04, bias);
}

void C6000_setDacRange(uint8_t value)
{
    uint8_t dacData = value + 1;
    if(dacData > 31) dacData = 31;
    _writeReg(0x04, 0x37, dacData);
}

bool C6000_spiInUse()
{
    return (gpio_readPin(DMR_CS) == 0) ? true :  false;
}
