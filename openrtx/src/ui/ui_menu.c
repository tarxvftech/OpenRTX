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

#include <math.h>
#include <maidenhead.h>
#include <norad.h>
#include <stdlib.h> //for abs()
#include <observe.h>
#define PI  3.141592653589793238462643383279
#define CENTURY 2000
extern void _ui_drawMainBackground();
extern void _ui_drawBottom();

double curTime_to_julian_day(curTime_t t){
  //expects t to be after year 2000
  //many thanks to Peter Baum, and his "Date Algorithms" reference.
  uint8_t s = t.second; //.day is the _weekday_, date is day-of-month
  uint8_t m = t.minute; //.day is the _weekday_, date is day-of-month
  uint8_t h = t.hour; //.day is the _weekday_, date is day-of-month
  uint8_t D = t.date; //.day is the _weekday_, date is day-of-month
  uint8_t M = t.month;
  short Y = CENTURY + t.year;
  /*printf("%04d/%02d/%02d  %02d:%02d:%02d\n", Y,M,D,h,m,s);*/

  int Z = Y + (M-14)/12; //relies on int truncation
  const short Fvec[] = {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275 };
  short F = Fvec[M-1];

  //! note difference between floor and int truncation
  //floor(-1.5) => -2
  //int(-1.5) => -1
  double jd = D + F + 365*Z + floor(Z/4) - floor(Z/100) + floor(Z/400) + 1721118.5; 
  //that +.5 is because julian .0 is actually _noon_, not midnight 
  //so JD #.5 is _halfway through that julian day_, at _midnight_
  //so we add hours, minutes, and seconds into a fractional day and add that to our new "midnight" epoch and...
  jd += ((float)h)/24 + ((float)m)/1440 + ((float)s)/86400;
  //voila!

  /*printf("jd: %.6f\n",jd);*/
  return jd;
}
void test_curTime_to_julian_day(){
  curTime_t t;

}
curTime_t julian_day_to_curTime( double jd ){
  //expects jd to be after year 2000
  curTime_t t;
  long Z = jd - 1721118.5;
  double R = jd - 1721118.5 - Z;
  double G = Z - .25;
  //TODO unfinished:
  /*
  A = INT(G / 36524.25)
  B = A -INT(A / 4)
  year = INT((B+G) / 365.25)
  C = B + Z -INT(365.25 * year)
  month = FIX((5 * C + 456) / 153)
  day = C -FIX((153 * month -457) / 5) + R
  if(month>12){year++;month-=12;}
  */

  /*t.second = 0;*/
  /*t.minute = 0;*/
  /*t.hour = 0;*/

  /*t.date = D;*/
  /*t.month = M;*/
  /*t.year = 2000-Y;*/
  return t;

}
double jd_to_j2k(double jd){
  return jd - 2451545.0;
}
double wrap( double in, double max){
   while( in < 0 ){
      in += max;
   }
   while( in >= max ){
      in -= max;
   }
   return in;
}
double local_sidereal_degrees(double j2k, double longitude ){
   double UT = 24*(j2k - floor(j2k) + .5);
   /*printf("J2K: %.6f \n", j2k);*/
   /*printf("UT: %.6f \n", UT);*/
   double degrees_rotation_per_day = .985647;
   double gmst_j2k_correction = 100.46;
   double lond = longitude * 180/PI;
   double local_sidereal_time = gmst_j2k_correction + degrees_rotation_per_day * j2k + lond + 15*UT;
   local_sidereal_time = wrap(local_sidereal_time, 360);
   return local_sidereal_time;
}
double hour_angle_degrees( double local_sidereal_time, double right_ascension){
   double ha = local_sidereal_time - right_ascension;
   ha = wrap( ha, 360 );
   return ha;
}

void ra_dec_to_az_alt(double jd,
      double latitude, double longitude,
      double ra, double dec,
      double * az_out, double * alt_out){

   double j2k = jd_to_j2k(jd);
   double lst = local_sidereal_degrees(j2k, longitude); //✔
   /*printf("lst %.4f\n", lst);*/
   double ha = hour_angle_degrees(lst, ra*180/PI) * PI/180;

   latitude = latitude;

   double A = cos(ha)*cos(dec)*cos(latitude) - sin(dec)*sin(latitude);
   double B = sin(ha)*cos(dec);
   double C = cos(ha)*cos(dec)*sin(latitude) + sin(dec)*cos(latitude);
   double az = atan2(B,A)+PI;
   double alt = asin(C);

   az = wrap(az, 2*PI);

   *az_out = az;
   *alt_out = alt;
   return;
}


int calcSat( tle_t tle, double time_jd, 
    double lat, double lon, double height_meters,
    double * out_ra,
    double * out_dec,
    double * out_dist
      ){
   int is_deep = select_ephemeris( &tle );
   int ephem = 1;       /* default to SGP4 */
   double sat_params[N_SAT_PARAMS], observer_loc[3];
   double rho_sin_phi;
   double rho_cos_phi;

   double ra;
   double dec;
   double dist_to_satellite;

   double t_since;
   double pos[3];
   double vel[3];
   int err_val = 0;

   //remember to put lat and lon in rad by this point
   earth_lat_alt_to_parallax( lat, height_meters, &rho_cos_phi, &rho_sin_phi);
   observer_cartesian_coords( time_jd, lon , rho_cos_phi, rho_sin_phi, observer_loc);
   if( is_deep && (ephem == 1 || ephem == 2)){
      ephem += 2;
   }
   if( !is_deep && (ephem == 3 || ephem == 4)){
      ephem -= 2;
   }

   t_since = (time_jd - tle.epoch) * 1440;
   switch( ephem) {
      case 0:
         SGP_init( sat_params, &tle);
         err_val = SGP( t_since, &tle, sat_params, pos, NULL);
         break;
      case 1:
         SGP4_init( sat_params, &tle);
         err_val = SGP4( t_since, &tle, sat_params, pos, NULL);
         break;
      case 2:
         SGP8_init( sat_params, &tle);
         err_val = SGP8( t_since, &tle, sat_params, pos, NULL);
         break;
      case 3:
         SDP4_init( sat_params, &tle);
         err_val = SDP4( t_since, &tle, sat_params, pos, NULL);
         break;
      case 4:
         SDP8_init( sat_params, &tle);
         err_val = SDP8( t_since, &tle, sat_params, pos, NULL);
         break;
      default:
         printf( "? How did we get here? ephem = %d\n", ephem);
         err_val = 0;
         break;
   }
   if( err_val ){
      printf( "Ephemeris error %d\n", err_val);
   }
   get_satellite_ra_dec_delta( observer_loc, pos, &ra, &dec, &dist_to_satellite);
   epoch_of_date_to_j2000( time_jd, &ra, &dec);
   /*printf("POS: %.4f,%.4f,%.4f\n", pos[0], pos[1], pos[2] );*/
   /*printf("VEL: %.4f,%.4f,%.4f\n", vel[0], vel[1], vel[2] );*/
   *out_ra=ra;
   *out_dec=dec;
   *out_dist=dist_to_satellite;
   return err_val;
}
int isAboveHorizon( double time_jd, double lat, double lon, double ra, double dec ){
   double az;
   double alt;
   ra_dec_to_az_alt(time_jd, lat, lon, ra, dec, &az, &alt);
   return alt >= 0;
}
typedef struct _satpass {
  double rise_jd;
  double rise_az;
  float max_elev;
  double set_jd;
  double set_az;
} satpass;

double nextpass_jd(
      //sat in question
      tle_t tle,
      //start time
      double start_jd,
      //observer location
      double latd, double lond, double height_meters
      ){
      //search end time and sampling period from TLE data
      //determine sampling from TLE parse, which should include orbit period
      //getting a list of passes involves successive calls to this function

   double jd = start_jd;
   double lat = latd*PI/180;
   double lon = lond*PI/180;


   int aboveHorizonAtJD = 0;
   int i = 0;
   while( jd < start_jd + .25 ){ //search next N day(s)
      jd += .000348; //increment by ~30s
      double ra;
      double dec;
      double dist;
      calcSat( tle, jd, lat, lon, height_meters, &ra, &dec, &dist);
      aboveHorizonAtJD = isAboveHorizon( jd, lat, lon, ra, dec );
      i++;
      if( aboveHorizonAtJD ){
         break;
      }
   }
   while( aboveHorizonAtJD ){
      double ra;
      double dec;
      double dist;
      calcSat( tle, jd, lat, lon, height_meters, &ra, &dec, &dist);
      aboveHorizonAtJD = isAboveHorizon( jd, lat, lon, ra, dec );
      i++;
      //could speed this up by bisect bracketing the transition from - to +
      //should also find the end of the pass going + to -
      jd -= .00010; // <1s
   }
   printf("found in %d iterations\n", i);
   return jd;
}





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
            gfx_print(pos, entry_buf, layout.menu_font, TEXT_ALIGN_LEFT, text_color);
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
            gfx_print(pos, entry_buf, layout.menu_font, TEXT_ALIGN_LEFT, text_color);
            gfx_print(pos, value_buf, layout.menu_font, TEXT_ALIGN_RIGHT, text_color);
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
        case 0: // Battery voltage
            snprintf(buf, max_len, "%.1fV", last_state.v_bat);
            break;
        case 1: // Battery charge
            snprintf(buf, max_len, "%.1f%%", last_state.charge * 100);
            break;
        case 2: // RSSI
            snprintf(buf, max_len, "%.1fdBm", last_state.rssi);
            break;
        case 3: // Model
            snprintf(buf, max_len, "%s", hwinfo->name);
            break;
        case 4: // Band
            snprintf(buf, max_len, "%s %s", hwinfo->vhf_band ? "VHF" : "", hwinfo->uhf_band ? "UHF" : "");
            break;
        case 5: // VHF
            snprintf(buf, max_len, "%d - %d", hwinfo->vhf_minFreq, hwinfo->vhf_maxFreq);
            break;
        case 6: // UHF
            snprintf(buf, max_len, "%d - %d", hwinfo->uhf_minFreq, hwinfo->uhf_maxFreq);
            break;
        case 7: // LCD Type
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
    gfx_print(layout.top_pos, "Menu", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print menu entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getMenuTopEntryName);
}

void _ui_drawMenuZone(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Zone" on top bar
    gfx_print(layout.top_pos, "Zone", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print zone entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getZoneName);
}

void _ui_drawMenuChannel(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Channel" on top bar
    gfx_print(layout.top_pos, "Channels", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print channel entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getChannelName);
}

void _ui_drawMenuContacts(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Contacts" on top bar
    gfx_print(layout.top_pos, "Contacts", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print contact entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getContactName);
}

#ifdef HAS_GPS
void _ui_drawMenuGPS()
{
    char *fix_buf, *type_buf;
    char lat_buf[12] = { 0 };
    char lon_buf[12] = { 0 };
    char data_buf[25] = { 0 };
    gfx_clearScreen();
    // Print "GPS" on top bar
    gfx_print(layout.top_pos, "GPS", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    point_t fix_pos = {layout.line2_pos.x, SCREEN_HEIGHT * 2 / 5};
    // Print GPS status, if no fix, hide details
    if(!last_state.settings.gps_enabled)
        gfx_print(fix_pos, "GPS OFF", layout.line3_font, TEXT_ALIGN_CENTER, color_white);
    else if (last_state.gps_data.fix_quality == 0)
        gfx_print(fix_pos, "No Fix", layout.line3_font, TEXT_ALIGN_CENTER, color_white);
    else if (last_state.gps_data.fix_quality == 6)
        gfx_print(fix_pos, "Fix Lost", layout.line3_font, TEXT_ALIGN_CENTER, color_white);
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
        gfx_print(layout.line1_pos, fix_buf, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white);
        gfx_print(layout.line1_pos, "N     ", layout.top_font, TEXT_ALIGN_CENTER,
                  color_white);
        snprintf(lat_buf, 12, "%8.6f", last_state.gps_data.latitude);
        gfx_print(layout.line1_pos, lat_buf, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white);
        gfx_print(layout.line2_pos, type_buf, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white);
        // Convert from signed longitude, to unsigned + direction
        float longitude = last_state.gps_data.longitude;
        char *direction = (longitude < 0) ? "W     " : "E     ";
        longitude = (longitude < 0) ? -longitude : longitude;
        gfx_print(layout.line2_pos, direction, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white);
        snprintf(lon_buf, 12, "%8.6f", longitude);
        gfx_print(layout.line2_pos, lon_buf, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white);
        snprintf(data_buf, 25, "S %4.1fkm/h  A %4.1fm",
                 last_state.gps_data.speed,
                 last_state.gps_data.altitude);
        gfx_print(layout.bottom_pos, data_buf, layout.bottom_font, TEXT_ALIGN_CENTER,
                  color_white);
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
void _ui_drawMenuSAT()
{
    
    char sbuf[25] = { 0 };
    gfx_clearScreen();

    if( 0 ){ //! last_state.settings.gps_enabled || last_state.gps_data.fix_quality == 0 ){
      //fix_type is 1 sometimes when it shouldn't be, have to use fix_quality
      gfx_print(layout.line3_pos, "no gps fix", FONT_SIZE_12PT, TEXT_ALIGN_CENTER, color_white);
      return;
    }
    /*float lat = last_state.gps_data.latitude;*/
    float lat =  41.70011;
    /*float lon = last_state.gps_data.longitude;*/
    float lon = -70.29947;
    /*float alt = last_state.gps_data.altitude; //msl geoid meters*/
    float alt = 0; //msl geoid meters
    latlon pos;
    pos.lat = lat;
    pos.lon = lon;
    char gridsquare[7] = {0};
    latlon_to_maidenhead_locator(pos, gridsquare, 3);
    gfx_print(layout.line2_pos, gridsquare, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);

    tle_t tle; 
    //replace this with a system of storing 
    //tle_t's in codeplug area later
    //
    //iss
    char * line1 = "1 25544U 98067A   21090.24132166  .00001671  00000-0  38531-4 0  9996";
    char * line2 = "2 25544  51.6473   8.2465 0003085 162.7840 356.2009 15.48967005276518";

    //qo-100
    /*char * line1 = "1 43700U 18090A   21091.25978243  .00000133  00000-0  00000+0 0  9991";*/
    /*char * line2 = "2 43700   0.0249 193.2167 0002611 211.4635 264.4585  1.00269262  8833";*/
    int err_val = parse_elements( line1, line2, &tle );

    double jd = curTime_to_julian_day(last_state.time);
    //decimal time in hours
    double UT = (double)last_state.time.hour + ((double)last_state.time.minute)/60 + ((double) last_state.time.second)/3600;

    /*printf("JD %.6f\r\n", jd);*/
    snprintf(sbuf, 25, "JD %.6f", jd);
    gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);

    //TODO:
    //drawing system for illustrating a pass?
    //need a satellite selection view
    //  and then eventually
    //a pass list view
    //a pass detail view
    //details we care about: most important, time and max elev
    //rise time, duration, max elev, start and stop azimuths

    double latr = lat*PI/180; //sat code works in radians
    double lonr = lon*PI/180; //so r for radians
    double ra = 0;
    double dec = 0;

    double dist = 0;
    double ra2 = 0;
    double dec2 = 0;
    double dist2 = 0;

    double time_offset = 5;
    double toff_jd = time_offset/(24*60*60);
    calcSat( tle, jd, latr, lonr, alt, &ra, &dec, &dist);
    calcSat( tle, jd + toff_jd, latr, lonr, alt, &ra2, &dec2, &dist2);
    float toff_dist = dist-dist2; //km difference
    float radial_v = (toff_dist)/time_offset; //km/s
    float freq = 433e6; //MHz
    int doppler_offset = freq*radial_v/300000; //hz
    
    /*printf("toff_jd: %.20f\r\n", toff_jd);*/
    /*printf("dd: %.20f\r\n", toff_dist);*/
    /*printf("doppler_offset: %d Hz\r\n", doppler_offset);*/
    /*snprintf(sbuf, 25, "rv %.1f", radial_v);*/
    /*gfx_print(layout.line2_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);*/
    /*snprintf(sbuf, 25, "dec %.6f", dec);*/
    /*gfx_print(layout.line2_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);*/

    double az = 0;
    double elev = 0;
    ra_dec_to_az_alt(jd, latr, lonr, ra, dec, &az, &elev);
    //replace with lunars:
    //void DLL_FUNC full_ra_dec_to_alt_az( const DPT DLLPTR *ra_dec,
    /*          DPT DLLPTR *alt_az,*/
    /*          DPT DLLPTR *loc_epoch, const DPT DLLPTR *latlon,*/
    /*          const double jd_utc, double DLLPTR *hr_ang)*/

    elev *= 180/PI; //radians to degrees
    az *= 180/PI;

    /*double pass = nextpass_jd(tle, jd, lat, lon, alt ); //works in degrees*/
    /*int pass_diff = (pass - UT)*3600; //diff is seconds until (+) or since (-) the mark timestamp*/

    //I've been using this to keep an eye on alignment, but remove later
    gfx_drawVLine(SCREEN_WIDTH/2, 1, color_grey);

    double pass_start_jd = 2459306.456495;
    double pass_duration_jd = .025; //~90s
    double pass_end_jd = pass_start_jd + pass_duration_jd; 

    char * pass_state;
    double mark = pass_start_jd; 
    //mark is the time we care about most - e.g. before a pass, it's the time the pass starts
    //or during a pass, it's the LOS time (when the sat will go below the horizon)
    if( UT < pass_start_jd ){
        //before this pass comes over the horizon
        pass_state = "AOS";
        mark = pass_start_jd;
    } else if ( UT > pass_start_jd && UT < pass_end_jd ){
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
    int diff = (mark - UT)*3600; //diff is seconds until (+) or since (-) the mark timestamp


    const char * sat_name = "ISS";
    /*gfx_print(layout.top_pos, sat_name, FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);*/
    if( diff > 60 || diff < -60 ){ 
        //if we're too far before or after the mark time, it
        //makes more sense to show something else rather than, say, 12387123 seconds
        //therefore print the time of the mark time
        int mark_h = 0;
        int mark_m = 0;
        snprintf(sbuf, 25, "%s %s %02d:%02d", sat_name, pass_state, mark_h, mark_m);
        /*gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT,*/
                /*TEXT_ALIGN_CENTER, color_white);*/
    } else {
        //will look like these examples:
        //"AOS 30s" meaning it'll come over the horizon in 30s
        //or "LOS 90s" meaning it's in your sky now and will be back over the horizon in 90s
        //or "LOS -30s" meaning it went over the horizon 30 seconds ago
        snprintf(sbuf, 25, "%s %s %ds", sat_name, pass_state, diff);
        /*gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT,*/
                /*TEXT_ALIGN_CENTER, color_white);*/
    }
    /*snprintf(sbuf, 25, "diff %d", diff);*/
    /*gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);*/


    snprintf(sbuf, 25, "AZ %.1f", az);
    gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);
    snprintf(sbuf, 25, "EL %.1f", elev);
    gfx_print(layout.line2_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_LEFT, color_white);
    snprintf(sbuf, 25, "%.1fk DOP", ((float)doppler_offset)/1000);
    gfx_print(layout.line1_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_RIGHT, color_white);

    snprintf(sbuf, 25, "%02d:%02d:%02d", last_state.time.hour, last_state.time.minute, last_state.time.second);
    gfx_print(layout.top_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);


    _ui_drawMainBackground(); //causes a crash on entering this menu
    uint16_t bat_width = SCREEN_WIDTH / 9;
    uint16_t bat_height = layout.top_h - (layout.status_v_pad * 2);
    point_t bat_pos = {SCREEN_WIDTH - bat_width - layout.horizontal_pad,
                       layout.status_v_pad};
    gfx_drawBattery(bat_pos, bat_width, bat_height, last_state.charge);


    _ui_drawBottom();

    return;
}
#endif

void _ui_drawMenuSettings(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Settings" on top bar
    gfx_print(layout.top_pos, "Settings", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print menu entries
    _ui_drawMenuList(ui_state->menu_selected, _ui_getSettingsEntryName);
}

void _ui_drawMenuInfo(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Info" on top bar
    gfx_print(layout.top_pos, "Info", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
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
        gfx_print(openrtx_pos, "OpenRTX", layout.line3_font, TEXT_ALIGN_CENTER, color_white);
    char author_buf[MAX_ENTRY_LEN] = "";
    uint8_t line_h = layout.menu_h;
    point_t pos = {SCREEN_WIDTH / 7, SCREEN_HEIGHT - (line_h * (author_num - 1)) - 5};
    for(int author = 0; author < author_num; author++)
    {
        snprintf(author_buf, MAX_ENTRY_LEN, "%s", authors[author]);
        gfx_print(pos, author_buf, layout.top_font, TEXT_ALIGN_LEFT, color_white);
        pos.y += line_h;
    }
}

void _ui_drawSettingsDisplay(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "Display" on top bar
    gfx_print(layout.top_pos, "Display", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print display settings entries
    _ui_drawMenuListValue(ui_state, ui_state->menu_selected, _ui_getDisplayEntryName,
                           _ui_getDisplayValueName);
}

#ifdef HAS_GPS
void _ui_drawSettingsGPS(ui_state_t* ui_state)
{
    gfx_clearScreen();
    // Print "GPS Settings" on top bar
    gfx_print(layout.top_pos, "GPS Settings", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
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
    // Print "Time&Date" on top bar
    gfx_print(layout.top_pos, "Time&Date", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
    // Print current time and date
    char date_buf[10] = "";
    char time_buf[10] = "";
    snprintf(date_buf, sizeof(date_buf), "%02d/%02d/%02d", 
             last_state.time.date, last_state.time.month, last_state.time.year);
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", 
             last_state.time.hour, last_state.time.minute, last_state.time.second);
    gfx_print(layout.line2_pos, date_buf, layout.input_font, TEXT_ALIGN_CENTER,
              color_white);
    gfx_print(layout.line3_pos, time_buf, layout.input_font, TEXT_ALIGN_CENTER,
              color_white);
}

void _ui_drawSettingsTimeDateSet(ui_state_t* ui_state)
{
    (void) last_state;

    gfx_clearScreen();
    // Print "Time&Date" on top bar
    gfx_print(layout.top_pos, "Time&Date", layout.top_font,
              TEXT_ALIGN_CENTER, color_white);
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
    gfx_print(layout.line2_pos, ui_state->new_date_buf, layout.input_font, TEXT_ALIGN_CENTER,
              color_white);
    gfx_print(layout.line3_pos, ui_state->new_time_buf, layout.input_font, TEXT_ALIGN_CENTER,
              color_white);
}
#endif

bool _ui_drawMacroMenu() {
        // Header
        gfx_print(layout.top_pos, "Macro Menu", layout.top_font, TEXT_ALIGN_CENTER,
                  color_white);
        // First row
        gfx_print(layout.line1_pos, "1", layout.top_font, TEXT_ALIGN_LEFT,
                  yellow_fab413);
        char code_str[11] = { 0 };
        snprintf(code_str, 11, "  %6.1f",
                 ctcss_tone[last_state.channel.fm.txTone]/10.0f);
        gfx_print(layout.line1_pos, code_str, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white);
        gfx_print(layout.line1_pos, "2       ", layout.top_font, TEXT_ALIGN_CENTER,
                  yellow_fab413);
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
        gfx_print(layout.line1_pos, encdec_str, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white);
        gfx_print(layout.line1_pos, "3        ", layout.top_font, TEXT_ALIGN_RIGHT,
                  yellow_fab413);
        char pow_str[9] = { 0 };
        snprintf(pow_str, 9, "%.1gW", last_state.channel.power);
        gfx_print(layout.line1_pos, pow_str, layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white);
        // Second row
        // Calculate symmetric second row position, line2_pos is asymmetric like main screen
        point_t pos_2 = {layout.line1_pos.x, layout.line1_pos.y + (layout.line3_pos.y - layout.line1_pos.y)/2};
        gfx_print(pos_2, "4", layout.top_font, TEXT_ALIGN_LEFT,
                  yellow_fab413);
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
        gfx_print(pos_2, bw_str, layout.top_font, TEXT_ALIGN_LEFT,
                  color_white);
        gfx_print(pos_2, "5       ", layout.top_font, TEXT_ALIGN_CENTER,
                  yellow_fab413);
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
        gfx_print(pos_2, mode_str, layout.top_font, TEXT_ALIGN_CENTER,
                  color_white);
        gfx_print(pos_2, "6        ", layout.top_font, TEXT_ALIGN_RIGHT,
                  yellow_fab413);
        gfx_print(pos_2, "Lck", layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white);
        // Third row
        gfx_print(layout.line3_pos, "7", layout.top_font, TEXT_ALIGN_LEFT,
                  yellow_fab413);
        gfx_print(layout.line3_pos, "    B+", layout.top_font, TEXT_ALIGN_LEFT,
                  color_white);
        gfx_print(layout.line3_pos, "8       ", layout.top_font, TEXT_ALIGN_CENTER,
                  yellow_fab413);
        gfx_print(layout.line3_pos, "     B-", layout.top_font, TEXT_ALIGN_CENTER,
                  color_white);
        gfx_print(layout.line3_pos, "9        ", layout.top_font, TEXT_ALIGN_RIGHT,
                  yellow_fab413);
        gfx_print(layout.line3_pos, "Sav", layout.top_font, TEXT_ALIGN_RIGHT,
                  color_white);
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
