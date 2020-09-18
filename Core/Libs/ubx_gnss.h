/*
 * ubx_gnss.h
 *
 *  Created on: Aug 7, 2020
 *      Author: primoz
 */

#ifndef LIBS_UBX_GNSS_H_
#define LIBS_UBX_GNSS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

#define MAX_GNSS          256
#define UART_BUF_SIZE     256

/*UBX sync chars*/
#define UBX_SYN_CHAR1    0xB5
#define UBX_SYN_CHAR2    0x62

/*UBX SM states*/
#define  SM_UBX_BEFORE      0
#define  SM_UBX_SYN2    	1
#define  SM_UBX_CLASS     	2
#define  SM_UBX_ID    		3
#define  SM_UBX_PAYLEN1  	4
#define  SM_UBX_PAYLEN2   	5
#define  SM_UBX_PAYLOAD     6
#define  SM_UBX_CHK1      	7
#define  SM_UBX_CHK2      	8
#define  SM_UBX_ERR         9
#define  SM_UBX_END        10

 typedef struct
 {
   int state;
   int ctr;
   uint8_t buf[MAX_GNSS];
 } tGNSSrx;

 typedef union
 {
   uint8_t b[4];
   int32_t i;
 } mlong;

 typedef union
 {
   int8_t b[2];
   int16_t i;
 } mshort;

 enum{
	// UBX CLASS ID
	UBX_NAV     	    = 0x01,   //Navigation Results Messages
	UBX_RXM     	 	= 0x02,   //Monitoring Messages
	UBX_MON     	 	= 0x0A,   //Monitoring Messages
	// UBX MESSAGE ID
	UBX_NAV_PVT         = 0x07,   //NAV-PVT: Navigation Position Velocity Time Solution
	UBX_NAV_RELPOSNED   = 0x3C,   //NAV-RELPOSNED: Relative Positioning Information in NED frame
	UBX_MON_MSGPP       = 0x06,   //MON-MSGPP: Message Parse and Process Status
	UBX_RXM_RTCM		= 0x32
 };

// Fix Types
typedef enum {
	RTK_NO_FIX  = 0x00U,
	RTK_DRO		= 0x01U,
	RTK_2D		= 0x02U,
	RTK_3D		= 0x03U,
	RTK_GNSS	= 0x04U,
	RTK_TIM		= 0x05U
}GNSS_FixTypeDef;

// Fix Types
typedef enum {
	RTCM_DNK  = 0x00U, // Do Not Know
	RTCM_NU	  = 0x01U, // Not Used
	RTCM_SUC  = 0x02U  // RTCM message used successfully by the receiver
}GNSS_RTCMTypeDef;

 typedef struct
 {
 	float lat;
 	float lon;
 	float alt;
 } Pos;

 typedef struct
 {
 	float N;
 	float E;
 	float D;
 	float velocity;
 } Vel;

 typedef struct{
 	float N;
 	float E;
 	float D;
 } PosRel;

 // GNSS Structure
 typedef struct CGNSS{
	Pos pos;
	Vel vel;
	PosRel relPos;
	GNSS_RTCMTypeDef rtcmStat;
	GNSS_FixTypeDef fixType;
	float hAcc;
	float vAcc;
	float iTOW;
	uint16_t UTCyear;
	uint8_t UTCmonth;
	uint8_t UTCday;
	uint8_t UTChour;
	uint8_t UTCminute;
	uint8_t UTCsecond;
	uint8_t parseUBX;
	int msgs;
	uint8_t rx_byte;
 }CGNSS;



void ubx_handleGNSS(CGNSS* handle);
void EventsCommGNSS(CGNSS* handle, uint8_t *msgbuf, int32_t cnt);
void initGNSSrx(CGNSS* handle);
int readUBXpkt(CGNSS* handle, uint8_t *retbuf);
uint8_t parseUBX(CGNSS* handle, uint8_t *buf, int cnt);
int addUBXpktByte(uint8_t ch, tGNSSrx *pr);
int checkUBX(uint8_t *buf, int cnt);
void crcUBX(uint8_t *buf, int cnt, uint8_t *pcha, uint8_t *pchb);

int32_t bytesToLong(uint8_t *b);
int16_t bytesToShort(uint8_t *b);

#ifdef __cplusplus
}
#endif


#endif /* LIBS_UBX_GNSS_H_ */
