#ifndef GPS_H
#define GPS_H

#include <stdint.h>

typedef struct {
    float latitude;
    char lat_direction;
    float longitude;
    char lon_direction;
    float altitude;
    float time;
    uint32_t date;
    int  satellites_visible;
    int  satellites_in_use;
    int fix_mode;
    int GPS_quality;
    float PDOP;				//precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad |
    float HDOP;				//precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad |
    float VDOP;				//precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad |
    float ground_speed_knots;
    float ground_speed_kph;
} NMEA_Data;


void GPS_init(void);

void GPS_GetData(void);
int GPS_IsDataReady(void);
void GPS_PrintRXBuffer(void);
char* GPS_GetCompleteLocation(void);
float GPS_GetLatitude(void);
char GPS_GetLatDirection();
float GPS_GetLongitude(void);
char GPS_GetLonDirection();
int GPS_GetAltitude(void);
char* GPS_GetCompleteDate(void);
int GPS_GetDay(void);
int GPS_GetMonth(void);
int GPS_GetYear(void);
int GPS_GetYearLong(void);
char* GPS_GetCompleteTime(int offset);
int GPS_GetHour(int offset);
int GPS_GetMinute(void);
int GPS_GetSecond(void);
int GPS_GetSatellitesVisible(void);
int GPS_GetSatellitesInUse(void);
char* GPS_GetFixMode(void);
char* GPS_GetGPSQuality(void);
float GPS_GetPDOP(void);
float GPS_GetHDOP(void);
float GPS_GetVDOP(void);
float GPS_GetSpeedKnots(void);
float GPS_GetSpeedKph(void);

#endif /* GPS_H */
