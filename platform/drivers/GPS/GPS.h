/***************************************************************************
 *   Copyright (C) 2021 by Federico Amedeo Izzo IU2NUO,                    *
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

#ifndef GPS_H
#define GPS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Low-level driver for interfacing with radio's on-board GPS module.
 */

/**
 * Initialise the GPS driver.
 * This function does not turn on the GPS module.
 * @param baud: baud rate of GPS serial interface.
 */
void gps_init(const uint16_t baud);

/**
 * Shut down the GPS module and terminate the GPS driver.
 */
void gps_terminate();

/**
 * Turn on the GPS module.
 */
void gps_enable();

/**
 * Turn off GPS module.
 */
void gps_disable();

/**
 * Detect if a GPS module is present in the system, it can be called also
 * when driver is not initialised.
 * @param timeout: timeout for GPS detection, in milliseconds.
 * @return true if a GPS module is present, false otherwise.
 */
bool gps_detect(uint16_t timeout);

/**
 * Read a NMEA sentence from the GPS module, blocking function.
 * @param buf: buffer to which the NMEA sentence is written.
 * @param maxLength: maximum writable length inside the buffer.
 * @return number of characters written in the buffer or -1 on error.
 */
int gps_getNmeaSentence(char *buf, const size_t maxLength);

#endif /* GPS_H */
