#include "../GPS/GPS.h"

#include "main.h"
#include <usart.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#define PRINT_DEBUGGER
//#define PRINT_BUFFER
//#define PRINT_PARSED_DATA
//#define PRINT_COMPLETE_DATA

uint8_t received_byte;
uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_buffer_index = 0;
uint8_t gps_data_ready_flag;
uint32_t rx_data_timer = 0;

const char *nmea_id[] = {"RMC", "VTG", "GGA", "GSA", "GSV", "GLL"};
char *substring_start[NMEA_SENTENCES] = {NULL};
uint8_t substring_length[NMEA_SENTENCES] = {0};

NMEAData gps;

static void parse_rmc(void);
static void parse_vtg(void);
static void parse_gga(void);
static void parse_gsa(void);
static void parse_gsv(void);
static void parse_gll(void);

void gps_init(void){
	HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
}

int gps_get_data(void){
	if(!gps_is_data_ready())	return 0;

	for (int i = 0; i < NMEA_SENTENCES; i++) {
		substring_start[i] = strstr((char *)rx_buffer, nmea_id[i]) + 1;
		if (substring_start[i] != NULL) {
			substring_start[i] += strlen(nmea_id[i]);
			substring_length[i] = strstr(substring_start[i], "*") - substring_start[i];
			if (substring_length[i] > 0) {
				substring_length[i] += 1;
			} else {
				substring_length[i] = strlen(substring_start[i]);
			}
		}
	}

	parse_rmc();	/* populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | ground_speed_knots | date | */
	parse_vtg();	/* populates following variables with data: | ground_speed_knots | ground_speed_kph | */
	parse_gga();	/* populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | GPS_quality | satellites_in_use | HDOP | altitude | */
	parse_gsa();	/* populates following variables with data: | fix_mode | PDOP | HDOP | VDOP | */
	parse_gsv();	/* populates following variables with data: | satellites_visible | */
	parse_gll();	/* populates following variables with data: | latitude | lat_direction | longitude | lon_direction | time | */

#ifdef PRINT_BUFFER
	gps_print_rx_buffer();
#endif
#ifdef PRINT_PARSED_DATA
	printf("LOC:  %s\n", gps_complete_location_string());
	printf("TIME: %s\n", gps_complete_time_string(0));
	printf("DATE: %s\n", gps_complete_date_string());
#endif
#ifdef PRINT_COMPLETE_DATA
	printf("Lat: %f %c\n", gps.latitude, gps.lat_direction);
	printf("Lon: %f %c\n", gps.longitude, gps.lon_direction);
	printf("Alt: %f\n", gps.altitude);
	printf("Time: %f\n", gps.time);
	printf("Date: %ld\n", gps.date);
	printf("SV: %d\n", gps.satellites_visible);
	printf("SU: %d\n", gps.satellites_in_use);
	printf("FIX: %d\n", gps.fix_mode);
	printf("Quality: %d\n", gps.gps_quality);
	printf("PDOP: %f\n", gps.pdop);
	printf("HDOP: %f\n", gps.hdop);
	printf("VDOP: %f\n", gps.vdop);
	printf("kts: %f\n", gps.ground_speed_knots);
	printf("kph: %f\n", gps.ground_speed_kph);
#endif

	gps_data_ready_flag = 0;
	rx_buffer_index = 0;

	return 1;
}

int gps_is_data_ready(){
	uint32_t current_time = HAL_GetTick();

	if(gps_data_ready_flag) {
		return 1;
	}
	else {
		if(current_time >= (rx_data_timer + 5) && rx_buffer[rx_buffer_index-1] == 0x0A && rx_buffer[rx_buffer_index-2] == 0x0D) {
			if(rx_buffer_index <= LOWEST_BUFFER_SIZE) {
				rx_buffer_index = 0;
				return 0;
			}
			return 1;
		}
	}
	return 0;
}

void gps_print_rx_buffer(){
	printf("\n----rxBuffer_START----\n\n");
	printf("%.*s\n\n", rx_buffer_index-1, rx_buffer);
	printf("----rxBuffer_STOP----\n\n");
}

void parse_rmc(void) {
	char *temp_nmea_sentence = (char *)malloc(substring_length[0] + 1);
	snprintf(temp_nmea_sentence, substring_length[0], "%s", substring_start[0]);
	sscanf(temp_nmea_sentence, "%f,%*c,%f,%c,%f,%c,%f,,%lu,,,%*c", &gps.time, &gps.latitude, &gps.lat_direction, &gps.longitude, &gps.lon_direction, &gps.ground_speed_knots, &gps.date);

#ifdef PRINT_DEBUGGER
	printf("RMC: %s\n", temp_nmea_sentence);
	printf("Time: %f\n", gps.time);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("Speed knots: %f\n", gps.ground_speed_knots);
	printf("Date: %lu\n", gps.date);
	printf("\n");
#endif
	free(temp_nmea_sentence);
}

void parse_vtg(void) {
	char *temp_nmea_sentence = (char *)malloc(substring_length[1] + 1);
	snprintf(temp_nmea_sentence, substring_length[1], "%s", substring_start[1]);
	sscanf(temp_nmea_sentence, ",%*c,,%*c,%f,%*c,%f,%*c,%*c", &gps.ground_speed_knots, &gps.ground_speed_kph);

#ifdef PRINT_DEBUGGER
	printf("VTG: %s\n", temp_nmea_sentence);
	printf("Speed knots: %f\n", gps.ground_speed_knots);
	printf("Speed km/h: %f\n", gps.ground_speed_kph);
	printf("\n");
#endif
	free(temp_nmea_sentence);
}

void parse_gga(void) {
	char *temp_nmea_sentence = (char *)malloc(substring_length[2] + 1);
	snprintf(temp_nmea_sentence, substring_length[2], "%s", substring_start[2]);
	sscanf(temp_nmea_sentence, "%f,%f,%c,%f,%c,%d,%d,%f,%f,%*c,%*f,%*c,,", &gps.time, &gps.latitude, &gps.lat_direction, &gps.longitude, &gps.lon_direction, &gps.gps_quality, &gps.satellites_in_use, &gps.hdop, &gps.altitude);

#ifdef PRINT_DEBUGGER
	printf("GGA: %s\n", temp_nmea_sentence);
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
	free(temp_nmea_sentence);
}

void parse_gsa(void) {
	char *temp_nmea_sentence = (char *)malloc(substring_length[3] + 1);
	snprintf(temp_nmea_sentence, substring_length[3], "%s", substring_start[3]);
	sscanf(temp_nmea_sentence, "%*c,%d", &gps.fix_mode);
	/* Length of GSA may differ depending on amount of connected satellites, while loop searches for 14'th comma - where PDOP, HDOP, VDOP data starts */
	int coma_counter = 0;
	int index = 0;
	while(coma_counter != 14)	if(temp_nmea_sentence[index++] == ',')	coma_counter++;
	sscanf(temp_nmea_sentence + index, "%f,%f,%f", &gps.pdop, &gps.hdop, &gps.vdop);

#ifdef PRINT_DEBUGGER
	printf("GSA: %s\n", temp_nmea_sentence);
	printf("Fix mode: %d\n", gps.fix_mode);	//1 - no fix | 2 - 2D | 3 - 3D |
	printf("PDOP: %f\n", gps.pdop);
	printf("HDOP: %f\n", gps.hdop);
	printf("VDOP: %f\n", gps.vdop);
	printf("\n");
#endif
	free(temp_nmea_sentence);
}

void parse_gsv(void) {
	char *temp_nmea_sentence = (char *)malloc(substring_length[4] + 1);
	snprintf(temp_nmea_sentence, substring_length[4], "%s", substring_start[4]);
	sscanf(temp_nmea_sentence, "%*d,%*d,%d", &gps.satellites_visible);

#ifdef PRINT_DEBUGGER
	printf("GSV: %s\n", temp_nmea_sentence);
	printf("satellites_visible: %d\n", gps.satellites_visible);
	printf("\n");
#endif
	free(temp_nmea_sentence);
}

void parse_gll(void) {
	char *temp_nmea_sentence = (char *)malloc(substring_length[5] + 1);
	snprintf(temp_nmea_sentence, substring_length[5], "%s", substring_start[5]);
	sscanf(temp_nmea_sentence, "%f,%c,%f,%c,%f,%*c,%*c", &gps.latitude, &gps.lat_direction, &gps.longitude, &gps.lon_direction, &gps.time);

#ifdef PRINT_DEBUGGER
	printf("GLL: %s\n", temp_nmea_sentence);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("Time: %f\n", gps.time);
	printf("\n");
#endif
	free(temp_nmea_sentence);
}

char* gps_complete_location_string() {
	static char buffer[23];
	sprintf(buffer, "%f %c, %f %c", gps_latitude(), gps_lat_direction(), gps_longitude(), gps_lon_direction());
	return buffer;
}

float gps_latitude(){
	int minutes = gps.latitude/100;
	float seconds = (gps.latitude-minutes*100)/60;
	return minutes+seconds;
}

char gps_lat_direction(){
	return	gps.lat_direction;
}

float gps_longitude(){
	int minutes = gps.longitude/100;
	float seconds = (gps.longitude-minutes*100)/60;
	return minutes+seconds;
}

char gps_lon_direction(){
	return	gps.lon_direction;
}

int gps_altitude(void){
	return gps.altitude;
}

//Time offset doesn't affect date. Date is always for UTC+0
char* gps_complete_date_string(void){
	static char buffer[20];
	if(gps_day() < 10)	sprintf(buffer, "0%d/", gps_day());
	else	sprintf(buffer, "%d/", gps_day());
	if(gps_month() < 10)	sprintf(buffer + strlen(buffer), "0%d/", gps_month());
	else	sprintf(buffer + strlen(buffer), "%d/", gps_month());
	sprintf(buffer + strlen(buffer), "%d", gps_year_long_format());
	return buffer;
}

int gps_day(void){
	return gps.date/10000;
}

int gps_month(void){
	return (gps.date-gps_day()*10000)/100;
}

int gps_year(void){
	return (gps.date-(gps_day()*10000)-(gps_month()*100));
}

int gps_year_long_format(void){
	return (2000 + gps_year());
}

char* gps_complete_time_string(int offset){
	static char buffer[20];
	if(gps_hour(offset) < 10)	sprintf(buffer, "0%d:", gps_hour(offset));
	else	sprintf(buffer, "%d:", gps_hour(offset));
	if(gps_minute() < 10)	sprintf(buffer + strlen(buffer), "0%d:", gps_minute());
	else	sprintf(buffer + strlen(buffer), "%d:", gps_minute());
	if(gps_second() < 10)	sprintf(buffer + strlen(buffer), "0%d", gps_second());
	else	sprintf(buffer + strlen(buffer), "%d", gps_second());
	return buffer;
}

int gps_hour(int offset){
	int hour = gps.time / 10000;
	hour += offset;
	if (hour < 0) hour += 24;
	else if (hour >= 24) hour -= 24;
	return hour;
}

int gps_minute(void){
	int hour = gps.time / 10000;
	return (gps.time - hour * 10000) / 100;
}

int gps_second(void){
	int hour = gps.time / 10000;
	int minute = (gps.time - hour * 10000) / 100;
	return gps.time - hour * 10000 - minute * 100;
}

int gps_satellites_visible(void){
	return gps.satellites_visible;
}

int gps_satellites_in_use(void){
	return gps.satellites_in_use;
}

char* gps_fix_mode_string(void){
	static char buffer[10];
	switch(gps.fix_mode) {
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

char* gps_quality_string(void){
	static char buffer[16];
	switch(gps.gps_quality) {
	case 0:
		strcpy(buffer, "FIX NOT VALID");
		break;
	case 1:
		strcpy(buffer, "GPS FIX");
		break;
	case 2:
		strcpy(buffer, "3D FIX");
		break;
	case 3:
		strcpy(buffer, "FIX NOT VALID");
		break;
	case 4:
		strcpy(buffer, "GPS FIX");
		break;
	case 5:
		strcpy(buffer, "3D FIX");
		break;
	case 6:
		strcpy(buffer, "3D FIX");
		break;
	default:
		strcpy(buffer, "FIX ERROR");
		break;
	}
	return buffer;
}

float gps_pdop(void){
	return gps.pdop;
}

float gps_hdop(void){
	return gps.hdop;
}

float gps_vdop(void){
	return gps.vdop;
}

float gps_speed_knots(void){
	return gps.ground_speed_knots;
}

float gps_speed_kph(void){
	return gps.ground_speed_kph;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == GPS_UART)
	{
		if(rx_buffer_index < RX_BUFFER_SIZE) {
			rx_data_timer = HAL_GetTick();
			rx_buffer[rx_buffer_index++] = received_byte;
		} else {
			gps_data_ready_flag = 1;
		}
		HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
	}
}

