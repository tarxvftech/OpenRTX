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

#include <stdio.h>
#include <interfaces/nvmem.h>

// Simulate CPS with 16 channels, 16 zones, 16 contacts
const uint32_t maxNumZones = 16;
const uint32_t maxNumChannels = 16;
const uint32_t maxNumContacts = 16;
const freq_t dummy_base_freq = 145500000;

void nvm_init()
{
}

void nvm_terminate()
{
}

int nvm_readChannelData(channel_t *channel, uint16_t pos)
{
    if(pos >= maxNumChannels) return -1;

    /* Generate dummy channel name */
    snprintf(channel->name, 16, "Channel %d", pos);
    /* Generate dummy frequency values */
    channel->rx_frequency = dummy_base_freq + pos * 100000;
    channel->tx_frequency = dummy_base_freq + pos * 100000;

    return 0;
}

int nvm_readZoneData(zone_t *zone, uint16_t pos)
{
    if(pos >= maxNumZones) return -1;

    /* Generate dummy zone name */
    snprintf(zone->name, 16, "Zone %d", pos);

    return 0;
}

int nvm_readContactData(contact_t *contact, uint16_t pos)
{
    if(pos >= maxNumContacts) return -1;

    /* Generate dummy contact name */
    snprintf(contact->name, 16, "Contact %d", pos);

    return 0;
}

