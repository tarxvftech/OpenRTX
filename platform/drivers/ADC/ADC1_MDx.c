/***************************************************************************
 *   Copyright (C) 2020 by Silvano Seva IU2KWO and Niccolò Izzo IU2KIN     *
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
#include <stdlib.h>
#include "ADC1_MDx.h"


/*
 * The sample buffer is structured as follows:
 *
 * | vol | vbat | vox | rssi | sw1 | sw2 | rssi2 | htemp |
 *
 * NOTE: we are forced to allocate it through a malloc in order to make it be
 * in the "large" 128kB RAM. This because the linker script maps the .data and
 * .bss sections in the "small" 64kB CCM RAM, which cannot be reached by the DMA.
 */
uint16_t *sampleBuf = NULL;

#if defined(PLATFORM_MD9600)
static const size_t nChannels = 8;
#elif defined(PLATFORM_MD3x0)
static const size_t nChannels = 4;
#else
static const size_t nChannels = 2;
#endif

void adc1_init()
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    __DSB();

    sampleBuf = ((uint16_t *) malloc(4 * sizeof(uint16_t)));

    /*
     * Configure GPIOs to analog input mode:
     */
    gpio_setMode(AIN_VBAT,   INPUT_ANALOG);
    gpio_setMode(AIN_VOLUME, INPUT_ANALOG);
    #if defined(PLATFORM_MD3x0) || defined(PLATFORM_MD9600)
    gpio_setMode(AIN_MIC,   INPUT_ANALOG);
    gpio_setMode(AIN_RSSI,  INPUT_ANALOG);
    #if defined(PLATFORM_MD9600)
    gpio_setMode(AIN_SW2,   INPUT_ANALOG);
    gpio_setMode(AIN_SW1,   INPUT_ANALOG);
    gpio_setMode(AIN_RSSI2, INPUT_ANALOG);
    gpio_setMode(AIN_HTEMP, INPUT_ANALOG);
    #endif
    #endif

    /*
     * ADC clock is APB2 frequency divided by 8, giving 10.5MHz.
     * We set the sample time of each channel to 480 ADC cycles and we have to
     * scan four channels: given that a conversion takes 12 cycles, we have a
     * total conversion time of ~187us.
     */
    ADC->CCR |= ADC_CCR_ADCPRE;
    ADC1->SMPR2 = ADC_SMPR2_SMP0
                | ADC_SMPR2_SMP1
                | ADC_SMPR2_SMP3
                | ADC_SMPR2_SMP6
                | ADC_SMPR2_SMP7
                | ADC_SMPR2_SMP8
                | ADC_SMPR2_SMP9;
    ADC1->SMPR1 = ADC_SMPR1_SMP15;

    /*
     * No overrun interrupt, 12-bit resolution, no analog watchdog, no
     * discontinuous mode, enable scan mode, no end of conversion interrupts,
     * enable continuous conversion (free-running).
     */
    ADC1->CR1 |= ADC_CR1_SCAN;
    ADC1->CR2 |= ADC_CR2_DMA
              | ADC_CR2_DDS
              | ADC_CR2_CONT
              | ADC_CR2_ADON;

    /* Scan sequence config. */
    #if defined(PLATFORM_MD9600)
    ADC1->SQR1 = 7 << 20;    /* Eight channels to be converted          */
    ADC1->SQR3 |= (0 << 0)   /* CH0,  volume potentiometer level on PA0 */
               |  (1 << 5)   /* CH1,  battery voltage on PA1            */
               |  (3 << 10)  /* CH3,  vox level on PA3                  */
               |  (8 << 15)  /* CH8,  RSSI value on PB0                 */
               |  (7 << 20)  /* CH7,  SW1 value on PA7                  */
               |  (6 << 25); /* CH6,  SW2 value on PA6                  */
    ADC1->SQR2 |= (9  << 0)  /* CH9,  RSSI2 value on PB1                */
               |  (15 << 5); /* CH15, HTEMP value on PC5                */
    #elif defined(PLATFORM_MD3x0)
    ADC1->SQR1 = 3 << 20;    /* Four channels to be converted          */
    ADC1->SQR3 |= (0 << 0)   /* CH0, volume potentiometer level on PA0 */
               |  (1 << 5)   /* CH1, battery voltage on PA1            */
               |  (3 << 10)  /* CH3, vox level on PA3                  */
               |  (8 << 15); /* CH8, RSSI value on PB0                 */
    #else
    ADC1->SQR1 = 1 << 20;    /* Convert two channel                    */
    ADC1->SQR3 |= (0 << 0)   /* CH0, volume potentiometer level on PA0 */
               |  (1 << 5);  /* CH1, battery voltage on PA1            */
    #endif

    /* DMA2 Stream 0 configuration:
     * - channel 0: ADC1
     * - low priority
     * - half-word transfer, both memory and peripheral
     * - increment memory
     * - circular mode
     * - peripheral-to-memory transfer
     * - no interrupts
     */
    DMA2_Stream0->PAR = ((uint32_t) &(ADC1->DR));
    DMA2_Stream0->M0AR = ((uint32_t) sampleBuf);
    DMA2_Stream0->NDTR = nChannels;
    DMA2_Stream0->CR = DMA_SxCR_MSIZE_0     /* Memory size: 16 bit     */
                     | DMA_SxCR_PSIZE_0     /* Peripheral size: 16 bit */
                     | DMA_SxCR_PL_0        /* Medium priority         */
                     | DMA_SxCR_MINC        /* Increment memory        */
                     | DMA_SxCR_CIRC        /* Circular mode           */
                     | DMA_SxCR_EN;

    /* Finally, start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;
}

void adc1_terminate()
{
    free(sampleBuf);
    sampleBuf = NULL;
    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    ADC1->CR2 &= ~ADC_CR2_ADON;
    RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
    __DSB();
}

float adc1_getMeasurement(uint8_t ch)
{
    if((ch > (nChannels-1)) || (sampleBuf == NULL)) return 0.0f;

    float value = ((float) sampleBuf[ch]);
    return (value * 3300.0f)/4096.0f;
}
