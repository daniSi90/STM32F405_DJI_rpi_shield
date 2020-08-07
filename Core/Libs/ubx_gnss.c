/*
 * ubx_gnss.c
 *
 *  Created on: Aug 7, 2020
 *      Author: primoz
 */

#include "ubx_gnss.h"

uint8_t rxString[MAX_GNSS];
int rxindex = 0;
uint8_t *rxBufferGNSSp;  // DODANO
tGNSSrx GNSSrx;



void ubx_handleGNSS(CGNSS* handle){
	uint8_t msgbuf[MAX_GNSS];
	int32_t msgcnt;
	msgcnt = readUBXpkt (handle, msgbuf);

	if(msgcnt>0)
	{
		EventsCommGNSS(handle, msgbuf,msgcnt);
	}
	else if(msgcnt==-1)
	{
		initGNSSrx (handle);
	}
}

void EventsCommGNSS(CGNSS* handle, uint8_t *msgbuf, int32_t cnt)
{
	parseUBX(handle, msgbuf,cnt);
}

void initGNSSrx(CGNSS* handle)
{
  GNSSrx.state = SM_UBX_BEFORE;
  GNSSrx.ctr = 0;
}

int readUBXpkt(CGNSS* handle, uint8_t *retbuf)
{
	int i = 0;

	if(GNSSrx.ctr<MAX_GNSS)
	{
	if(addUBXpktByte(handle->rx_byte,&(GNSSrx))>0)
	{
	  GNSSrx.state=SM_UBX_BEFORE;
	  for(i=0;i<GNSSrx.ctr;i++) retbuf[i]=GNSSrx.buf[i];
	  GNSSrx.ctr=0;
	  if(checkUBX(retbuf,i)==0)
	  {
		return (i-2);
	  }
	  else
	  {
		return 0;
	  }
	}
	}
	if(GNSSrx.ctr>=MAX_GNSS)
	{
	GNSSrx.ctr=0;
	GNSSrx.state=SM_UBX_BEFORE;
	}
	return 0;
}


uint8_t parseUBX(CGNSS* handle, uint8_t *buf, int cnt)
{
	uint8_t ok = 0;

	if(buf[0]==UBX_NAV)
	{
		if(buf[1]==UBX_NAV_PVT && cnt>=92)
		{
		  handle->iTOW = bytesToLong(&(buf[4]));
		  handle->UTCyear = bytesToShort(&(buf[8]));
		  handle->UTCmonth = (int)buf[10];
		  handle->UTCday = (int)buf[11];
		  handle->UTChour = (int)buf[12];
		  handle->UTCminute = (int)buf[13];
		  handle->UTCsecond = (int)buf[14];
		  handle->fixType = (int)buf[24];
		  handle->hAcc = bytesToLong(&(buf[44]));
		  handle->vAcc = bytesToLong(&(buf[48]));
		  handle->pos.lon = bytesToLong(&(buf[28]))*1.0e-7;
		  handle->pos.lat = bytesToLong(&(buf[32]))*1.0e-7;
		  handle->pos.alt = bytesToLong(&(buf[36]))*1.0e-7;
		}
		else if(buf[1]==UBX_NAV_RELPOSNED && cnt>=40)
		{
			handle->relPos.N = bytesToLong(&(buf[12]))+0.01f*(float)buf[24];
			handle->relPos.E = bytesToLong(&(buf[16]))+0.01f*(float)buf[25];
			handle->relPos.D = bytesToLong(&(buf[20]))+0.01f*(float)buf[26];
		}
	}
	else if(buf[0]==UBX_MON)
	{
		if(buf[1]==UBX_MON_MSGPP && cnt>=120){
			handle->msgs = bytesToShort(&(buf[46]));
		}
	}
	return ok;
}

int addUBXpktByte(uint8_t ch, tGNSSrx *pr)
{
	switch(pr->state)
		{
		case SM_UBX_BEFORE:
			if(ch==UBX_SYN_CHAR1) pr->state=SM_UBX_SYN2;     //SYNCHAR1
			break;
		case SM_UBX_SYN2:
			if(ch==UBX_SYN_CHAR2) pr->state=SM_UBX_CLASS;     //SYNCHAR2
			else pr->state=SM_UBX_BEFORE;
			break;
		case SM_UBX_CLASS:
			pr->buf[pr->ctr++]=ch;          //CLASS
			pr->state=SM_UBX_ID;
			break;
		case SM_UBX_ID:
			pr->buf[pr->ctr++]=ch;          //ID
			pr->state=SM_UBX_PAYLEN1;
			break;
		case SM_UBX_PAYLEN1:
			pr->buf[pr->ctr++]=ch;          //PAYLOAD LENGTH1
			pr->state=SM_UBX_PAYLEN2;
			break;
		case SM_UBX_PAYLEN2:
			pr->buf[pr->ctr++]=ch;          //PAYLOAD LENGTH2
			pr->state=SM_UBX_PAYLOAD;
			break;
		case SM_UBX_PAYLOAD:
			pr->buf[pr->ctr++]=ch;          //PAYLOAD
			if(pr->ctr >= (bytesToShort((uint8_t *)&(pr->buf[2])) + 4)) pr->state=SM_UBX_CHK1;
			else if(pr->ctr >= (UART_BUF_SIZE-10)) pr->state=SM_UBX_ERR;
			break;
		case SM_UBX_CHK1:
			pr->buf[pr->ctr++]=ch;
			pr->state=SM_UBX_CHK2;			//CHECKSUM1
			break;
		case SM_UBX_CHK2:
			pr->buf[pr->ctr++]=ch;
			pr->state=SM_UBX_END;			//CHECKSUM1
			break;
		case SM_UBX_ERR:
			pr->state=SM_UBX_BEFORE;
			break;
		default:
			pr->state=SM_UBX_ERR;
			break;
		}
		if(pr->state==SM_UBX_ERR || pr->state==SM_UBX_BEFORE)
		{
			return(-1);
		}
		else if(pr->state==SM_UBX_END)
		{
			pr->state=SM_UBX_BEFORE;
			return(pr->ctr);
		}
		else return(0);
}

int checkUBX(uint8_t *buf, int cnt)
{
	uint8_t cha=0, chb=0;

	crcUBX(buf,cnt-2,&cha,&chb);
	if((cha == buf[cnt-2]) && (chb == buf[cnt-1])) return(0);
	return(-1);
}

void crcUBX(uint8_t *buf, int cnt, uint8_t *pcha, uint8_t *pchb)
{
	int i=0;
	*pcha=0;
	*pchb=0;
	for(i=0 ; i<cnt ; i++)
	{
		(*pcha) = (uint8_t)((*pcha) + buf[i]);
		(*pchb) = (uint8_t)((*pchb) + (*pcha));
	}
}

int32_t bytesToLong(uint8_t *b)
{
	int8_t i;
	mlong x;
	for(i=0 ; i<4 ; i++)
	{
		x.b[i] = b[i];
	}
	return(x.i);
}

int16_t bytesToShort(uint8_t *b)
{
	mshort x;
	x.b[1] = b[1];
	x.b[0] = b[0];
	return(x.i);
}
