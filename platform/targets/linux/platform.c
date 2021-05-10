/***************************************************************************
 *   Copyright (C) 2020 by Frederik Saraci IU2NRO                          *
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

#include <interfaces/platform.h>
#include <interfaces/gpio.h>
#include <stdio.h>
#include "emulator.h"

hwInfo_t hwInfo;

void platform_init()
{
    //printf("Platform init\n");
    // Fill hwinfo struct
    snprintf(hwInfo.name, 10, "Linux");
    hwInfo.vhf_band = 1;
    hwInfo.uhf_band = 1;

    emulator_start();
}

void platform_terminate()
{
    printf("Platform terminate\n");
}

void platform_setBacklightLevel(__attribute__((unused)) uint8_t level)
{
    //printf("platform_setBacklightLevel(%u)\n", level);
}

// Simulate a fully charged lithium battery
float platform_getVbat()
{
    return Radio_State.Vbat;
}


float platform_getMicLevel()
{
    return Radio_State.micLevel;
}


float platform_getVolumeLevel()
{
    return Radio_State.volumeLevel;
}


int8_t platform_getChSelector()
{
    return Radio_State.chSelector;
}


bool platform_getPttStatus()
{
    return Radio_State.PttStatus;
}

bool platform_pwrButtonStatus()
{
    /* Suppose radio is always on */
    return true;
}

void platform_ledOn(__attribute__((unused)) led_t led)
{
    // Commented to reduce verbosity on Linux
    //char* str;
    //switch(led)
    //{
    //    case 0:
    //        str = "GREEN";
    //        break;
    //    case 1:
    //        str = "RED";
    //        break;
    //    case 2:
    //        str = "YELLOW";
    //        break;
    //    case 3:
    //        str = "WHITE";
    //        break;
    //}
    //printf("platform_ledOn(%s)\n", str);
}


void platform_ledOff(__attribute__((unused)) led_t led)
{
    // Commented to reduce verbosity on Linux
    //printf("platform_ledOff()\n");
}


void platform_beepStart(uint16_t freq)
{
    printf("platform_beepStart(%u)\n", freq);
}


void platform_beepStop()
{
    printf("platform_beepStop()\n");
}

const void *platform_getCalibrationData()
{
    return NULL;
}

const hwInfo_t *platform_getHwInfo()
{
    return &hwInfo;
}
