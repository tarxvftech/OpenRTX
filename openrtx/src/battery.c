/***************************************************************************
 *   Copyright (C) 2020 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN,                            *
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

#include <battery.h>
#include <hwconfig.h>
#include <math.h>

/* This array acts as a lookup table for converting Li-Po voltage into
 * charge percentage, elements range from 5% to 95% (included) with 5% steps.
 * Data is taken from (https://blog.ampow.com/lipo-voltage-chart/).
 */
#define V_LUT_STEPS 21
#if defined BAT_LIPO_1S
float bat_v_min = 3.61f;
float bat_v_max = 4.15f;
#elif defined BAT_LIPO_2S
float bat_v_min = 7.10f;
float bat_v_max = 8.10f;
#elif defined BAT_LIPO_3S
float bat_v_min = 10.83;
float bat_v_max = 12.45;
#elif defined BAT_NONE
float bat_v_min = 0.0;
float bat_v_max = 0.0;
#else
#error Please define a battery type into platform/targets/.../hwconfig.h
#endif

float battery_getCharge(float vbat)
{
    #ifndef BAT_NONE
    // Perform a linear interpolation between minimum and maximum charge values.
    return (vbat - bat_v_min) / (bat_v_max - bat_v_min);
    #else
    // Return full charge if no battery is present.
    (void) vbat;
    return 1.0f;
    #endif
}
