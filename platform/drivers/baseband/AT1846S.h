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

#ifndef AT1846S_H
#define AT1846S_H

#include <stdint.h>
#include <stdbool.h>
#include <datatypes.h>

typedef enum
{
    AT1846S_BW_12P5 = 0,
    AT1846S_BW_25   = 1
}
AT1846S_bw_t;

typedef enum
{
    AT1846S_OP_FM  = 0,
    AT1846S_OP_DMR = 1
}
AT1846S_op_t;

typedef enum
{
    AT1846S_OFF = 0,
    AT1846S_RX  = 1,
    AT1846S_TX  = 2,
}
AT1846S_func_t;

/**
 * 
 */
void AT1846S_init();

/**
 * 
 */
void AT1846S_postInit();

/**
 * 
 */
void AT1846S_setFrequency(const freq_t freq);

/**
 * 
 */
void AT1846S_setBandwidth(AT1846S_bw_t band);

/**
 * 
 */
void AT1846S_setOpMode(AT1846S_op_t mode);

/**
 * 
 */
void AT1846S_setFuncMode(AT1846S_func_t mode);

/**
 * 
 */
uint16_t AT1846S_readRSSI();

/**
 * 
 */
void AT1846S_setPgaGain(uint8_t gain);

/**
 * 
 */
void AT1846S_setMicGain(uint8_t gain);


/**
 * 
 */
void AT1846S_setTxDeviation(uint8_t dev);

/**
 * 
 */
void AT1846S_setAgcGain(uint8_t gain);

/**
 * 
 */
void AT1846S_setRxAudioGain(uint8_t gainWb, uint8_t gainNb);

/**
 * 
 */
void AT1846S_setPaDrive(uint8_t value);

/**
 * 
 */
void AT1846S_setAnalogSqlThresh(uint8_t thresh);

#endif /* AT1846S_H */
