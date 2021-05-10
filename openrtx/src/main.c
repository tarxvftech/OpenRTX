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

#include <ui.h>
#include <stdlib.h>
#include <inttypes.h>
#include <threads.h>
#include <interfaces/platform.h>
#include <battery.h>
#include <interfaces/graphics.h>
#include <interfaces/delays.h>
#include <hwconfig.h>

extern void *ui_task(void *arg);

int main(void)
{
    // MD-9600 does not have a proper power on/off mechanism and the MCU is
    // always powered on. We thus need to place a busy wait on the power on
    // button to manage the on/off mechanism.
    // A do-while block is used to avoid re-powering on after a power off due to
    // MCU rebooting before user stops pressing the power button.
    #ifdef PLATFORM_MD9600
    do
    {
        sleepFor(1, 0);
    }
    while(!platform_pwrButtonStatus());
    #endif

    // Initialize platform drivers
    platform_init();

    // Initialize display and graphics driver
    gfx_init();

    // Set default contrast
    display_setContrast(default_settings.contrast);

    // Initialize user interface
    ui_init();

    // Display splash screen
    ui_drawSplashScreen(true);
    gfx_render();

    // Wait 30ms before turning on backlight to hide random pixels on screen
    sleepFor(0u, 30u);
    platform_setBacklightLevel(255);

    // Keep the splash screen for 1 second
    sleepFor(1u, 0u);

    // Create OpenRTX threads
    create_threads();

    // Jump to the UI task
    ui_task(NULL);
}
