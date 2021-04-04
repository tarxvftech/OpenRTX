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
#include <string.h>
#include <ui.h>
#include <interfaces/nvmem.h>
#include <interfaces/platform.h>

#include <maidenhead.h>
extern void _ui_drawMainBackground();
extern void _ui_drawMainTop();
extern void _ui_drawBottom();

#include <math.h>
#include <stdarg.h>
#define PI  3.141592653589793238462643383279
extern double curTime_to_julian_day(curTime_t t);

void _ui_drawMenuList(uint8_t selected, int (*getCurrentEntry)(char *buf, uint8_t max_len, uint8_t index))
{
    point_t pos = layout.line1_pos;
    // Number of menu entries that fit in the screen height
    uint8_t entries_in_screen = (SCREEN_HEIGHT - 1 - pos.y) / layout.menu_h + 1;
    uint8_t scroll = 0;
    char entry_buf[MAX_ENTRY_LEN] = "";
    color_t text_color = color_white;
    for(int item=0, result=0; (result == 0) && (pos.y < SCREEN_HEIGHT); item++)
    {
        // If selection is off the screen, scroll screen
        if(selected >= entries_in_screen)
            scroll = selected - entries_in_screen + 1;
        // Call function pointer to get current menu entry string
        result = (*getCurrentEntry)(entry_buf, sizeof(entry_buf), item+scroll);
        if(result != -1)
        {
            text_color = color_white;
            if(item + scroll == selected)
            {
                text_color = color_black;
                // Draw rectangle under selected item, compensating for text height
                point_t rect_pos = {0, pos.y - layout.menu_h + 3};
                gfx_drawRect(rect_pos, SCREEN_WIDTH, layout.menu_h, color_white, true); 
            }
            gfx_print(pos, layout.menu_font, TEXT_ALIGN_LEFT, text_color, entry_buf);
            pos.y += layout.menu_h;
        }
    }
}

void _ui_drawMenuListValue(ui_state_t* ui_state, uint8_t selected, 
                           int (*getCurrentEntry)(char *buf, uint8_t max_len, uint8_t index), 
                           int (*getCurrentValue)(char *buf, uint8_t max_len, uint8_t index))
{
    point_t pos = layout.line1_pos;
    // Number of menu entries that fit in the screen height
    uint8_t entries_in_screen = (SCREEN_HEIGHT - 1 - pos.y) / layout.menu_h + 1;
    uint8_t scroll = 0;
    char entry_buf[MAX_ENTRY_LEN] = "";
    char value_buf[MAX_ENTRY_LEN] = "";
    color_t text_color = color_white;
    for(int item=0, result=0; (result == 0) && (pos.y < SCREEN_HEIGHT); item++)
    {
        // If selection is off the screen, scroll screen
        if(selected >= entries_in_screen)
            scroll = selected - entries_in_screen + 1;
        // Call function pointer to get current menu entry string
        result = (*getCurrentEntry)(entry_buf, sizeof(entry_buf), item+scroll);
        // Call function pointer to get current entry value string
        result = (*getCurrentValue)(value_buf, sizeof(value_buf), item+scroll);
        if(result != -1)
        {
            text_color = color_white;
            if(item + scroll == selected)
            {
                // Draw rectangle under selected item, compensating for text height
                // If we are in edit mode, draw a hollow rectangle
                text_color = color_black;
                bool full_rect = true;
                if(ui_state->edit_mode)
                {
                    text_color = color_white;
                    full_rect = false;
                }
                point_t rect_pos = {0, pos.y - layout.menu_h + 3};
                gfx_drawRect(rect_pos, SCREEN_WIDTH, layout.menu_h, color_white, full_rect); 
            }
            gfx_print(pos, layout.menu_font, TEXT_ALIGN_LEFT, text_color, entry_buf);
            gfx_print(pos, layout.menu_font, TEXT_ALIGN_RIGHT, text_color, value_buf);
            pos.y += layout.menu_h;
        }
    }
}

int _ui_getMenuTopEntryName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= menu_num) return -1;
    snprintf(buf, max_len, "%s", menu_items[index]);
    return 0;
}

int _ui_getSettingsEntryName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= settings_num) return -1;
    snprintf(buf, max_len, "%s", settings_items[index]);
    return 0;
}

int _ui_getDisplayEntryName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= display_num) return -1;
    snprintf(buf, max_len, "%s", display_items[index]);
    return 0;
}

int _ui_getDisplayValueName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= display_num) return -1;
    uint8_t value = 0;
    switch(index)
    {
        case D_BRIGHTNESS:
            value = last_state.settings.brightness;
            break;
#ifdef SCREEN_CONTRAST
        case D_CONTRAST:
            value = last_state.settings.contrast;
            break;
#endif
    }
    snprintf(buf, max_len, "%d", value);
    return 0;
}

#ifdef HAS_GPS
int _ui_getSettingsGPSEntryName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= settings_gps_num) return -1;
    snprintf(buf, max_len, "%s", settings_gps_items[index]);
    return 0;
}

int _ui_getSettingsGPSValueName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= settings_gps_num) return -1;
    switch(index)
    {
        case G_ENABLED:
            snprintf(buf, max_len, "%s", (last_state.settings.gps_enabled) ? "ON" : "OFF");
            break;
        case G_SET_TIME:
            snprintf(buf, max_len, "%s", (last_state.settings.gps_set_time) ? "ON" : "OFF");
            break;
        case G_TIMEZONE:
            // Add + prefix to positive numbers
            if(last_state.settings.utc_timezone > 0)
                snprintf(buf, max_len, "+%d", last_state.settings.utc_timezone);
            else
                snprintf(buf, max_len, "%d", last_state.settings.utc_timezone);
            break;
    }
    return 0;
}
#endif

int _ui_getInfoEntryName(char *buf, uint8_t max_len, uint8_t index)
{
    if(index >= info_num) return -1;
    snprintf(buf, max_len, "%s", info_items[index]);
    return 0;
}

int _ui_getInfoValueName(char *buf, uint8_t max_len, uint8_t index)
{
    const hwInfo_t* hwinfo = platform_getHwInfo();
    if(index >= info_num) return -1;
    switch(index)
    {
        case 0: // Git Version
            snprintf(buf, max_len, "%s", GIT_VERSION);
            break;
        case 1: // Battery voltage
            snprintf(buf, max_len, "%.1fV", last_state.v_bat);
            break;
        case 2: // Battery charge
            snprintf(buf, max_len, "%.1f%%", last_state.charge * 100);
            break;
        case 3: // RSSI
            snprintf(buf, max_len, "%.1fdBm", last_state.rssi);
            break;
        case 4: // Model
            snprintf(buf, max_len, "%s", hwinfo->name);
            break;
        case 5: // Band
            snprintf(buf, max_len, "%s %s", hwinfo->vhf_band ? "VHF" : "", hwinfo->uhf_band ? "UHF" : "");
            break;
        case 6: // VHF
            snprintf(buf, max_len, "%d - %d", hwinfo->vhf_minFreq, hwinfo->vhf_maxFreq);
            break;
        case 7: // UHF
            snprintf(buf, max_len, "%d - %d", hwinfo->uhf_minFreq, hwinfo->uhf_maxFreq);
            break;
        case 8: // LCD Type
            snprintf(buf, max_len, "%d", hwinfo->lcd_type);
            break;
    }
    return 0;
}

int _ui_getZoneName(char *buf, uint8_t max_len, uint8_t index)
{
    int result = 0;
    // First zone "All channels" is not read from flash
    if(index == 0)
    {
        snprintf(buf, max_len, "All channels");
    }
    else
    {
        zone_t zone;
        result = nvm_readZoneData(&zone, index);
        if(result != -1)
            snprintf(buf, max_len, "%s", zone.name);
    }
    return result;
}

int _ui_getChannelName(char *buf, uint8_t max_len, uint8_t index)
{
    channel_t channel;
    int result = nvm_readChannelData(&channel, index + 1);
    if(result != -1)
        snprintf(buf, max_len, "%s", channel.name);
    return result;
}

int _ui_getContactName(char *buf, uint8_t max_len, uint8_t index)
{
    contact_t contact;
    int result = nvm_readContactData(&contact, index + 1);
    if(result != -1)
        snprintf(buf, max_len, "%s", contact.name);
    return result;
}

void _ui_drawMenuTop(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Menu" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Menu");
    // Print menu entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getMenuTopEntryName);
}

void _ui_drawMenuZone(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Zone" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Zone");
    // Print zone entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getZoneName);
}

void _ui_drawMenuChannel(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Channel" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Channels");
    // Print channel entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getChannelName);
}

void _ui_drawMenuContacts(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Contacts" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Contacts");
    // Print contact entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getContactName);
}

#ifdef HAS_GPS
void _ui_drawMenuGPS()
{
    char *fix_buf, *type_buf;
    gfx_clearScreen();
    // Print "GPS" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "GPS");
    point_t fix_pos = {layout.line2_pos.x, SCREEN_HEIGHT * 2 / 5};
    // Print GPS status, if no fix, hide details
    if(!last_state.settings.gps_enabled)
        gfx_print(fix_pos, layout.line3_font, TEXT_ALIGN_CENTER, 
                  color_white, "GPS OFF");
    else if (last_state.gps_data.fix_quality == 0)
        gfx_print(fix_pos, layout.line3_font, TEXT_ALIGN_CENTER, 
                  color_white, "No Fix");
    else if (last_state.gps_data.fix_quality == 6)
        gfx_print(fix_pos, layout.line3_font, TEXT_ALIGN_CENTER, 
                  color_white, "Fix Lost");
    else
    {
        switch(last_state.gps_data.fix_quality)
        {
            case 1:
                fix_buf = "SPS";
                break;
            case 2:
                fix_buf = "DGPS";
                break;
            case 3:
                fix_buf = "PPS";
                break;
            default:
                fix_buf = "ERROR";
                break;
        }

        switch(last_state.gps_data.fix_type)
        {
            case 1:
                type_buf = "";
                break;
            case 2:
                type_buf = "2D";
                break;
            case 3:
                type_buf = "3D";
                break;
            default:
                type_buf = "ERROR";
                break;
        }
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, fix_buf);
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white, "N     ");
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white, "%8.6f", last_state.gps_data.latitude);
        gfx_print(layout.line2_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, type_buf);
        // Convert from signed longitude, to unsigned + direction
        float longitude = last_state.gps_data.longitude;
        char *direction = (longitude < 0) ? "W     " : "E     ";
        longitude = (longitude < 0) ? -longitude : longitude;
        gfx_print(layout.line2_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white, direction);
        gfx_print(layout.line2_pos, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white, "%8.6f", longitude);
        gfx_print(layout.bottom_pos, layout.bottom_font, TEXT_ALIGN_CENTER,
                  color_white, "S %4.1fkm/h  A %4.1fm",
                  last_state.gps_data.speed,
                  last_state.gps_data.altitude);
    }
    // Draw compass
    point_t compass_pos = {layout.horizontal_pad * 2, SCREEN_HEIGHT / 2};
    gfx_drawGPScompass(compass_pos,
                       SCREEN_WIDTH / 9 + 2,
                       last_state.gps_data.tmg_true,
                       last_state.gps_data.fix_quality != 0 &&
                       last_state.gps_data.fix_quality != 6);
    // Draw satellites bar graph
    point_t bar_pos = {layout.line3_pos.x + SCREEN_WIDTH * 1 / 3, SCREEN_HEIGHT / 2};
    gfx_drawGPSgraph(bar_pos,
                     (SCREEN_WIDTH * 2 / 3) - layout.horizontal_pad,
                     SCREEN_HEIGHT / 3,
                     last_state.gps_data.satellites,
                     last_state.gps_data.active_sats);
}
void _ui_drawMenuSATpass(){
    //may crash on real radio when opened if task stack == 4k, 8k is fine though
    
    char sbuf[25] = { 0 }; //general purpose snprintf buffer
    char gridsquare[7] = {0}; //we want to use this as a c-style string so that extra byte stays zero

    float lat;
    float lon;
    /*float alt;*/
    double az = 0;
    double elev = 0;
    int doppler_offset = 0; //i want a auto_SI_prefix fn again

    gfx_clearScreen();
    //I've been using this to keep an eye on alignment, but remove later
    gfx_drawVLine(SCREEN_WIDTH/2, 1, color_grey);
    _ui_drawMainBackground(); 
    _ui_drawMainTop();
    _ui_drawBottom();

    //get a position. This will be used all over the place.
    if( ! last_state.settings.gps_enabled || last_state.gps_data.fix_quality == 0 ){
      //fix_type is 1 sometimes when it shouldn't be, have to use fix_quality 
      
      //TODO: need a way to show gps enabled/disable, gps fix/nofix
      /*gfx_print(layout.line3_pos, "no gps fix", FONT_SIZE_12PT, TEXT_ALIGN_CENTER, color_white);*/

      //TODO pull from manual position data rather than hardcoding
      lat =  41.70011;
      lon = -70.29947;
      /*alt = 0; //msl geoid meters*/
    } else {
      lat = last_state.gps_data.latitude;
      lon = last_state.gps_data.longitude;
      /*alt = last_state.gps_data.altitude; //msl geoid meters*/
    }
    
    //draw gridsquare text
    lat_lon_to_maidenhead(lat, lon, gridsquare, 3); //precision=3 here means 6 characters like FN41uq
    gfx_print(layout.line2_pos, gridsquare, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);

    snprintf(sbuf, 25, "AZ %.1f", az);
    gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);
    snprintf(sbuf, 25, "EL %.1f", elev);
    gfx_print(layout.line2_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);

    snprintf(sbuf, 25, "%.1fk DOP", ((float)doppler_offset)/1000);
    /*gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);*/
}
point_t azel_deg_to_xy( float az_deg, float elev_deg, float radius){
    /*printf("azel: %.2f %.2f\n", az_deg, elev_deg);*/
    point_t ret = {0,0};
    float az_r = (az_deg -90) * PI/180; 
    float r0 = (90-elev_deg)/90*radius;
    int spx = r0 * cos(az_r);
    int spy = r0 * sin(az_r);
    ret.x = spx;
    ret.y = spy;
    return ret;
}
void test_azel_deg_to_xy(){
    point_t in_azel = {90,0}; //so east and at max deflection
    int radius = 10;
    point_t out_xy = azel_deg_to_xy( in_azel.x, in_azel.y, radius);
    if( out_xy.x != 10 || out_xy.y != 0 ){
        printf("Error! Got %d %d\n", out_xy.x, out_xy.y);
    }
}
point_t offset_point( point_t center, int num, ... ){
    //doesn't work right now because point_t x and y are unsigned...

    va_list args;
    va_start(args, num);
    for( int i = 0; i < num; i++ ){
        point_t offset = va_arg(args, point_t);
        center.x += offset.x;
        center.y += offset.y;
    }
    return center;
}
typedef struct sat_pos {
  double jd;  //julian day/time stamp
  float az;   //degrees
  float elev; //degrees
  /*float dist; //meters*/
} sat_pos;
void _ui_drawMenuSAT()
{
    /*gfx_clearScreen();*/
    /*_ui_drawMainBackground(); */
    /*_ui_drawMainTop();*/
    /*_ui_drawBottom();*/
    test_azel_deg_to_xy();
    
    char sbuf[25] = { 0 }; //general purpose snprintf buffer
    char gridsquare[7] = {0}; //we want to use this as a c-style string so that extra byte stays zero

    float lat;
    float lon;
    /*float alt;*/
    double az = 0;
    double elev = 30;
    int doppler_offset = 0; //i want a auto_SI_prefix fn again

    sat_pos pass_azel[] = {
        {2459314.407656, 304.6, 0.1},{2459314.407666, 304.6,
        0.1},{2459314.407676, 304.6, 0.2},{2459314.407686, 304.6,
        0.2},{2459314.407696, 304.6, 0.3},{2459314.407706, 304.6,
        0.3},{2459314.407716, 304.6, 0.3},{2459314.407726, 304.6,
        0.4},{2459314.407736, 304.6, 0.4},{2459314.407746, 304.6,
        0.5},{2459314.407756, 304.5, 0.5},{2459314.407766, 304.5,
        0.6},{2459314.407776, 304.5, 0.6},{2459314.407786, 304.5,
        0.7},{2459314.407796, 304.5, 0.7},{2459314.407806, 304.5,
        0.8},{2459314.407816, 304.5, 0.8},{2459314.407826, 304.5,
        0.9},{2459314.407836, 304.5, 0.9},{2459314.407846, 304.5,
        1.0},{2459314.407856, 304.5, 1.0},{2459314.407866, 304.5,
        1.1},{2459314.407876, 304.5, 1.1},{2459314.407886, 304.5,
        1.1},{2459314.407896, 304.5, 1.2},{2459314.407906, 304.5,
        1.2},{2459314.407916, 304.5, 1.3},{2459314.407926, 304.5,
        1.3},{2459314.407936, 304.5, 1.4},{2459314.407946, 304.5,
        1.4},{2459314.407956, 304.5, 1.5},{2459314.407966, 304.5,
        1.5},{2459314.407976, 304.5, 1.6},{2459314.407986, 304.5,
        1.6},{2459314.407996, 304.5, 1.7},{2459314.408006, 304.5,
        1.7},{2459314.408016, 304.5, 1.8},{2459314.408026, 304.5,
        1.8},{2459314.408036, 304.5, 1.9},{2459314.408046, 304.5,
        1.9},{2459314.408056, 304.4, 2.0},{2459314.408066, 304.4,
        2.0},{2459314.408076, 304.4, 2.1},{2459314.408086, 304.4,
        2.1},{2459314.408096, 304.4, 2.2},{2459314.408106, 304.4,
        2.2},{2459314.408116, 304.4, 2.3},{2459314.408126, 304.4,
        2.3},{2459314.408136, 304.4, 2.4},{2459314.408146, 304.4,
        2.4},{2459314.408156, 304.4, 2.5},{2459314.408166, 304.4,
        2.5},{2459314.408176, 304.4, 2.6},{2459314.408186, 304.4,
        2.6},{2459314.408196, 304.4, 2.7},{2459314.408206, 304.4,
        2.7},{2459314.408216, 304.4, 2.8},{2459314.408226, 304.4,
        2.8},{2459314.408236, 304.4, 2.9},{2459314.408246, 304.4,
        2.9},{2459314.408256, 304.4, 3.0},{2459314.408266, 304.4,
        3.0},{2459314.408276, 304.4, 3.1},{2459314.408286, 304.4,
        3.1},{2459314.408296, 304.4, 3.2},{2459314.408306, 304.4,
        3.3},{2459314.408316, 304.3, 3.3},{2459314.408326, 304.3,
        3.4},{2459314.408336, 304.3, 3.4},{2459314.408346, 304.3,
        3.5},{2459314.408356, 304.3, 3.5},{2459314.408366, 304.3,
        3.6},{2459314.408376, 304.3, 3.6},{2459314.408386, 304.3,
        3.7},{2459314.408396, 304.3, 3.7},{2459314.408406, 304.3,
        3.8},{2459314.408416, 304.3, 3.8},{2459314.408426, 304.3,
        3.9},{2459314.408436, 304.3, 3.9},{2459314.408446, 304.3,
        4.0},{2459314.408456, 304.3, 4.1},{2459314.408466, 304.3,
        4.1},{2459314.408476, 304.3, 4.2},{2459314.408486, 304.3,
        4.2},{2459314.408496, 304.3, 4.3},{2459314.408506, 304.3,
        4.3},{2459314.408516, 304.3, 4.4},{2459314.408526, 304.3,
        4.4},{2459314.408536, 304.3, 4.5},{2459314.408546, 304.2,
        4.6},{2459314.408556, 304.2, 4.6},{2459314.408566, 304.2,
        4.7},{2459314.408576, 304.2, 4.7},{2459314.408586, 304.2,
        4.8},{2459314.408596, 304.2, 4.8},{2459314.408606, 304.2,
        4.9},{2459314.408616, 304.2, 4.9},{2459314.408626, 304.2,
        5.0},{2459314.408636, 304.2, 5.1},{2459314.408646, 304.2,
        5.1},{2459314.408656, 304.2, 5.2},{2459314.408666, 304.2,
        5.2},{2459314.408676, 304.2, 5.3},{2459314.408686, 304.2,
        5.4},{2459314.408696, 304.2, 5.4},{2459314.408706, 304.2,
        5.5},{2459314.408716, 304.2, 5.5},{2459314.408726, 304.2,
        5.6},{2459314.408736, 304.2, 5.6},{2459314.408746, 304.1,
        5.7},{2459314.408756, 304.1, 5.8},{2459314.408766, 304.1,
        5.8},{2459314.408776, 304.1, 5.9},{2459314.408786, 304.1,
        5.9},{2459314.408796, 304.1, 6.0},{2459314.408806, 304.1,
        6.1},{2459314.408816, 304.1, 6.1},{2459314.408826, 304.1,
        6.2},{2459314.408836, 304.1, 6.2},{2459314.408846, 304.1,
        6.3},{2459314.408856, 304.1, 6.4},{2459314.408866, 304.1,
        6.4},{2459314.408876, 304.1, 6.5},{2459314.408886, 304.1,
        6.6},{2459314.408896, 304.1, 6.6},{2459314.408906, 304.1,
        6.7},{2459314.408916, 304.1, 6.7},{2459314.408926, 304.0,
        6.8},{2459314.408936, 304.0, 6.9},{2459314.408946, 304.0,
        6.9},{2459314.408956, 304.0, 7.0},{2459314.408966, 304.0,
        7.1},{2459314.408976, 304.0, 7.1},{2459314.408986, 304.0,
        7.2},{2459314.408996, 304.0, 7.2},{2459314.409006, 304.0,
        7.3},{2459314.409016, 304.0, 7.4},{2459314.409026, 304.0,
        7.4},{2459314.409036, 304.0, 7.5},{2459314.409046, 304.0,
        7.6},{2459314.409056, 304.0, 7.6},{2459314.409066, 304.0,
        7.7},{2459314.409076, 304.0, 7.8},{2459314.409086, 303.9,
        7.8},{2459314.409096, 303.9, 7.9},{2459314.409106, 303.9,
        8.0},{2459314.409116, 303.9, 8.0},{2459314.409126, 303.9,
        8.1},{2459314.409136, 303.9, 8.2},{2459314.409146, 303.9,
        8.2},{2459314.409156, 303.9, 8.3},{2459314.409166, 303.9,
        8.4},{2459314.409176, 303.9, 8.4},{2459314.409186, 303.9,
        8.5},{2459314.409196, 303.9, 8.6},{2459314.409206, 303.9,
        8.7},{2459314.409216, 303.9, 8.7},{2459314.409226, 303.8,
        8.8},{2459314.409236, 303.8, 8.9},{2459314.409246, 303.8,
        8.9},{2459314.409256, 303.8, 9.0},{2459314.409266, 303.8,
        9.1},{2459314.409276, 303.8, 9.1},{2459314.409286, 303.8,
        9.2},{2459314.409296, 303.8, 9.3},{2459314.409306, 303.8,
        9.4},{2459314.409316, 303.8, 9.4},{2459314.409326, 303.8,
        9.5},{2459314.409336, 303.8, 9.6},{2459314.409346, 303.8,
        9.7},{2459314.409356, 303.7, 9.7},{2459314.409366, 303.7,
        9.8},{2459314.409376, 303.7, 9.9},{2459314.409386, 303.7,
        9.9},{2459314.409396, 303.7, 10.0},{2459314.409406, 303.7,
        10.1},{2459314.409416, 303.7, 10.2},{2459314.409426, 303.7,
        10.3},{2459314.409436, 303.7, 10.3},{2459314.409446, 303.7,
        10.4},{2459314.409456, 303.7, 10.5},{2459314.409466, 303.7,
        10.6},{2459314.409476, 303.6, 10.6},{2459314.409486, 303.6,
        10.7},{2459314.409496, 303.6, 10.8},{2459314.409506, 303.6,
        10.9},{2459314.409516, 303.6, 10.9},{2459314.409526, 303.6,
        11.0},{2459314.409536, 303.6, 11.1},{2459314.409546, 303.6,
        11.2},{2459314.409556, 303.6, 11.3},{2459314.409566, 303.6,
        11.4},{2459314.409576, 303.6, 11.4},{2459314.409586, 303.5,
        11.5},{2459314.409596, 303.5, 11.6},{2459314.409606, 303.5,
        11.7},{2459314.409616, 303.5, 11.8},{2459314.409626, 303.5,
        11.8},{2459314.409636, 303.5, 11.9},{2459314.409646, 303.5,
        12.0},{2459314.409656, 303.5, 12.1},{2459314.409666, 303.5,
        12.2},{2459314.409676, 303.5, 12.3},{2459314.409686, 303.4,
        12.4},{2459314.409696, 303.4, 12.4},{2459314.409706, 303.4,
        12.5},{2459314.409716, 303.4, 12.6},{2459314.409726, 303.4,
        12.7},{2459314.409736, 303.4, 12.8},{2459314.409746, 303.4,
        12.9},{2459314.409756, 303.4, 13.0},{2459314.409766, 303.4,
        13.1},{2459314.409776, 303.4, 13.1},{2459314.409786, 303.3,
        13.2},{2459314.409796, 303.3, 13.3},{2459314.409806, 303.3,
        13.4},{2459314.409816, 303.3, 13.5},{2459314.409826, 303.3,
        13.6},{2459314.409836, 303.3, 13.7},{2459314.409846, 303.3,
        13.8},{2459314.409856, 303.3, 13.9},{2459314.409866, 303.2,
        14.0},{2459314.409876, 303.2, 14.1},{2459314.409886, 303.2,
        14.2},{2459314.409896, 303.2, 14.3},{2459314.409906, 303.2,
        14.4},{2459314.409916, 303.2, 14.5},{2459314.409926, 303.2,
        14.6},{2459314.409936, 303.2, 14.7},{2459314.409946, 303.1,
        14.8},{2459314.409956, 303.1, 14.9},{2459314.409966, 303.1,
        15.0},{2459314.409976, 303.1, 15.1},{2459314.409986, 303.1,
        15.2},{2459314.409996, 303.1, 15.3},{2459314.410006, 303.1,
        15.4},{2459314.410016, 303.1, 15.5},{2459314.410026, 303.0,
        15.6},{2459314.410036, 303.0, 15.7},{2459314.410046, 303.0,
        15.8},{2459314.410056, 303.0, 15.9},{2459314.410066, 303.0,
        16.0},{2459314.410076, 303.0, 16.1},{2459314.410086, 303.0,
        16.2},{2459314.410096, 302.9, 16.3},{2459314.410106, 302.9,
        16.4},{2459314.410116, 302.9, 16.5},{2459314.410126, 302.9,
        16.7},{2459314.410136, 302.9, 16.8},{2459314.410146, 302.9,
        16.9},{2459314.410156, 302.8, 17.0},{2459314.410166, 302.8,
        17.1},{2459314.410176, 302.8, 17.2},{2459314.410186, 302.8,
        17.3},{2459314.410196, 302.8, 17.5},{2459314.410206, 302.8,
        17.6},{2459314.410216, 302.7, 17.7},{2459314.410226, 302.7,
        17.8},{2459314.410236, 302.7, 17.9},{2459314.410246, 302.7,
        18.0},{2459314.410256, 302.7, 18.2},{2459314.410266, 302.7,
        18.3},{2459314.410276, 302.6, 18.4},{2459314.410286, 302.6,
        18.5},{2459314.410296, 302.6, 18.7},{2459314.410306, 302.6,
        18.8},{2459314.410316, 302.6, 18.9},{2459314.410326, 302.5,
        19.0},{2459314.410336, 302.5, 19.2},{2459314.410346, 302.5,
        19.3},{2459314.410356, 302.5, 19.4},{2459314.410366, 302.5,
        19.6},{2459314.410376, 302.4, 19.7},{2459314.410386, 302.4,
        19.8},{2459314.410396, 302.4, 20.0},{2459314.410406, 302.4,
        20.1},{2459314.410416, 302.4, 20.2},{2459314.410426, 302.3,
        20.4},{2459314.410436, 302.3, 20.5},{2459314.410446, 302.3,
        20.7},{2459314.410456, 302.3, 20.8},{2459314.410466, 302.3,
        20.9},{2459314.410476, 302.2, 21.1},{2459314.410486, 302.2,
        21.2},{2459314.410496, 302.2, 21.4},{2459314.410506, 302.2,
        21.5},{2459314.410516, 302.1, 21.7},{2459314.410526, 302.1,
        21.8},{2459314.410536, 302.1, 22.0},{2459314.410546, 302.1,
        22.1},{2459314.410556, 302.0, 22.3},{2459314.410566, 302.0,
        22.4},{2459314.410576, 302.0, 22.6},{2459314.410586, 302.0,
        22.8},{2459314.410596, 301.9, 22.9},{2459314.410606, 301.9,
        23.1},{2459314.410616, 301.9, 23.2},{2459314.410626, 301.9,
        23.4},{2459314.410636, 301.8, 23.6},{2459314.410646, 301.8,
        23.7},{2459314.410656, 301.8, 23.9},{2459314.410666, 301.7,
        24.1},{2459314.410676, 301.7, 24.2},{2459314.410686, 301.7,
        24.4},{2459314.410696, 301.7, 24.6},{2459314.410706, 301.6,
        24.8},{2459314.410716, 301.6, 24.9},{2459314.410726, 301.6,
        25.1},{2459314.410736, 301.5, 25.3},{2459314.410746, 301.5,
        25.5},{2459314.410756, 301.5, 25.7},{2459314.410766, 301.4,
        25.9},{2459314.410776, 301.4, 26.1},{2459314.410786, 301.4,
        26.2},{2459314.410796, 301.3, 26.4},{2459314.410806, 301.3,
        26.6},{2459314.410816, 301.3, 26.8},{2459314.410826, 301.2,
        27.0},{2459314.410836, 301.2, 27.2},{2459314.410846, 301.1,
        27.4},{2459314.410856, 301.1, 27.6},{2459314.410866, 301.1,
        27.8},{2459314.410876, 301.0, 28.0},{2459314.410886, 301.0,
        28.3},{2459314.410896, 301.0, 28.5},{2459314.410906, 300.9,
        28.7},{2459314.410916, 300.9, 28.9},{2459314.410926, 300.8,
        29.1},{2459314.410936, 300.8, 29.3},{2459314.410946, 300.7,
        29.6},{2459314.410956, 300.7, 29.8},{2459314.410966, 300.6,
        30.0},{2459314.410976, 300.6, 30.3},{2459314.410986, 300.6,
        30.5},{2459314.410996, 300.5, 30.7},{2459314.411006, 300.5,
        31.0},{2459314.411016, 300.4, 31.2},{2459314.411026, 300.4,
        31.5},{2459314.411036, 300.3, 31.7},{2459314.411046, 300.3,
        32.0},{2459314.411056, 300.2, 32.2},{2459314.411066, 300.1,
        32.5},{2459314.411076, 300.1, 32.7},{2459314.411086, 300.0,
        33.0},{2459314.411096, 300.0, 33.3},{2459314.411106, 299.9,
        33.5},{2459314.411116, 299.9, 33.8},{2459314.411126, 299.8,
        34.1},{2459314.411136, 299.7, 34.4},{2459314.411146, 299.7,
        34.7},{2459314.411156, 299.6, 34.9},{2459314.411166, 299.5,
        35.2},{2459314.411176, 299.5, 35.5},{2459314.411186, 299.4,
        35.8},{2459314.411196, 299.3, 36.1},{2459314.411206, 299.3,
        36.4},{2459314.411216, 299.2, 36.7},{2459314.411226, 299.1,
        37.0},{2459314.411236, 299.0, 37.4},{2459314.411246, 298.9,
        37.7},{2459314.411256, 298.9, 38.0},{2459314.411266, 298.8,
        38.3},{2459314.411276, 298.7, 38.7},{2459314.411286, 298.6,
        39.0},{2459314.411296, 298.5, 39.3},{2459314.411306, 298.4,
        39.7},{2459314.411316, 298.3, 40.0},{2459314.411326, 298.2,
        40.4},{2459314.411336, 298.1, 40.8},{2459314.411346, 298.0,
        41.1},{2459314.411356, 297.9, 41.5},{2459314.411366, 297.8,
        41.9},{2459314.411376, 297.7, 42.2},{2459314.411386, 297.6,
        42.6},{2459314.411396, 297.5, 43.0},{2459314.411406, 297.4,
        43.4},{2459314.411416, 297.3, 43.8},{2459314.411426, 297.1,
        44.2},{2459314.411436, 297.0, 44.6},{2459314.411446, 296.9,
        45.0},{2459314.411456, 296.7, 45.5},{2459314.411466, 296.6,
        45.9},{2459314.411476, 296.5, 46.3},{2459314.411486, 296.3,
        46.7},{2459314.411496, 296.2, 47.2},{2459314.411506, 296.0,
        47.6},{2459314.411516, 295.8, 48.1},{2459314.411526, 295.7,
        48.6},{2459314.411536, 295.5, 49.0},{2459314.411546, 295.3,
        49.5},{2459314.411556, 295.1, 50.0},{2459314.411566, 294.9,
        50.5},{2459314.411576, 294.7, 51.0},{2459314.411586, 294.5,
        51.5},{2459314.411596, 294.3, 52.0},{2459314.411606, 294.1,
        52.5},{2459314.411616, 293.9, 53.0},{2459314.411626, 293.6,
        53.5},{2459314.411636, 293.4, 54.0},{2459314.411646, 293.1,
        54.6},{2459314.411656, 292.9, 55.1},{2459314.411666, 292.6,
        55.7},{2459314.411676, 292.3, 56.2},{2459314.411686, 292.0,
        56.8},{2459314.411696, 291.7, 57.4},{2459314.411706, 291.3,
        57.9},{2459314.411716, 291.0, 58.5},{2459314.411726, 290.6,
        59.1},{2459314.411736, 290.2, 59.7},{2459314.411746, 289.8,
        60.3},{2459314.411756, 289.4, 60.9},{2459314.411766, 288.9,
        61.5},{2459314.411776, 288.5, 62.2},{2459314.411786, 288.0,
        62.8},{2459314.411796, 287.5, 63.4},{2459314.411806, 286.9,
        64.1},{2459314.411816, 286.3, 64.7},{2459314.411826, 285.7,
        65.3},{2459314.411836, 285.0, 66.0},{2459314.411846, 284.3,
        66.6},{2459314.411856, 283.6, 67.3},{2459314.411866, 282.8,
        68.0},{2459314.411876, 281.9, 68.6},{2459314.411886, 281.0,
        69.3},{2459314.411896, 280.0, 70.0},{2459314.411906, 278.9,
        70.6},{2459314.411916, 277.8, 71.3},{2459314.411926, 276.6,
        72.0},{2459314.411936, 275.2, 72.6},{2459314.411946, 273.8,
        73.3},{2459314.411956, 272.2, 73.9},{2459314.411966, 270.4,
        74.6},{2459314.411976, 268.6, 75.2},{2459314.411986, 266.5,
        75.8},{2459314.411996, 264.2, 76.4},{2459314.412006, 261.7,
        77.0},{2459314.412016, 259.0, 77.6},{2459314.412026, 256.0,
        78.1},{2459314.412036, 252.8, 78.6},{2459314.412046, 249.2,
        79.1},{2459314.412056, 245.3, 79.5},{2459314.412066, 241.0,
        79.9},{2459314.412076, 236.5, 80.2},{2459314.412086, 231.7,
        80.5},{2459314.412096, 226.6, 80.6},{2459314.412106, 221.4,
        80.8},{2459314.412116, 216.0, 80.8},{2459314.412126, 210.7,
        80.7},{2459314.412136, 205.5, 80.6},{2459314.412146, 200.4,
        80.4},{2459314.412156, 195.5, 80.1},{2459314.412166, 191.0,
        79.8},{2459314.412176, 186.8, 79.4},{2459314.412186, 182.8,
        79.0},{2459314.412196, 179.3, 78.5},{2459314.412206, 176.0,
        77.9},{2459314.412216, 173.0, 77.4},{2459314.412226, 170.2,
        76.8},{2459314.412236, 167.7, 76.2},{2459314.412246, 165.5,
        75.5},{2459314.412256, 163.4, 74.9},{2459314.412266, 161.5,
        74.2},{2459314.412276, 159.8, 73.5},{2459314.412286, 158.2,
        72.8},{2459314.412296, 156.7, 72.1},{2459314.412306, 155.4,
        71.4},{2459314.412316, 154.1, 70.7},{2459314.412326, 153.0,
        70.0},{2459314.412336, 151.9, 69.2},{2459314.412346, 150.9,
        68.5},{2459314.412356, 150.0, 67.8},{2459314.412366, 149.2,
        67.1},{2459314.412376, 148.4, 66.3},{2459314.412386, 147.6,
        65.6},{2459314.412396, 146.9, 64.9},{2459314.412406, 146.2,
        64.2},{2459314.412416, 145.6, 63.5},{2459314.412426, 145.0,
        62.8},{2459314.412436, 144.5, 62.1},{2459314.412446, 143.9,
        61.4},{2459314.412456, 143.4, 60.7},{2459314.412466, 143.0,
        60.0},{2459314.412476, 142.5, 59.3},{2459314.412486, 142.1,
        58.6},{2459314.412496, 141.7, 58.0},{2459314.412506, 141.3,
        57.3},{2459314.412516, 140.9, 56.7},{2459314.412526, 140.6,
        56.0},{2459314.412536, 140.3, 55.4},{2459314.412546, 139.9,
        54.7},{2459314.412556, 139.6, 54.1},{2459314.412566, 139.3,
        53.5},{2459314.412576, 139.1, 52.9},{2459314.412586, 138.8,
        52.2},{2459314.412596, 138.5, 51.6},{2459314.412606, 138.3,
        51.0},{2459314.412616, 138.0, 50.5},{2459314.412626, 137.8,
        49.9},{2459314.412636, 137.6, 49.3},{2459314.412646, 137.4,
        48.7},{2459314.412656, 137.2, 48.2},{2459314.412666, 137.0,
        47.6},{2459314.412676, 136.8, 47.1},{2459314.412686, 136.6,
        46.5},{2459314.412696, 136.4, 46.0},{2459314.412706, 136.2,
        45.5},{2459314.412716, 136.1, 44.9},{2459314.412726, 135.9,
        44.4},{2459314.412736, 135.7, 43.9},{2459314.412746, 135.6,
        43.4},{2459314.412756, 135.4, 42.9},{2459314.412766, 135.3,
        42.4},{2459314.412776, 135.2, 42.0},{2459314.412786, 135.0,
        41.5},{2459314.412796, 134.9, 41.0},{2459314.412806, 134.8,
        40.6},{2459314.412816, 134.6, 40.1},{2459314.412826, 134.5,
        39.6},{2459314.412836, 134.4, 39.2},{2459314.412846, 134.3,
        38.8},{2459314.412856, 134.2, 38.3},{2459314.412866, 134.1,
        37.9},{2459314.412876, 134.0, 37.5},{2459314.412886, 133.9,
        37.1},{2459314.412896, 133.8, 36.7},{2459314.412906, 133.7,
        36.3},{2459314.412916, 133.6, 35.9},{2459314.412926, 133.5,
        35.5},{2459314.412936, 133.4, 35.1},{2459314.412946, 133.3,
        34.7},{2459314.412956, 133.2, 34.3},{2459314.412966, 133.1,
        33.9},{2459314.412976, 133.0, 33.6},{2459314.412986, 133.0,
        33.2},{2459314.412996, 132.9, 32.8},{2459314.413006, 132.8,
        32.5},{2459314.413016, 132.7, 32.1},{2459314.413026, 132.6,
        31.8},{2459314.413036, 132.6, 31.5},{2459314.413046, 132.5,
        31.1},{2459314.413056, 132.4, 30.8},{2459314.413066, 132.4,
        30.5},{2459314.413076, 132.3, 30.1},{2459314.413086, 132.2,
        29.8},{2459314.413096, 132.2, 29.5},{2459314.413106, 132.1,
        29.2},{2459314.413116, 132.0, 28.9},{2459314.413126, 132.0,
        28.6},{2459314.413136, 131.9, 28.3},{2459314.413146, 131.9,
        28.0},{2459314.413156, 131.8, 27.7},{2459314.413166, 131.8,
        27.4},{2459314.413176, 131.7, 27.1},{2459314.413186, 131.6,
        26.8},{2459314.413196, 131.6, 26.5},{2459314.413206, 131.5,
        26.3},{2459314.413216, 131.5, 26.0},{2459314.413226, 131.4,
        25.7},{2459314.413236, 131.4, 25.5},{2459314.413246, 131.3,
        25.2},{2459314.413256, 131.3, 24.9},{2459314.413266, 131.2,
        24.7},{2459314.413276, 131.2, 24.4},{2459314.413286, 131.2,
        24.2},{2459314.413296, 131.1, 23.9},{2459314.413306, 131.1,
        23.7},{2459314.413316, 131.0, 23.4},{2459314.413326, 131.0,
        23.2},{2459314.413336, 130.9, 23.0},{2459314.413346, 130.9,
        22.7},{2459314.413356, 130.9, 22.5},{2459314.413366, 130.8,
        22.3},{2459314.413376, 130.8, 22.0},{2459314.413386, 130.7,
        21.8},{2459314.413396, 130.7, 21.6},{2459314.413406, 130.7,
        21.4},{2459314.413416, 130.6, 21.2},{2459314.413426, 130.6,
        20.9},{2459314.413436, 130.6, 20.7},{2459314.413446, 130.5,
        20.5},{2459314.413456, 130.5, 20.3},{2459314.413466, 130.5,
        20.1},{2459314.413476, 130.4, 19.9},{2459314.413486, 130.4,
        19.7},{2459314.413496, 130.4, 19.5},{2459314.413506, 130.3,
        19.3},{2459314.413516, 130.3, 19.1},{2459314.413526, 130.3,
        18.9},{2459314.413536, 130.2, 18.7},{2459314.413546, 130.2,
        18.5},{2459314.413556, 130.2, 18.3},{2459314.413566, 130.1,
        18.2},{2459314.413576, 130.1, 18.0},{2459314.413586, 130.1,
        17.8},{2459314.413596, 130.1, 17.6},{2459314.413606, 130.0,
        17.4},{2459314.413616, 130.0, 17.2},{2459314.413626, 130.0,
        17.1},{2459314.413636, 129.9, 16.9},{2459314.413646, 129.9,
        16.7},{2459314.413656, 129.9, 16.6},{2459314.413666, 129.9,
        16.4},{2459314.413676, 129.8, 16.2},{2459314.413686, 129.8,
        16.1},{2459314.413696, 129.8, 15.9},{2459314.413706, 129.8,
        15.7},{2459314.413716, 129.7, 15.6},{2459314.413726, 129.7,
        15.4},{2459314.413736, 129.7, 15.2},{2459314.413746, 129.7,
        15.1},{2459314.413756, 129.6, 14.9},{2459314.413766, 129.6,
        14.8},{2459314.413776, 129.6, 14.6},{2459314.413786, 129.6,
        14.5},{2459314.413796, 129.6, 14.3},{2459314.413806, 129.5,
        14.2},{2459314.413816, 129.5, 14.0},{2459314.413826, 129.5,
        13.9},{2459314.413836, 129.5, 13.7},{2459314.413846, 129.4,
        13.6},{2459314.413856, 129.4, 13.4},{2459314.413866, 129.4,
        13.3},{2459314.413876, 129.4, 13.1},{2459314.413886, 129.4,
        13.0},{2459314.413896, 129.3, 12.9},{2459314.413906, 129.3,
        12.7},{2459314.413916, 129.3, 12.6},{2459314.413926, 129.3,
        12.5},{2459314.413936, 129.3, 12.3},{2459314.413946, 129.3,
        12.2},{2459314.413956, 129.2, 12.1},{2459314.413966, 129.2,
        11.9},{2459314.413976, 129.2, 11.8},{2459314.413986, 129.2,
        11.7},{2459314.413996, 129.2, 11.5},{2459314.414006, 129.1,
        11.4},{2459314.414016, 129.1, 11.3},{2459314.414026, 129.1,
        11.2},{2459314.414036, 129.1, 11.0},{2459314.414046, 129.1,
        10.9},{2459314.414056, 129.1, 10.8},{2459314.414066, 129.0,
        10.7},{2459314.414076, 129.0, 10.5},{2459314.414086, 129.0,
        10.4},{2459314.414096, 129.0, 10.3},{2459314.414106, 129.0,
        10.2},{2459314.414116, 129.0, 10.1},{2459314.414126, 128.9,
        9.9},{2459314.414136, 128.9, 9.8},{2459314.414146, 128.9,
        9.7},{2459314.414156, 128.9, 9.6},{2459314.414166, 128.9,
        9.5},{2459314.414176, 128.9, 9.4},{2459314.414186, 128.9,
        9.3},{2459314.414196, 128.8, 9.1},{2459314.414206, 128.8,
        9.0},{2459314.414216, 128.8, 8.9},{2459314.414226, 128.8,
        8.8},{2459314.414236, 128.8, 8.7},{2459314.414246, 128.8,
        8.6},{2459314.414256, 128.8, 8.5},{2459314.414266, 128.7,
        8.4},{2459314.414276, 128.7, 8.3},{2459314.414286, 128.7,
        8.2},{2459314.414296, 128.7, 8.1},{2459314.414306, 128.7,
        8.0},{2459314.414316, 128.7, 7.9},{2459314.414326, 128.7,
        7.8},{2459314.414336, 128.6, 7.7},{2459314.414346, 128.6,
        7.6},{2459314.414356, 128.6, 7.5},{2459314.414366, 128.6,
        7.4},{2459314.414376, 128.6, 7.3},{2459314.414386, 128.6,
        7.2},{2459314.414396, 128.6, 7.1},{2459314.414406, 128.6,
        7.0},{2459314.414416, 128.5, 6.9},{2459314.414426, 128.5,
        6.8},{2459314.414436, 128.5, 6.7},{2459314.414446, 128.5,
        6.6},{2459314.414456, 128.5, 6.5},{2459314.414466, 128.5,
        6.4},{2459314.414476, 128.5, 6.3},{2459314.414486, 128.5,
        6.2},{2459314.414496, 128.5, 6.1},{2459314.414506, 128.4,
        6.0},{2459314.414516, 128.4, 5.9},{2459314.414526, 128.4,
        5.8},{2459314.414536, 128.4, 5.7},{2459314.414546, 128.4,
        5.6},{2459314.414556, 128.4, 5.6},{2459314.414566, 128.4,
        5.5},{2459314.414576, 128.4, 5.4},{2459314.414586, 128.4,
        5.3},{2459314.414596, 128.3, 5.2},{2459314.414606, 128.3,
        5.1},{2459314.414616, 128.3, 5.0},{2459314.414626, 128.3,
        4.9},{2459314.414636, 128.3, 4.9},{2459314.414646, 128.3,
        4.8},{2459314.414656, 128.3, 4.7},{2459314.414666, 128.3,
        4.6},{2459314.414676, 128.3, 4.5},{2459314.414686, 128.3,
        4.4},{2459314.414696, 128.2, 4.3},{2459314.414706, 128.2,
        4.3},{2459314.414716, 128.2, 4.2},{2459314.414726, 128.2,
        4.1},{2459314.414736, 128.2, 4.0},{2459314.414746, 128.2,
        3.9},{2459314.414756, 128.2, 3.9},{2459314.414766, 128.2,
        3.8},{2459314.414776, 128.2, 3.7},{2459314.414786, 128.2,
        3.6},{2459314.414796, 128.2, 3.5},{2459314.414806, 128.1,
        3.5},{2459314.414816, 128.1, 3.4},{2459314.414826, 128.1,
        3.3},{2459314.414836, 128.1, 3.2},{2459314.414846, 128.1,
        3.1},{2459314.414856, 128.1, 3.1},{2459314.414866, 128.1,
        3.0},{2459314.414876, 128.1, 2.9},{2459314.414886, 128.1,
        2.8},{2459314.414896, 128.1, 2.8},{2459314.414906, 128.1,
        2.7},{2459314.414916, 128.0, 2.6},{2459314.414926, 128.0,
        2.5},{2459314.414936, 128.0, 2.5},{2459314.414946, 128.0,
        2.4},{2459314.414956, 128.0, 2.3},{2459314.414966, 128.0,
        2.2},{2459314.414976, 128.0, 2.2},{2459314.414986, 128.0,
        2.1},{2459314.414996, 128.0, 2.0},{2459314.415006, 128.0,
        1.9},{2459314.415016, 128.0, 1.9},{2459314.415026, 128.0,
        1.8},{2459314.415036, 128.0, 1.7},{2459314.415046, 127.9,
        1.7},{2459314.415056, 127.9, 1.6},{2459314.415066, 127.9,
        1.5},{2459314.415076, 127.9, 1.4},{2459314.415086, 127.9,
        1.4},{2459314.415096, 127.9, 1.3},{2459314.415106, 127.9,
        1.2},{2459314.415116, 127.9, 1.2},{2459314.415126, 127.9,
        1.1},{2459314.415136, 127.9, 1.0},{2459314.415146, 127.9,
        1.0},{2459314.415156, 127.9, 0.9},{2459314.415166, 127.9,
        0.8},{2459314.415176, 127.8, 0.7},{2459314.415186, 127.8,
        0.7},{2459314.415196, 127.8, 0.6},{2459314.415206, 127.8,
        0.5},{2459314.415216, 127.8, 0.5},{2459314.415226, 127.8,
        0.4},{2459314.415236, 127.8, 0.3},{2459314.415246, 127.8,
        0.3},{2459314.415256, 127.8, 0.2},{2459314.415266, 127.8,
        0.1},{2459314.415276, 127.8, 0.1},{2459314.415286, 127.8,
        0.0},{2459314.415296, 127.8, -0.0},
    };
    int num_points_pass = sizeof(pass_azel) / sizeof(sat_pos);
    /*double UT = (double)last_state.time.hour + ((double)last_state.time.minute)/60 + ((double) last_state.time.second)/3600;*/
    /*double jd_frac = UT/24;*/
    double jd = curTime_to_julian_day(last_state.time);
    static double jd_offset = 0;
    if( jd_offset == 0 ){
        jd_offset = pass_azel[0].jd - jd;
    }
    jd = jd + jd_offset;
    for( int i = 0; i < num_points_pass; i++){
        sat_pos p = pass_azel[i];
        if( p.jd <= jd ){
            az = p.az;
            elev = p.elev;
        } else {
            printf("pass[%d]\n", i);
            break;
        }
    }

    gfx_clearScreen();
    //I've been using this to keep an eye on alignment, but remove later
    /*gfx_drawVLine(SCREEN_WIDTH/2, 1, color_grey);*/
    /*gfx_drawHLine(SCREEN_HEIGHT/2, 1, color_grey);*/
    _ui_drawMainBackground(); 
    _ui_drawMainTop();
    _ui_drawBottom();

    //get a position. This will be used all over the place.
    if( ! last_state.settings.gps_enabled || last_state.gps_data.fix_quality == 0 ){
      //fix_type is 1 sometimes when it shouldn't be, have to use fix_quality 
      
      //TODO: need a way to show gps enabled/disable, gps fix/nofix
      /*gfx_print(layout.line3_pos, "no gps fix", FONT_SIZE_12PT, TEXT_ALIGN_CENTER, color_white);*/

      //TODO pull from manual position data rather than hardcoding
      lat =  41.70011;
      lon = -70.29947;
      /*alt = 0; //msl geoid meters*/
    } else {
      lat = last_state.gps_data.latitude;
      lon = last_state.gps_data.longitude;
      /*alt = last_state.gps_data.altitude; //msl geoid meters*/
    }
    
    // left side
    // relative coordinates to satellite
    snprintf(sbuf, 25, "AZ %.1f", az);
    gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);
    snprintf(sbuf, 25, "EL %.1f", elev);
    gfx_print(layout.line2_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);

    //right side
    //doppler correction readout
    snprintf(sbuf, 25, "%.1fk DOP", ((float)doppler_offset)/1000);
    /*gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);*/
    //draw gridsquare text
    lat_lon_to_maidenhead(lat, lon, gridsquare, 3); //precision=3 here means 6 characters like FN41uq
    gfx_print(layout.line1_pos, gridsquare, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);

    //center bottom - show 
    //satellite and AOS/LOS countdown 
    /*gfx_print(layout.line3_pos, "ISS AOS 30s", FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);*/

    //draw Az/El
    int r1 = SCREEN_WIDTH/2/4; //0 degrees elev
    int r2 = r1/2; // 45 degrees
    point_t az_center = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2-5};
    point_t az_top = {SCREEN_WIDTH/2-1, SCREEN_HEIGHT/2-5-r1+6};
    point_t az_right = {SCREEN_WIDTH/2+r1-5, SCREEN_HEIGHT/2-3};
    point_t az_left = {SCREEN_WIDTH/2-r1+3, SCREEN_HEIGHT/2-3};
    point_t az_bot = {SCREEN_WIDTH/2-1, SCREEN_HEIGHT/2-5+r1};
    gfx_drawCircle(az_center, r1, color_grey);
    gfx_drawCircle(az_center, r2, color_grey);
    gfx_print(az_top,  "N", FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_grey);
    gfx_print(az_right,"E", FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_grey);
    gfx_print(az_left, "W", FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_grey);
    gfx_print(az_bot,  "S", FONT_SIZE_5PT, TEXT_ALIGN_LEFT, color_grey);


    point_t plus_ctr_offset_8pt = {-4,2};
    point_t relsatpos = azel_deg_to_xy( az, elev, r1);
    point_t satpos = offset_point(az_center, 2, plus_ctr_offset_8pt, relsatpos );
                        
    gfx_print(satpos,  "+", FONT_SIZE_8PT, TEXT_ALIGN_LEFT, yellow_fab413);
    
    for( int i = 0; i < num_points_pass; i+=2 ){
        point_t offset = azel_deg_to_xy( pass_azel[i].az, pass_azel[i].elev, r1);
        point_t set = offset_point( az_center, 1, offset);
        gfx_setPixel(set, color_white);
    }
  char * pass_state;
  double mark;
  double pass_start_jd = pass_azel[0].jd; 
  double pass_end_jd = pass_azel[num_points_pass-1].jd; 
  //mark is the time we care about most - e.g. before a pass, it's the time the pass starts
  //or during a pass, it's the LOS time (when the sat will go below the horizon)
  if( jd < pass_start_jd ){
    //before this pass comes over the horizon
    pass_state = "AOS";
    mark = pass_start_jd;
  } else if ( jd > pass_start_jd && jd < pass_end_jd ){
    //during the pass
    pass_state = "LOS";
    mark = pass_end_jd;
  } else {
    //now it's gone over the horizon, so same as the elif above (just will be a
    //negative number to show it was in the past)
    //left here for clarity to show the actual LOS condition
    pass_state = "LOS"; 
    mark = pass_end_jd;
  }
  float diff = (mark - jd)*86400; //diff is seconds until (+) or since (-) the mark timestamp
  /*printf("%f\n",diff);*/
  const char * sat_name = "ISS";
  snprintf(sbuf, 25, "%s %s %.0fs", sat_name, pass_state, diff);
  gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);
}
#endif

void _ui_drawMenuSettings(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Settings" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Settings");
    // Print menu entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getSettingsEntryName);
}

void _ui_drawMenuInfo(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Info" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Info");
    // Print menu entries
    _ui_drawMenuListValue(ui_state, ui_state->menu_selected, _ui_getInfoEntryName,
                           _ui_getInfoValueName);
}

void _ui_drawMenuAbout()
{
    gfx_clearScreen();
    point_t openrtx_pos = {layout.horizontal_pad, layout.line3_h};
    if(SCREEN_HEIGHT >= 100)
        ui_drawSplashScreen(false);
    else
        gfx_print(openrtx_pos, layout.line3_font, TEXT_ALIGN_CENTER, 
                  color_white, "OpenRTX");
    uint8_t line_h = layout.menu_h;
    point_t pos = {SCREEN_WIDTH / 7, SCREEN_HEIGHT - (line_h * (author_num - 1)) - 5};
    for(int author = 0; author < author_num; author++)
    {
        gfx_print(pos, layout.top_font, TEXT_ALIGN_LEFT, 
                  color_white, "%s", authors[author]);
        pos.y += line_h;
    }
}

void _ui_drawSettingsDisplay(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Display" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Display");
    // Print display settings entries
    _ui_drawMenuListValue(ui_state, ui_state->menu_selected, _ui_getDisplayEntryName,
                           _ui_getDisplayValueName);
}

#ifdef HAS_GPS
void _ui_drawSettingsGPS(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "GPS Settings" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "GPS Settings");
    // Print display settings entries
    _ui_drawMenuListValue(ui_state, ui_state->menu_selected, 
                          _ui_getSettingsGPSEntryName,
                          _ui_getSettingsGPSValueName);
}
#endif

#ifdef HAS_RTC
void _ui_drawSettingsTimeDate()
{
    gfx_clearScreen();
    curTime_t local_time = state_getLocalTime(last_state.time);
    // Print "Time&Date" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Time&Date");
    // Print current time and date
    gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
              color_white, "%02d/%02d/%02d", 
              local_time.date, local_time.month, local_time.year);
    gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
              color_white, "%02d:%02d:%02d", 
              local_time.hour, local_time.minute, local_time.second);
}

void _ui_drawSettingsTimeDateSet(ui_state_t* ui_state)
{
    (void) last_state;

    gfx_clearScreen();
    // Print "Time&Date" on top bar
    gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER, 
              color_white, "Time&Date");
    if(ui_state->input_position <= 0)
    {
        strcpy(ui_state->new_date_buf, "__/__/__");
        strcpy(ui_state->new_time_buf, "__:__:00");
    }
    else
    {
        char input_char = ui_state->input_number + '0';
        // Insert date digit
        if(ui_state->input_position <= 6)
        {
            uint8_t pos = ui_state->input_position -1;
            // Skip "/"
            if(ui_state->input_position > 2) pos += 1;
            if(ui_state->input_position > 4) pos += 1;
            ui_state->new_date_buf[pos] = input_char;
        }
        // Insert time digit
        else
        {
            uint8_t pos = ui_state->input_position -7;
            // Skip ":"
            if(ui_state->input_position > 8) pos += 1;
            ui_state->new_time_buf[pos] = input_char; 
        }
    }
    gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
              color_white, ui_state->new_date_buf);
    gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
              color_white, ui_state->new_time_buf);
}
#endif

bool _ui_drawMacroMenu() {
        // Header
        gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white, "Macro Menu");
        // First row
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  yellow_fab413, "1");
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, "  %6.1f",
                  ctcss_tone[last_state.channel.fm.txTone]/10.0f);
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  yellow_fab413, "2       ");
        char encdec_str[9] = { 0 };
        bool tone_tx_enable = last_state.channel.fm.txToneEn;
        bool tone_rx_enable = last_state.channel.fm.rxToneEn;
        if (tone_tx_enable && tone_rx_enable)
            snprintf(encdec_str, 9, "     E+D");
        else if (tone_tx_enable && !tone_rx_enable)
            snprintf(encdec_str, 9, "      E ");
        else if (!tone_tx_enable && tone_rx_enable)
            snprintf(encdec_str, 9, "      D ");
        else
            snprintf(encdec_str, 9, "        ");
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white, encdec_str);
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_RIGHT,
                  yellow_fab413, "3        ");
        gfx_print(layout.line1_pos, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white, "%.1gW", last_state.channel.power);
        // Second row
        // Calculate symmetric second row position, line2_pos is asymmetric like main screen
        point_t pos_2 = {layout.line1_pos.x, layout.line1_pos.y + 
                        (layout.line3_pos.y - layout.line1_pos.y)/2};
        gfx_print(pos_2, layout.top_font, TEXT_ALIGN_LEFT,
                  yellow_fab413, "4");
        char bw_str[8] = { 0 };
        switch (last_state.channel.bandwidth)
        {
            case BW_12_5:
                snprintf(bw_str, 8, "   12.5");
                break;
            case BW_20:
                snprintf(bw_str, 8, "     20");
                break;
            case BW_25:
                snprintf(bw_str, 8, "     25");
                break;
        }
        gfx_print(pos_2, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, bw_str);
        gfx_print(pos_2, layout.top_font, TEXT_ALIGN_CENTER,
                  yellow_fab413, "5       ");
        char mode_str[9] = "";
        switch(last_state.channel.mode)
        {
            case FM:
            snprintf(mode_str, 9,"      FM");
            break;
            case DMR:
            snprintf(mode_str, 9,"     DMR");
            break;
        }
        gfx_print(pos_2, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white, mode_str);
        gfx_print(pos_2, layout.top_font, TEXT_ALIGN_RIGHT,
                  yellow_fab413, "6        ");
        gfx_print(pos_2, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white, "Lck");
        // Third row
        gfx_print(layout.line3_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  yellow_fab413, "7");
        gfx_print(layout.line3_pos, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white, "    B+");
        gfx_print(layout.line3_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  yellow_fab413, "8       ");
        gfx_print(layout.line3_pos, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white, "     B-");
        gfx_print(layout.line3_pos, layout.top_font, TEXT_ALIGN_RIGHT,
                  yellow_fab413, "9        ");
        gfx_print(layout.line3_pos, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white, "Sav");
        // Smeter bar
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
                       yellow_fab413);

        return true;
}
