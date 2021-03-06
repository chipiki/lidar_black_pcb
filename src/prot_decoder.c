/*
 * prot_decoder.c
 *
 *  Created on: 7 ???. 2021 ?.
 *      Author: DA.Tsekh
 */


#include "prot_decoder.h"

#define M_PI		3.14159265358979323846
// #define M_PIl          3.141592653589793238462643383279502884L

#define HEADER_B1			(uint8_t)0xAA
#define HEADER_B2			(uint8_t)0x55

#define DATA_PACKET_TYPE	(uint8_t)0x28
#define MAX_DATA_RECORDS	(uint8_t)0x28

#define STEP	( -M_PI * 2.0 )

typedef enum {
	PROT_INIT = 0,
	PROT_BYTE1,
	PROT_BYTE2,
	PROT_PACKET_TYPE,
	PROT_DATA_COUNT,
	PROT_ANGLE_START_B1,
	PROT_ANGLE_START_B2,
	PROT_ANGLE_STOP_B1,
	PROT_ANGLE_STOP_B2,
	PROT_UNKNOWN_DATA_B1,
	PROT_UNKNOWN_DATA_B2,
	PROT_READ_DATA
} pr_state_t;

typedef struct __attribute__((packed)) {
	uint8_t	quality;
	uint16_t distance;
} data_rec_t ;

typedef struct __attribute__((packed)) {
	uint8_t	b1;								// 0xAA
	uint8_t	b2;								// 0x55
	uint8_t	type;							// 0x28 - data, or spinning speed
	uint8_t	data_cnt;						// count of data fields
	uint16_t angle_start;					// starting angle
	uint16_t angle_stop;					// starting angle
	uint8_t unknown_data_b1;				// unknown_data byte 1
	uint8_t unknown_data_b2;				// unknown_data byte 2
	data_rec_t data[MAX_DATA_RECORDS];		// Data fields
} packet_t;

typedef struct __attribute__((packed)) {
	float	angle;
	float	distance;
} angle_dist_t ;


// State of protocol
pr_state_t prstate = PROT_INIT;

// Packet structure from lidar
packet_t p;

// Index of all bytes in packet
uint8_t data_bytes_indx = 0;

// Index of data measurements
uint8_t data_mess_indx = 0;

// Index of byte inside of measurement
uint8_t mess_bytes_indx = 0;

// Angles and distances
angle_dist_t ad[MAX_DATA_RECORDS] = {0};

void init_prot(){
	p.b1 = 0;
	p.b2 = 0;
	p.type = 0;
	p.data_cnt = 0;
	p.angle_start = 0;
	p.angle_stop = 0;
	p.unknown_data_b1 = 0;
	p.unknown_data_b2 = 0;

	for ( int i = 0; i < MAX_DATA_RECORDS; i++ ){
		p.data[i].quality = 0;
		p.data[i].distance = 0;
	}

}


void process_data( const uint8_t byte ){

	switch (data_bytes_indx) {
		case 0:
			p.data[data_mess_indx].quality = byte;
			data_bytes_indx++;
			break;

		case 1:
			p.data[data_mess_indx].distance = byte;
			data_bytes_indx++;
			break;

		case 2:
			p.data[data_mess_indx].distance = (p.data[data_mess_indx].distance << 8) + byte;
			data_bytes_indx = 0;
			data_mess_indx++;
			break;
		default:
			break;
	}

}

void print_packet( void ){
	printf( "\n\nPacket header:\n" );
	printf( "%02X %02X %02X %02X %d %d %02X %02X\n", p.b1, p.b2, p.type, p.data_cnt, p.angle_start, p.angle_stop, p.unknown_data_b1, p.unknown_data_b2 );

	printf( "\nPacket data:\n" );

	for( int i = 0; i < p.data_cnt; i++ ){
		printf( "[%d]\t%d\t%d\n", i, p.data[i].quality, p.data[i].distance );
	}
}

void calculete_angles(){

	uint16_t diff = 0;
	float angle_per_sample = 0.0;
	float angle = 0.0;
	float anglef = 0.0;

	diff = p.angle_stop - p.angle_start;
	if( p.angle_stop < p.angle_start ){
		diff = (uint16_t)0xB400 - p.angle_start + p.angle_stop;
	}

	if (diff > 1){
		angle_per_sample = (float)diff / (float)( p.data_cnt - 1);
	}

	for (int i = 0; i < p.data_cnt ; i++){
		angle = (p.angle_start + angle_per_sample * i);
		anglef = STEP * (angle / 0xB400) ;

		ad[i].angle = anglef;
		ad[i].distance = p.data[i].distance / 1000.0;

		printf("%.6f;%.6f\n", ad[i].angle, ad[i].distance );
	}

}



void decoder( const uint8_t byte ){

	switch ( prstate ) {

		case PROT_INIT:
			init_prot();
			prstate = PROT_BYTE1;
			break;

		case PROT_BYTE1:
			if(byte==HEADER_B1){
				p.b1 = byte;
				data_bytes_indx = 0;
				data_mess_indx = 0;
				mess_bytes_indx = 0;
				prstate = PROT_BYTE2;
			}
			break;

		case PROT_BYTE2:
			if(byte==HEADER_B2){
				p.b2 = byte;
				prstate = PROT_PACKET_TYPE;
			} else {
				prstate = PROT_BYTE1;
			}
			break;

		case PROT_PACKET_TYPE:
			p.type = byte;
			prstate = PROT_DATA_COUNT;
			break;

		case PROT_DATA_COUNT:
			p.data_cnt = byte;
			prstate = PROT_ANGLE_START_B1;
			break;

		case PROT_ANGLE_START_B1:
			p.angle_start = byte;
			prstate = PROT_ANGLE_START_B2;
			break;

		case PROT_ANGLE_START_B2:
			p.angle_start = (uint16_t)(( ((uint16_t)byte) << 8) + p.angle_start);
			prstate = PROT_ANGLE_STOP_B1;
			break;

		case PROT_ANGLE_STOP_B1:
			p.angle_stop = byte;
			prstate = PROT_ANGLE_STOP_B2;
			break;

		case PROT_ANGLE_STOP_B2:
			p.angle_stop = (uint16_t)(( ((uint16_t)byte) << 8) + p.angle_stop);
			prstate = PROT_UNKNOWN_DATA_B1;
			break;

		case PROT_UNKNOWN_DATA_B1:
			p.unknown_data_b1 = byte;
			prstate = PROT_UNKNOWN_DATA_B2;
			break;

		case PROT_UNKNOWN_DATA_B2:
			p.unknown_data_b2 = byte;
			prstate = PROT_READ_DATA;
			break;

		case PROT_READ_DATA:

			process_data( byte );

			if ( (data_mess_indx + 1) == p.data_cnt ){
				//print_packet();
				calculete_angles();
				prstate = PROT_BYTE1;
			}
			break;

		default:
			break;
	}

}
