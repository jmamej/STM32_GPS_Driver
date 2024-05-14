#include "../GPS/GPS.h"

#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#define PRINT_DEBUGGER

extern UART_HandleTypeDef huart3;

uint8_t receivedByte;
uint8_t rxBuffer[550];
volatile uint16_t rxBufferIndex = 0;
uint8_t last_NMEA_sentence_flag, GPS_data_ready_flag;


char RMC[64];
char VTG[32];
char GGA[72];
char GSA[64];
char GSV[64];
char GLL[64];

const char *NMEA_ID[] = {"RMC", "VTG", "GGA", "GSA", "GSV", "GLL"};
char *start[6] = {NULL};
char *stop[6] = {NULL};

NMEA_Data GPS;

void parseRMC(void);
void parseVTG(void);
void parseGGA(void);
void parseGSA(void);
void parseGSV(void);
void parseGLL(void);

void GPS_init(void){
    // No need for extern here, assuming huart3 is a global variable
    #define GPS_UART &huart3
    extern uint8_t receivedByte; // Declare the global variable from main.c
    HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&receivedByte, 1);
}

void GPS_GetData(void){

	char tempBuffer[550];
	snprintf(tempBuffer, sizeof(tempBuffer), "%s", rxBuffer);

	for(int i = 0; i < 6; i++)
	{
		start[i] = strstr(tempBuffer, NMEA_ID[i]) + sizeof(NMEA_ID[i]);
		stop[i] = strstr(start[i], "*") + 1;
	}

	parseRMC();	//populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | ground_speed_knots | date |
	parseVTG();	//populates following variables with data: | ground_speed_knots | ground_speed_kph |
	parseGGA();	//populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | GPS_quality | satellites_in_use | HDOP | altitude |
	parseGSA();	//populates following variables with data: | fix_mode | PDOP | HDOP | VDOP |
	parseGSV();	//populates following variables with data: | satellites_visible |
	parseGLL();	//populates following variables with data: | latitude | lat_direction | longitude | lon_direction | time |

	GPS_data_ready_flag = 0;
}

int GPS_IsDataReady(){
	return GPS_data_ready_flag;
}


void GPS_PrintRXBuffer(){
	printf("\n----rxBuffer_START----\n\n");
	printf("%s\n", rxBuffer);
	printf("----rxBuffer_STOP----\n");
}

void parseRMC(){
	snprintf(RMC, stop[0] - start[0], "%s", start[0]);
	sscanf(RMC, "%f,%*c,%f,%c,%f,%c,%f,,%lu,,,%*c", &GPS.time, &GPS.latitude, &GPS.lat_direction, &GPS.longitude, &GPS.lon_direction, &GPS.ground_speed_knots, &GPS.date);

#ifdef PRINT_DEBUGGER
	printf("RMC: %s\n", RMC);
	printf("Time: %f\n", GPS.time);
	printf("Latitude: %f\n", GPS.latitude);
	printf("Lat_dir: %c\n", GPS.lat_direction);
	printf("longitude: %f\n", GPS.longitude);
	printf("Long_dir: %c\n", GPS.lon_direction);
	printf("Speed knots: %f\n", GPS.ground_speed_knots);
	printf("Date: %lu\n", GPS.date);
	printf("\n");
#endif

}


void parseVTG(void){
	snprintf(VTG, stop[1] - start[1], "%s", start[1]);
	sscanf(VTG, ",%*c,,%*c,%f,%*c,%f,%*c,%*c", &GPS.ground_speed_knots, &GPS.ground_speed_kph);

#ifdef PRINT_DEBUGGER
	printf("VTG: %s\n", VTG);
	printf("Speed knots: %f\n", GPS.ground_speed_knots);
	printf("Speed km/h: %f\n", GPS.ground_speed_kph);
	printf("\n");
#endif

}


void parseGGA(void){
	snprintf(GGA, stop[2] - start[2], "%s", start[2]);
	sscanf(GGA, "%f,%f,%c,%f,%c,%d,%d,%f,%f,%*c,%*f,%*c,,", &GPS.time, &GPS.latitude, &GPS.lat_direction, &GPS.longitude, &GPS.lon_direction, &GPS.GPS_quality, &GPS.satellites_in_use, &GPS.HDOP, &GPS.altitude);

#ifdef PRINT_DEBUGGER
	printf("GGA: %s\n", GGA);
	printf("Time: %f\n", GPS.time);
	printf("Latitude: %f\n", GPS.latitude);
	printf("Lat_dir: %c\n", GPS.lat_direction);
	printf("longitude: %f\n", GPS.longitude);
	printf("Long_dir: %c\n", GPS.lon_direction);
	printf("GPS quality: %d\n", GPS.GPS_quality);
	printf("satelites_in_use: %d\n", GPS.satellites_in_use);
	printf("Precision (HDOP): %f\n", GPS.HDOP);
	printf("Altitude: %f\n", GPS.altitude);
	printf("\n");
#endif

}

void parseGSA(void){
	snprintf(GSA, stop[3] - start[3], "%s", start[3]);
	sscanf(GSA, "%*c,%d", &GPS.fix_mode);
	//Length of GSA may differ depending on amount of connected satellites, while loop searches for 14'th comma - where PDOP, HDOP, VDOP data starts
	int coma_counter = 0;
	int index = 0;
	while(coma_counter != 14)	if(GSA[index++] == ',')	coma_counter++;

	//printf("GSA: %s\n", GSA + index);	// Rest of GSA sentence with PDOP, HDOP, VDOP data

	sscanf(GSA + index, "%f,%f,%f", &GPS.PDOP, &GPS.HDOP, &GPS.VDOP);

#ifdef PRINT_DEBUGGER
	printf("GSA: %s\n", GSA);
	printf("Fix mode: %d\n", GPS.fix_mode);	//1 - no fix | 2 - 2D | 3 - 3D |
	printf("PDOP: %f\n", GPS.PDOP);
	printf("HDOP: %f\n", GPS.HDOP);
	printf("VDOP: %f\n", GPS.VDOP);
	printf("\n");
#endif

}

void parseGSV(void){
	snprintf(GSV, stop[4] - start[4], "%s", start[4]);
	sscanf(GSV, "%*d,%*d,%d", &GPS.satellites_visible);

#ifdef PRINT_DEBUGGER
	printf("GSV: %s\n", GSV);
	printf("satellites_visible: %d\n", GPS.satellites_visible);
	printf("\n");
#endif

}

void parseGLL(void){
	snprintf(GLL, stop[5] - start[5], "%s", start[5]);
	sscanf(GLL, "%f,%c,%f,%c,%f,%*c,%*c", &GPS.latitude, &GPS.lat_direction, &GPS.longitude, &GPS.lon_direction, &GPS.time);

#ifdef PRINT_DEBUGGER
	printf("GLL: %s\n", GLL);
	printf("Latitude: %f\n", GPS.latitude);
	printf("Lat_dir: %c\n", GPS.lat_direction);
	printf("longitude: %f\n", GPS.longitude);
	printf("Long_dir: %c\n", GPS.lon_direction);
	printf("Time: %f\n", GPS.time);
	printf("\n");
#endif

}


char* GPS_GetCompleteLocation() {
	static char buffer[23];
	sprintf(buffer, "%f %c, %f %c", GPS_GetLatitude(), GPS_GetLatDirection(), GPS_GetLongitude(), GPS_GetLonDirection());
	return buffer;
}

float GPS_GetLatitude(){
    int minutes = GPS.latitude/100;
    float seconds = (GPS.latitude-minutes*100)/60;
    return minutes+seconds;
}

char GPS_GetLatDirection(){
	return	GPS.lat_direction;
}

float GPS_GetLongitude(){
    int minutes = GPS.longitude/100;
    float seconds = (GPS.longitude-minutes*100)/60;
    return minutes+seconds;
}

char GPS_GetLonDirection(){
	return	GPS.lon_direction;
}

int GPS_GetAltitude(void){
	return GPS.altitude;
}

//Time offset doesn't affect date. Date is always for UTC+0
char* GPS_GetCompleteDate(void){
	static char buffer[20];
	if(GPS_GetDay() < 10)	sprintf(buffer, "0%d/", GPS_GetDay());
	else	sprintf(buffer, "%d/", GPS_GetDay());
	if(GPS_GetMonth() < 10)	sprintf(buffer + strlen(buffer), "0%d/", GPS_GetMonth());
	else	sprintf(buffer + strlen(buffer), "%d/", GPS_GetMonth());
	sprintf(buffer + strlen(buffer), "%d", GPS_GetYearLong());
	return buffer;
}

int GPS_GetDay(void){
    return GPS.date/10000;
}

int GPS_GetMonth(void){
    return (GPS.date-GPS_GetDay()*10000)/100;
}

int GPS_GetYear(void){
    return (GPS.date-(GPS_GetDay()*10000)-(GPS_GetMonth()*100));
}

int GPS_GetYearLong(void){
    return (2000 + GPS_GetYear());
}

char* GPS_GetCompleteTime(int offset){
	static char buffer[20];
	if(GPS_GetHour(offset) < 10)	sprintf(buffer, "0%d:", GPS_GetHour(offset));
	else	sprintf(buffer, "%d:", GPS_GetHour(offset));
	if(GPS_GetMinute() < 10)	sprintf(buffer + strlen(buffer), "0%d:", GPS_GetMinute());
	else	sprintf(buffer + strlen(buffer), "%d:", GPS_GetMinute());
	if(GPS_GetSecond() < 10)	sprintf(buffer + strlen(buffer), "0%d", GPS_GetSecond());
	else	sprintf(buffer + strlen(buffer), "%d", GPS_GetSecond());

	return buffer;
}

int GPS_GetHour(int offset){
    int hour = GPS.time / 10000;
    hour += offset;
    if (hour < 0) hour += 24;
    else if (hour >= 24) hour -= 24;
    return hour;
}

int GPS_GetMinute(void){
    int hour = GPS.time / 10000;
    return (GPS.time - hour * 10000) / 100;
}

int GPS_GetSecond(void){
    int hour = GPS.time / 10000;
    int minute = (GPS.time - hour * 10000) / 100;
    return GPS.time - hour * 10000 - minute * 100;
}

int GPS_GetSatellitesVisible(void){
	return GPS.satellites_visible;
}

int GPS_GetSatellitesInUse(void){
	return GPS.satellites_in_use;
}

char* GPS_GetFixMode(void){
	static char buffer[10];
    switch(GPS.fix_mode) {
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

char* GPS_GetGPSQuality(void){
	static char buffer[16];
    switch(GPS.GPS_quality) {
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

float GPS_GetPDOP(void){
	return GPS.PDOP;
}

float GPS_GetHDOP(void){
	return GPS.HDOP;
}

float GPS_GetVDOP(void){
	return GPS.VDOP;
}

float GPS_GetSpeedKnots(void){
	return GPS.ground_speed_knots;
}

float GPS_GetSpeedKph(void){
	return GPS.ground_speed_kph;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == GPS_UART)
	{
		//printf("UART interrupt\n");
		receivedByte = huart->Instance->RDR;
		rxBuffer[rxBufferIndex++] = receivedByte;

		if(receivedByte == 'L')	last_NMEA_sentence_flag = 1;
		else if (receivedByte == '\n' && last_NMEA_sentence_flag) {
			rxBuffer[rxBufferIndex++] = '\0';
			printf("\nBuffer length: %d\n\n", rxBufferIndex);
			rxBufferIndex = 0;
			last_NMEA_sentence_flag = 0;
			GPS_data_ready_flag = 1;
		}
		HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&receivedByte, 1);
	}
}

