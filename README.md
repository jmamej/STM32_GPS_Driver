Read-only driver for GPS modules that supports: $GPRMC $GPVTG $GPGGA $GPGSA $GPGSV $GPGLL NMEA sentences


code:
```

int main(void){

GPS_init();
extern NMEA_Data GPS;

  while (1){
    if(GPS_IsDataReady()){
      GPS_GetData();
      GPS_PrintRXBuffer();
      printf("Loaction: %s\n", GPS_GetCompleteLocation());
      printf("Date: %s\n", GPS_GetCompleteDate());
      printf("Time: %s\n", GPS_GetCompleteTime(2));
      printf("SV visible: %d\n", GPS.satellites_visible);
      printf("SV in use: %d\n", GPS.satellites_in_use);
      printf("speed: %f km/h\n", GPS.ground_speed_kph);
    }
  }
}
```

output:

![13ef9868-ad7b-4de2-a570-496e1070e0f3](https://github.com/jmamej/STM32_GPS_Driver/assets/57408600/f47e9fdb-569a-4138-b41c-8f4e74cd808b)



## MX SETUP

![image](https://github.com/jmamej/STM32_GPS_Driver/assets/57408600/fb94b697-2a54-4e6a-b6c7-ed5db4ccdfb2)

![image](https://github.com/jmamej/STM32_GPS_Driver/assets/57408600/3024664d-136f-42b1-82bd-9d43c1e60896)

## printf scanf error fix

![image](https://github.com/jmamej/STM32_GPS_Driver/assets/57408600/46b2811c-2cd3-4530-a79a-8726a7bab8a5)


check both boxes:
- use float with printf
- use float with scanf


## main.c

```
#include "main.h"
#include "GPS.h"
#include <stdio.h>

int main(void)
{
  GPS_init();
  extern NMEA_Data GPS;  //only if you want to access raw data (GPS.latitude etc.) from main.c

  while (1){
    if(GPS_IsDataReady()){
      GPS_GetData();
      GPS_PrintRXBuffer();
      printf("Loaction: %s\n", GPS_GetCompleteLocation());
      printf("speed: %f km/h\n", GPS.ground_speed_kph);
    }
  }
}
```

GPS_init() - initiates an asynchronous reception of data from a UART

extern NMEA_Data GPS - use only if you want to access raw data from main.c

GPS_IsDataReady() - returns 1 when buffer is filled with data

GPS_PrintRXBuffer() - prints entire buffer

GPS_GetCompleteLocation() - returns string with location in readable form: GPS_GetLatitude() GPS_GetLatDirection(), GPS_GetLongitude() GPS_GetLonDirection()

GPS.ground_speed_kph - raw data extracted from buffer


### Raw data:


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

float PDOP;

float HDOP;

float VDOP;

float ground_speed_knots;

float ground_speed_kph;
    

### API's:

void GPS_init(void);  - initiates an asynchronous reception of data from a UART
void GPS_GetData(void);  - decodes buffer into separate NMEA sentences and extracts data from each.

GPS_GetData decodes only following NMEA sentences:

- $GPRMC
- $GPVTG
- $GPGGA
- $GPGSA
- $GPGSV
- $GPGLL

int GPS_IsDataReady(void);  - returns 1 when buffer is full and ready to decode

void GPS_PrintRXBuffer(void);  - prints entire buffer

char* GPS_GetCompleteLocation(void);  - returns location as string (latitude lat_direction, longitude lon_direction)

float GPS_GetLatitude(void);  - returns latitude

char GPS_GetLatDirection();  - returns lat_direction

float GPS_GetLongitude(void);  - returns longitude

char GPS_GetLonDirection();  - returns lon_direction

int GPS_GetAltitude(void);  - returns altitude

char* GPS_GetCompleteDate(void);  - returns date as string (DD/MM/YYYY)

int GPS_GetDay(void);  - returns day

int GPS_GetMonth(void);  - returns month

int GPS_GetYear(void); - returns year (YY)

int GPS_GetYearLong(void);  - returns year (YYYY)

char* GPS_GetCompleteTime(int offset);  - returns time as string (HH:MM:SS)

int GPS_GetHour(int offset);  - returns hour accounting for time zone difference

int GPS_GetMinute(void);  - returns minute

int GPS_GetSecond(void);  - returns second

int GPS_GetSatellitesVisible(void);  - returns ammount of visible satellites

int GPS_GetSatellitesInUse(void);  - returns ammount of connected satellites

char* GPS_GetFixMode(void);  - returns fix mode

char* GPS_GetGPSQuality(void);  - returns GPS quality

float GPS_GetPDOP(void);    - returns PDOP

float GPS_GetHDOP(void);  - returns HDOP

float GPS_GetVDOP(void);  - returns VDOP

float GPS_GetSpeedKnots(void);  - returns speed in knots

float GPS_GetSpeedKph(void);  - returns speed in km/h




