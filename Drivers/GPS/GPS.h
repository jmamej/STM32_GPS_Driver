#ifndef GPS_H
#define GPS_H

#include <stdint.h>

#define	GPS_UART	&huart1
#define NMEA_SENTENCES	6
#define LOWEST_BUFFER_SIZE	300
#define RX_BUFFER_SIZE  560
#define UART_IDLE_TIMEOUT	50

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
    int gps_quality;
    float pdop;				/* precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad | */
    float hdop;				/* precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad | */
    float vdop;				/* precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad | */
    float ground_speed_knots;
    float ground_speed_kph;
} NMEAData;


void gps_init(void);
int gps_get_data(void);
int gps_is_data_ready(void);
void gps_print_index(void);
void gps_print_rx_buffer(void);
char* gps_complete_location_string(void);
float gps_latitude(void);
char gps_lat_direction(void);
float gps_longitude(void);
char gps_lon_direction(void);
int gps_altitude(void);
char* gps_complete_date_string(void);
int gps_day(void);
int gps_month(void);
int gps_year(void);
int gps_year_long_format(void);
char* gps_complete_time_string(int offset);
int gps_hour(int offset);
int gps_minute(void);
int gps_second(void);
int gps_satellites_visible(void);
int gps_satellites_in_use(void);
char* gps_fix_mode_string(void);
char* gps_quality_string(void);
float gps_pdop(void);
float gps_hdop(void);
float gps_vdop(void);
float gps_speed_knots(void);
float gps_speed_kph(void);

#endif /* GPS_H */
