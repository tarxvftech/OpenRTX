#include <math.h>
#include <maidenhead.h>
#include <norad.h>
#include <stdlib.h> //for abs()
#include <observe.h>

#define PI  3.141592653589793238462643383279
#define CENTURY 2000

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




    struct misc_for_sat_tracking {
      char manual_gridsquare[9]; //to be converted to latlon for positioning when gps is missing or still warming up
      /*tle_t * tle; //pointer to */
      //julian day, to be recalculculated every second when rtc is queried
      /*double jd = curTime_to_julian_day(last_state.time);*/
      //decimal time in hours
      /*double UT = (double)last_state.time.hour + ((double)last_state.time.minute)/60 + ((double) last_state.time.second)/3600;*/
    };
    struct sat_pass {
      double start_jd;
      int duration_s; //only makes sense for LEO sats of course ... 
      float rise_az;
      float set_az;
      float max_elev;
    };
    struct sat_predictions { 
      char satcatid[16];
      double last_updated_jd;
      tle_t * tle;
      sat_pass passes[3];
    };
#define radial_vel_offset_sec 5  //was time_offset
      /*double time_offset = 5;*/

void x(){
    tle_t tle; 
    //replace this with a system of storing 
    //tle_t's in codeplug area later
    //
    //iss
    char * line1 = "1 25544U 98067A   21090.24132166  .00001671  00000-0  38531-4 0  9996";
    char * line2 = "2 25544  51.6473   8.2465 0003085 162.7840 356.2009 15.48967005276518";

    int err_val = parse_elements( line1, line2, &tle );

    /*printf("JD %.6f\r\n", jd);*/
    /*snprintf(sbuf, 25, "JD %.6f", jd);*/
    /*gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT, TEXT_ALIGN_CENTER, color_white);*/

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

    double toff_jd = time_offset/(24*60*60);
    calcSat( tle, jd, latr, lonr, alt, &ra, &dec, &dist);
    calcSat( tle, jd + toff_jd, latr, lonr, alt, &ra2, &dec2, &dist2);
    float toff_dist = dist-dist2; //km difference
    float radial_v = (toff_dist)/time_offset; //km/s
    float freq = 433e6; //MHz
    int doppler_offset = freq*radial_v/300000; //hz

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

    if( diff > 60 || diff < -60 ){ 
        //if we're too far before or after the mark time, it
        //makes more sense to show something else rather than, say, 12387123 seconds
        //therefore print the time of the mark time
        int mark_h = 0;
        int mark_m = 0;
        snprintf(sbuf, 25, "%s %s %02d:%02d", sat_name, pass_state, mark_h, mark_m);
        gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT,
                TEXT_ALIGN_CENTER, color_white);
    } else {
        //will look like these examples:
        //"AOS 30s" meaning it'll come over the horizon in 30s
        //or "LOS 90s" meaning it's in your sky now and will be back over the horizon in 90s
        //or "LOS -30s" meaning it went over the horizon 30 seconds ago
        snprintf(sbuf, 25, "%s %s %ds", sat_name, pass_state, diff);
        gfx_print(layout.line3_pos, sbuf, FONT_SIZE_8PT,
                TEXT_ALIGN_CENTER, color_white);
    }

}
