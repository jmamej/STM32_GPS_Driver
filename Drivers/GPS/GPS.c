#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "GPS.h"
#include "main.h"

/* Uncomment to debug parsing functions */
//#define PRINT_DEBUGGER

extern UART_HandleTypeDef huart3;
NMEAData gps;

uint8_t received_byte;
uint8_t rx_buffer[550];
volatile uint16_t rx_buffer_index = 0;
uint8_t last_nmea_sentence_flag, gps_data_ready_flag;
const char *nmea_id[] = {"RMC", "VTG", "GGA", "GSA", "GSV", "GLL"};
char *substring_start[6] = {NULL};
char *substring_stop[6] = {NULL};

/* static local variable to store a single NMEA sentence */
static char nmea_sentence_rmc[64];
static char nmea_sentence_vtg[32];
static char nmea_sentence_gga[72];
static char nmea_sentence_gsa[64];
static char nmea_sentence_gsv[64];
static char nmea_sentence_gll[64];

/* static function prototypes */
static void parse_rmc(void);
static void parse_vtg(void);
static void parse_gga(void);
static void parse_gsa(void);
static void parse_gsv(void);
static void parse_gll(void);

/* gps_init initializes UART interrupt, allowing GPS data to be read */
void gps_init(void) {
    #define GPS_UART &huart3
    extern uint8_t received_byte;
    HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
}

void gps_update_gps_data(void) {
	for (int i = 0; i < NMEA_SENTENCES; i++) {
		substring_start[i] = strstr((char *)rx_buffer, nmea_id[i]) + 1;
		if (substring_start[i] != NULL) {
			substring_start[i] += strlen(nmea_id[i]);
			substring_stop[i] = strstr(substring_start[i], "*");
			if (substring_stop[i] != NULL) {
				substring_stop[i] += 1;
			} else {
				substring_stop[i] = (char *)rx_buffer + strlen((char *)rx_buffer);
			}
		}
	}

	parse_rmc();	/* populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | ground_speed_knots | date | */
	parse_vtg();	/* populates following variables with data: | ground_speed_knots | ground_speed_kph | */
	parse_gga();	/* populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | GPS_quality | satellites_in_use | HDOP | altitude | */
	parse_gsa();	/* populates following variables with data: | fix_mode | PDOP | HDOP | VDOP | */
	parse_gsv();	/* populates following variables with data: | satellites_visible | */
	parse_gll();	/* populates following variables with data: | latitude | lat_direction | longitude | lon_direction | time | */

	gps_data_ready_flag = 0;
}

int gps_is_data_ready(void) {
	return gps_data_ready_flag;
}

void gps_print_rx_buffer(void) {
	printf("\n----rxBuffer_START----\n\n");
	printf("%s\n", rx_buffer);
	printf("----rxBuffer_STOP----\n");
}

void parse_rmc(void) {
	snprintf(nmea_sentence_rmc, substring_stop[0] - substring_start[0], "%s", substring_start[0]);
	sscanf(nmea_sentence_rmc, "%f,%*c,%f,%c,%f,%c,%f,,%lu,,,%*c", &gps.time, &gps.latitude, &gps.lat_direction, &gps.longitude, &gps.lon_direction, &gps.ground_speed_knots, &gps.date);

#ifdef PRINT_DEBUGGER
	printf("RMC: %s\n", nmea_sentence_rmc);
	printf("Time: %f\n", gps.time);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("Speed knots: %f\n", gps.ground_speed_knots);
	printf("Date: %lu\n", gps.date);
	printf("\n");
#endif
}

void parse_vtg(void) {
	snprintf(nmea_sentence_vtg, substring_stop[1] - substring_start[1], "%s", substring_start[1]);
	sscanf(nmea_sentence_vtg, ",%*c,,%*c,%f,%*c,%f,%*c,%*c", &gps.ground_speed_knots, &gps.ground_speed_kph);

#ifdef PRINT_DEBUGGER
	printf("VTG: %s\n", nmea_sentence_vtg);
	printf("Speed knots: %f\n", gps.ground_speed_knots);
	printf("Speed km/h: %f\n", gps.ground_speed_kph);
	printf("\n");
#endif
}

void parse_gga(void) {
	snprintf(nmea_sentence_gga, substring_stop[2] - substring_start[2], "%s", substring_start[2]);
	sscanf(nmea_sentence_gga, "%f,%f,%c,%f,%c,%d,%d,%f,%f,%*c,%*f,%*c,,", &gps.time, &gps.latitude, &gps.lat_direction, &gps.longitude, &gps.lon_direction, &gps.gps_quality, &gps.satellites_in_use, &gps.hdop, &gps.altitude);

#ifdef PRINT_DEBUGGER
	printf("GGA: %s\n", nmea_sentence_gga);
	printf("Time: %f\n", gps.time);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("GPS quality: %d\n", gps.gps_quality);
	printf("satelites_in_use: %d\n", gps.satellites_in_use);
	printf("Precision (HDOP): %f\n", gps.hdop);
	printf("Altitude: %f\n", gps.altitude);
	printf("\n");
#endif
}

void parse_gsa(void) {
	snprintf(nmea_sentence_gsa, substring_stop[3] - substring_start[3], "%s", substring_start[3]);
	sscanf(nmea_sentence_gsa, "%*c,%d", &gps.fix_mode);
	/* Length of GSA may differ depending on amount of connected satellites, while loop searches for 14'th comma - where PDOP, HDOP, VDOP data starts */
	int coma_counter = 0;
	int index = 0;
	while(coma_counter != 14)	if(nmea_sentence_gsa[index++] == ',')	coma_counter++;
	sscanf(nmea_sentence_gsa + index, "%f,%f,%f", &gps.pdop, &gps.hdop, &gps.vdop);

#ifdef PRINT_DEBUGGER
	printf("GSA: %s\n", nmea_sentence_gsa);
	printf("Fix mode: %d\n", gps.fix_mode);	//1 - no fix | 2 - 2D | 3 - 3D |
	printf("PDOP: %f\n", gps.pdop);
	printf("HDOP: %f\n", gps.hdop);
	printf("VDOP: %f\n", gps.vdop);
	printf("\n");
#endif
}

void parse_gsv(void) {
	snprintf(nmea_sentence_gsv, substring_stop[4] - substring_start[4], "%s", substring_start[4]);
	sscanf(nmea_sentence_gsv, "%*d,%*d,%d", &gps.satellites_visible);

#ifdef PRINT_DEBUGGER
	printf("GSV: %s\n", nmea_sentence_gsv);
	printf("satellites_visible: %d\n", gps.satellites_visible);
	printf("\n");
#endif
}

void parse_gll(void) {
	snprintf(nmea_sentence_gll, substring_stop[5] - substring_start[5], "%s", substring_start[5]);
	sscanf(nmea_sentence_gll, "%f,%c,%f,%c,%f,%*c,%*c", &gps.latitude, &gps.lat_direction, &gps.longitude, &gps.lon_direction, &gps.time);

#ifdef PRINT_DEBUGGER
	printf("GLL: %s\n", nmea_sentence_gll);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("Time: %f\n", gps.time);
	printf("\n");
#endif
}

char* gps_complete_location_string(void) {
	static char buffer[23];
	sprintf(buffer, "%lf %c, %lf %c", gps_latitude(), gps_lat_direction(), gps_longitude(), gps_lon_direction());
	return buffer;
}

float gps_latitude(void) {
    int minutes = gps.latitude/100;
    float seconds = (gps.latitude-minutes*100)/60;
    return minutes+seconds;
}

char gps_lat_direction(void) {
	return	gps.lat_direction;
}

float gps_longitude(void) {
    int minutes = gps.longitude/100;
    float seconds = (gps.longitude-minutes*100)/60;
    return minutes+seconds;
}

char gps_lon_direction(void) {
	return	gps.lon_direction;
}

int gps_altitude(void) {
	return gps.altitude;
}

/* Time offset doesn't affect date. Date is always for UTC+0 */
char* gps_complete_date_string(void) {
	static char buffer[20];
	if(gps_day() < 10)		sprintf(buffer, "0%d/", gps_day());
	else					sprintf(buffer, "%d/", gps_day());
	if(gps_month() < 10)	sprintf(buffer + strlen(buffer), "0%d/", gps_month());
	else					sprintf(buffer + strlen(buffer), "%d/", gps_month());
							sprintf(buffer + strlen(buffer), "%d", gps_year_long_format());
	return buffer;
}

int gps_day(void) {
    return gps.date/10000;
}

int gps_month(void) {
    return (gps.date-gps_day()*10000)/100;
}

int gps_year(void) {
    return (gps.date-(gps_day()*10000)-(gps_month()*100));
}

int gps_year_long_format(void) {
    return (2000 + gps_year());
}

char* gps_complete_time_string(int offset) {
	static char buffer[20];
	if (gps_hour(offset) < 10)	sprintf(buffer, "0%d:", gps_hour(offset));
	else						sprintf(buffer, "%d:", gps_hour(offset));
	if (gps_minute() < 10)		sprintf(buffer + strlen(buffer), "0%d:", gps_minute());
	else						sprintf(buffer + strlen(buffer), "%d:", gps_minute());
	if (gps_second() < 10)		sprintf(buffer + strlen(buffer), "0%d", gps_second());
	else						sprintf(buffer + strlen(buffer), "%d", gps_second());
	return buffer;
}

int gps_hour(int offset) {
    int hour = gps.time / 10000;
    hour += offset;
    if (hour < 0) hour += 24;
    else if (hour >= 24) hour -= 24;
    return hour;
}

int gps_minute(void) {
    int hour = gps.time / 10000;
    return (gps.time - hour * 10000) / 100;
}

int gps_second(void) {
    int hour = gps.time / 10000;
    int minute = (gps.time - hour * 10000) / 100;
    return gps.time - hour * 10000 - minute * 100;
}

int gps_satellites_visible(void) {
	return gps.satellites_visible;
}

int gps_satellites_in_use(void) {
	return gps.satellites_in_use;
}

char* gps_fix_mode_string(void) {
	static char buffer[10];
    switch (gps.fix_mode) {
        case 1:
            strcpy(buffer, "NO FIX");
            break;
        case 2:
            strcpy(buffer, "2D FIX");
            break;
        case 3:
            strcpy(buffer, "3D FIX");
            break;
        default:
            strcpy(buffer, "FIX ERROR");
            break;
    }
	return buffer;
}

char* gps_quality_string(void) {
	static char buffer[20];
    switch (gps.gps_quality) {
        case 0:
            strcpy(buffer, "FIX NOT VALID");
            break;
        case 1:
            strcpy(buffer, "GPS FIX");
            break;
        case 2:
            strcpy(buffer, "DIFFERENTIAL FIX");
            break;
        case 3:
            strcpy(buffer, "NOT APPLICABLE");
            break;
        case 4:
            strcpy(buffer, "RTK FIXED");
            break;
        case 5:
            strcpy(buffer, "RTK FLOAT");
            break;
        case 6:
            strcpy(buffer, "INS DEAD RECKONING");
            break;
        default:
            strcpy(buffer, "GPS QUALITY ERROR");
            break;
    }
	return buffer;
}

float gps_pdop(void) {
	return gps.pdop;
}

float gps_hdop(void) {
	return gps.hdop;
}

float gps_vdop(void) {
	return gps.vdop;
}

float gps_speed_knots(void) {
	return gps.ground_speed_knots;
}

float gps_speed_kph(void) {
	return gps.ground_speed_kph;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == GPS_UART) {
		received_byte = huart->Instance->RDR;
		rx_buffer[rx_buffer_index++] = received_byte;
		if (received_byte == 'L')	last_nmea_sentence_flag = 1;
		else if (received_byte == '\n' && last_nmea_sentence_flag) {
			rx_buffer[rx_buffer_index++] = '\0';
			rx_buffer_index = 0;
			last_nmea_sentence_flag = 0;
			gps_data_ready_flag = 1;
		}
		HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
	}
}

