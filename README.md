Read-only driver for GPS modules that supports: $GPRMC $GPVTG $GPGGA $GPGSA $GPGSV $GPGLL NMEA sentences


code:
```

int main(void){

gps_init();
extern NMEA_Data GPS;

  while (1){
    if(gps_is_data_ready()){
      gps_update_gps_data();
      gps_print_rx_buffer();
      printf("Loaction: %s\n", gps_complete_location_string());
      printf("Date: %s\n", gps_complete_date_string());
      printf("Time: %s\n", gps_complete_time_string(2));
      printf("SV visible: %d\n", gps_satellites_visible);
      printf("SV in use: %d\n", gps_satellites_in_use);
      printf("speed: %f km/h\n", gps_speed_kph);
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
#include <stdio.h>
#include "main.h"
#include "GPS.h"

int main(void)
{
  gps_init();
  extern NMEAData gps;  //only if you want to access raw data (GPS.latitude etc.) from main.c

  while (1) {
	  if (gps_is_data_ready()) {
		  gps_update_gps_data();
		  gps_print_rx_buffer();
		  printf("Location: %s\n", gps_complete_location_string());
		  printf("Date/Time: %s | %s\n", gps_complete_date_string(), gps_complete_time_string(2));
		  printf("Fix mode: %s\n", gps_fix_mode_string());
		  printf("GPS quality: %s\n", gps_quality_string());
      printf("sattelites in use: %s\n", gps.satellites_in_use);
	  }
  }
}
```

gps_init() - initiates an asynchronous reception of data from a UART

extern NMEAData gps - use only if you want to access raw data from main.c

gps_is_data_ready() - returns 1 when buffer is filled with data

gps_update_gps_data() - parses all nmea sentences and fills NMEAData struct elements with fresh data

gps_print_rx_buffer() - prints entire buffer

gps_complete_location_string() - returns string with location in readable form: XX.XXXXXX N, XX.XXXXXX E

gps.satellites_in_use - read data raw from struct


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
int gps_quality;
float pdop;
float hdop;
float vdop;
float ground_speed_knots;
float ground_speed_kph;
    

### Functions:

void gps_init(void);  - initiates an asynchronous reception of data from a UART
void gps_update_gps_data(void);  - decodes buffer into separate NMEA sentences and extracts data from each.

GPS_GetData decodes only following NMEA sentences:

- $GPRMC
- $GPVTG
- $GPGGA
- $GPGSA
- $GPGSV
- $GPGLL


int gps_is_data_ready(void);  - returns 1 when buffer is full and ready to decode

void gps_update_gps_data(void);  -  fills NMEAData elements with data

void gps_print_rx_buffer(void);  -  prints entire rx buffer

char* gps_complete_location_string(void);  - returns location as string (latitude lat_direction, longitude lon_direction)

float gps_latitude(void);  - returns latitude

char gps_lat_direction(void);  - returns latitude direction 

float gps_longitude(void);  - returns longitude

char gps_lon_direction(void);  - returns longitude direction

int gps_altitude(void);  - returns altitude

char* gps_complete_date_string(void);  - returns date as string (DD/MM/YYYY)

int gps_day(void);  - returns day

int gps_month(void);  - returns month

int gps_year(void); - returns year (YY)

int gps_year_long_format(void);  - returns year (YYYY)

char* gps_complete_time_string(int offset);  - returns time as string (HH:MM:SS)

int gps_hour(int offset);  - returns hour accounting for time zone difference

int gps_minute(void);  - returns minute

int gps_second(void);  - returns second

int gps_satellites_visible(void);  - returns ammount of visible satellites

int gps_satellites_in_use(void);  - returns ammount of connected satellites

char* gps_fix_mode_string(void);  - returns fix mode

char* gps_quality_string(void);  - returns GPS quality

float gps_pdop(void);  - returns PDOP

float gps_hdop(void);  - returns HDOP

float gps_vdop(void);  - returns VDOP

float gps_speed_knots(void);  - returns speed in knots

float gps_speed_kph(void);  - returns speed in km/h
