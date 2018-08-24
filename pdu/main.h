#ifndef MAIN_H__
#define MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <wchar.h>
#include <time.h>
#include <locale.h>

#define PHONE_NUM	32
#define MXA_SIZE    512
#define LIMIT_SIZE  66*2
#define LIMIT_7BIT  152
#define GSM_7BIT    0x00
#define GSM_8BIT    0x04
#define GSM_UNICODE 0x08
#define perror(txt); {printf("%s(%d):%s", __FUNCTION__, __LINE__, txt);}
typedef unsigned char uchar;

typedef struct
{
	uchar year;
	uchar mouth;
	uchar day;
	uchar hour;
	uchar minu;
	uchar sec;
	uchar timz;
}timp;

typedef struct gsmtp_t
{
	int merge_num;
	char pdu_mti;
	uchar merge_flag[2];
	int merge_end;
	uchar tppid;
	uchar tpdcs;
	int tplongth;
	uchar tp_udhi;
	uchar *tpdata[PHONE_NUM];
	uchar *result_data;
	timp timestamp;
	uchar smsc[PHONE_NUM];
	uchar rete[PHONE_NUM];
	uchar udli_data[PHONE_NUM/2];
}gsmtp;

/********7bit***********/
static int gsmcode_7bit(uchar * src,uchar * dst,int length);
static int gsmdecode_7bit(uchar * src, uchar  *dst, int length);

/********8bit***********/
static int gsmcode_8bit(uchar* src,uchar *dst, int length);
static int gsmdecode_8bit(uchar* src,uchar *dst, int length);

/********unic***********/
static int gsmcode_unic(uchar* src,uchar *dst, int length);
static int gsmdecode_unic(uchar* src,uchar *dst, int length);

/********unic***********/
static int gsmstring_byte(uchar *src, uchar *dst, int length);
static int gsmbyte_string(uchar *src, uchar *dst, int length);

/**********unica*********/
static int gsmby_normal(uchar *src,uchar *dst,int length);
static int gsmto_normal(uchar* src,uchar *dst, int length)  ;

/********pud***********/
int gsmcodepdu(uchar *src, gsmtp *gsm_data);
int gsmdecodepdu(uchar *src,gsmtp *gsm_data);

static void stamp_check(uchar *src, const struct tm *p);
static int conversion(uchar nsrc);
static void gsm_error(uchar *src);
#endif
