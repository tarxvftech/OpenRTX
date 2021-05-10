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

#include <interfaces/gpio.h>
#include <hwconfig.h>
#include <stdint.h>

/*
 * Implementation of external flash SPI interface for MDx devices.
 */

uint8_t spiFlash_SendRecv(uint8_t val)
{
    SPI1->DR = val;
    while((SPI1->SR & SPI_SR_RXNE) == 0) ;
    return SPI1->DR;
}

void spiFlash_init()
{
    gpio_setMode(FLASH_CLK, ALTERNATE);
    gpio_setMode(FLASH_SDO, ALTERNATE);
    gpio_setMode(FLASH_SDI, ALTERNATE);
    gpio_setAlternateFunction(FLASH_CLK, 5); /* SPI1 is on AF5 */
    gpio_setAlternateFunction(FLASH_SDO, 5);
    gpio_setAlternateFunction(FLASH_SDI, 5);

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    __DSB();

    SPI1->CR1 = SPI_CR1_SSM     /* Software managment of nCS */
              | SPI_CR1_SSI     /* Force internal nCS        */
              | SPI_CR1_BR_2    /* Fclock: 84MHz/64 = 1.3MHz */
              | SPI_CR1_BR_0
              | SPI_CR1_MSTR    /* Master mode               */
              | SPI_CR1_SPE;    /* Enable peripheral         */
}

void spiFlash_terminate()
{
    RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
    __DSB();

    gpio_setMode(FLASH_CLK, INPUT);
    gpio_setMode(FLASH_SDO, INPUT);
    gpio_setMode(FLASH_SDI, INPUT);
}
