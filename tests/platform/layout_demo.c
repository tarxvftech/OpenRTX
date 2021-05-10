/***************************************************************************
 *   Copyright (C) 2020 by Federico Izzo IU2NUO, Niccolò Izzo IU2KIN and   *
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

#include <stdint.h>
#include <stdlib.h>
#include <interfaces/graphics.h>
#include <interfaces/platform.h>
#include <interfaces/delays.h>
#include "ui.h"


int main(void)
{
    platform_init();
    // Init the graphic stack
    gfx_init();
    gfx_clearScreen();
    gfx_render();
    platform_setBacklightLevel(255);
    
    while(1)
    {
        for(int tot=1; tot<=10; tot++)
        {
            gfx_clearScreen();
            for(int cur=1; cur<=tot; cur++)
            {
                gfx_printLine(cur, tot, 0, 0, 0, FONT_SIZE_8PT, TEXT_ALIGN_CENTER,
                              color_white, "Line %2d of %2d", cur, tot);
            }
            gfx_render();
            // Sleep for 1 second
            sleepFor(1u, 0u);
        }
    }
}
