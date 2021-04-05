
#ifndef SATELLITE_H
#define SATELLITE_H

#define PI  3.141592653589793238462643383279
#define CENTURY 2000
#include <norad.h>

typedef struct sat_pos {
    double jd;  //julian day/time stamp
    float az;   //degrees
    float elev; //degrees
    /*float dist; //meters*/
} sat_pos;
//typedef struct _satpass {
//double rise_jd;
//double rise_az;
//float max_elev;
//double set_jd;
//double set_az;
//} satpass;

//typedef struct sat_pass {
//double start_jd;
//int duration_s; //only makes sense for LEO sats of course ...
//float rise_az;
//float set_az;
//float max_elev;
//} sat_pass;
//struct sat_predictions {
//double last_updated_jd;
//sat_pass passes[3];
//};
//struct satellite {
//char satcatid[16];
//tle_t tle;
//sat_predictions predictions;
//channel_t channels[];
//}
#define radial_vel_offset_sec 5  //was time_offset
//double time_offset = 5;

double curTime_to_julian_day(curTime_t t);
curTime_t julian_day_to_curTime( double jd );
double jd_to_j2k(double jd);
double local_sidereal_degrees(double j2k, double longitude );
double hour_angle_degrees( double local_sidereal_time, double right_ascension);
void ra_dec_to_az_alt(double jd,
                      double latitude, double longitude,
                      double ra, double dec,
                      double * az_out, double * alt_out);
int calcSat( tle_t tle, double time_jd,
             double lat, double lon, double height_meters,
             double * out_ra,
             double * out_dec,
             double * out_dist
           );
int isAboveHorizon( double time_jd, double lat, double lon, double ra, double dec );
point_t azel_deg_to_xy( float az_deg, float elev_deg, float radius);

point_t offset_point( point_t center, int num, ... );
double nextpass_jd(
    //sat in question
    tle_t tle,
    //start time
    double start_jd,
    //observer location
    double latd, double lond, double height_meters
);
#endif
