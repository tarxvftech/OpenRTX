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
#include <stdint.h>
#include <ui.h>
#include <string.h>

extern const rtxStatus_t rtxStatus;

void _ui_drawMainBackground()
{
    // Print top bar line of hline_h pixel height
    gfx_drawHLine(layout.top_h, layout.hline_h, color_grey);
    // Print bottom bar line of 1 pixel height
    gfx_drawHLine(SCREEN_HEIGHT - layout.bottom_h - 1, layout.hline_h, color_grey);
}

void _ui_drawMainTop()
{
#ifdef HAS_RTC
    // Print clock on top bar
    curTime_t local_time = state_getLocalTime(last_state.time);
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER,
              color_white, "%02d:%02d:%02d", local_time.hour,
              local_time.minute, local_time.second);
    color_t red = {0xff,0x00,0x00, 0xff};
#endif
    // If the radio has no built-in battery, print input voltage
#ifdef BAT_NONE
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_RIGHT,
              color_white,"%.1fV", last_state.v_bat);
#else
    // Otherwise print battery icon on top bar, use 4 px padding
    uint16_t bat_width = SCREEN_WIDTH / 9;
    uint16_t bat_height = layout.top_h - (layout.status_v_pad * 2);
    point_t bat_pos = {SCREEN_WIDTH - bat_width - layout.horizontal_pad,
                       layout.status_v_pad};
    gfx_drawBattery(bat_pos, bat_width, bat_height, last_state.charge);
#endif
    // Print radio mode on top bar
    switch(last_state.channel.mode)
    {
        case FM:
        gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, "FM");
        break;
        case DMR:
        gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, "DMR");
        break;
    }

    int left_offset = 12;
    int right_offset = 40;
    int top_line_height = 7;
    point_t top0 = {layout.top_pos.x, top_line_height};
    point_t top1 = {layout.horizontal_pad+left_offset, top_line_height};
    point_t top2 = {layout.horizontal_pad+left_offset, 2*top_line_height};
    point_t top3 = {SCREEN_WIDTH-layout.horizontal_pad-right_offset, top_line_height};
    point_t top4 = {SCREEN_WIDTH-layout.horizontal_pad-right_offset, 2*top_line_height};

    /*float power;                [>*< Transmission power, in watt              <]*/
    /*freq_t rx_frequency;        [>*< RX Frequency, in Hz                      <]*/
    /*freq_t tx_frequency;        [>*< TX Frequency, in Hz                      <]*/
    /*char name[16];              [>*< Channel name                             <]*/
    /*union*/
    /*{*/
        /*fmInfo_t  fm;           [>*< Information block for FM channels        <]*/
        /*dmrInfo_t dmr;          [>*< Information block for DMR channels       <]*/
    /*};*/
    uint8_t transmitting = rtxStatus.opStatus == TX;
    char tmp[10] = {0};
    snprintf(tmp, 10, "%.0fW", last_state.channel.power);
    gfx_print(top0, tmp, FONT_SIZE_5PT, TEXT_ALIGN_LEFT, transmitting? color_red:color_white);
    snprintf(tmp, 10, "%.4f", ((float)rtxStatus.rxFrequency)/1000000);
    gfx_print(top1, tmp, FONT_SIZE_5PT, TEXT_ALIGN_LEFT, transmitting?color_grey:color_white);
    snprintf(tmp, 10, "%.4f", ((float)rtxStatus.txFrequency)/1000000);
    gfx_print(top2, tmp, FONT_SIZE_5PT, TEXT_ALIGN_LEFT, transmitting?color_white:color_grey);
    if( transmitting && rtxStatus.txToneEn ){
        gfx_print(top3, "CTCSS", FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_red);
        snprintf(tmp, 10, "%.1f", ((float)rtxStatus.rxTone)/10);
        gfx_print(top4, tmp, FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_white);
    } else if( ! transmitting && rtxStatus.rxToneEn ){
        gfx_print(top3, "CTCSS", FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_white);
        snprintf(tmp, 10, "%.1f", ((float)rtxStatus.rxTone)/10);
        gfx_print(top4, tmp, FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_white);
    }
}

void _ui_drawZoneChannel()
{
    // Print Zone name
    if(!last_state.zone_enabled)
        gfx_print(layout.line1_pos, layout.line1_font, TEXT_ALIGN_LEFT,
                  color_white, "zone: All channels");
    else
        gfx_print(layout.line1_pos, layout.line1_font, TEXT_ALIGN_LEFT,
                  color_white,  "zone: %.13s", last_state.zone.name);
    // Print Channel name
    gfx_print(layout.line2_pos, layout.line2_font, TEXT_ALIGN_LEFT,
              color_white, "  %03d: %.12s",
              last_state.channel_index, last_state.channel.name);
}

void _ui_drawFrequency()
{
    // Print big numbers frequency
    gfx_print(layout.line3_pos, layout.line3_font, TEXT_ALIGN_CENTER,
              color_white, "%03lu.%05lu",
              (unsigned long)last_state.channel.rx_frequency/1000000,
              (unsigned long)last_state.channel.rx_frequency%1000000/10);
}

void _ui_drawVFOMiddleInput(ui_state_t* ui_state)
{
    // Add inserted number to string, skipping "Rx: "/"Tx: " and "."
    uint8_t insert_pos = ui_state->input_position + 3;
    if(ui_state->input_position > 3) insert_pos += 1;
    char input_char = ui_state->input_number + '0';

    if(ui_state->input_set == SET_RX)
    {
        if(ui_state->input_position == 0)
        {
            gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ">Rx:%03lu.%05lu",
                      (unsigned long)ui_state->new_rx_frequency/1000000,
                      (unsigned long)ui_state->new_rx_frequency%1000000/10);
        }
        else
        {
            // Replace Rx frequency with underscorses
            if(ui_state->input_position == 1)
                strcpy(ui_state->new_rx_freq_buf, ">Rx:___._____");
            ui_state->new_rx_freq_buf[insert_pos] = input_char;
            gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ui_state->new_rx_freq_buf);
        }
        gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
                  color_white, " Tx:%03lu.%05lu",
                  (unsigned long)last_state.channel.tx_frequency/1000000,
                  (unsigned long)last_state.channel.tx_frequency%1000000/10);
    }
    else if(ui_state->input_set == SET_TX)
    {
        gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
                  color_white, " Rx:%03lu.%05lu",
                  (unsigned long)ui_state->new_rx_frequency/1000000,
                  (unsigned long)ui_state->new_rx_frequency%1000000/10);
        // Replace Rx frequency with underscorses
        if(ui_state->input_position == 0)
        {
            gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ">Tx:%03lu.%05lu",
                      (unsigned long)ui_state->new_rx_frequency/1000000,
                      (unsigned long)ui_state->new_rx_frequency%1000000/10);
        }
        else
        {
            if(ui_state->input_position == 1)
                strcpy(ui_state->new_tx_freq_buf, ">Tx:___._____");
            ui_state->new_tx_freq_buf[insert_pos] = input_char;
            gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ui_state->new_tx_freq_buf);
        }
    }
}

void _ui_drawBottom()
{
    // Squelch bar
    float rssi = last_state.rssi;
    float squelch = last_state.sqlLevel / 16.0f;
    point_t smeter_pos = { layout.horizontal_pad,
                           layout.bottom_pos.y +
                           layout.status_v_pad +
                           layout.text_v_offset -
                           layout.bottom_h };
    gfx_drawSmeter(smeter_pos,
                   SCREEN_WIDTH - 2 * layout.horizontal_pad,
                   layout.bottom_h - 1,
                   rssi,
                   squelch,
                   color_white);
}

void _ui_drawMainVFO()
{
    gfx_clearScreen();
    _ui_drawMainTop();
    _ui_drawFrequency();
    _ui_drawBottom();
}

void _ui_drawMainVFOInput(ui_state_t* ui_state)
{
    gfx_clearScreen();
    _ui_drawMainTop();
    _ui_drawVFOMiddleInput(ui_state);
    _ui_drawBottom();
}

void _ui_drawMainMEM()
{
    gfx_clearScreen();
    _ui_drawMainTop();
    _ui_drawZoneChannel();
    _ui_drawFrequency();
    _ui_drawBottom();
}
