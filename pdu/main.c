#include "main.h"
/**********************************
函数：gsmcode7bit()
用7bit编码方式编码pdu消息
************************************/
static int gsmcode_7bit(uchar* src,uchar  *dst,int length)
{
    int num;
    int i;
    uchar left_num;
    for(i = 0;i <= length;i++)
    {
       num  = i % 7;
       if(num == 0)
            left_num = src[i];
       else{
        dst[i] = src[i] <<(8-num) | left_num;
        left_num = src[i] >> num;
       }
    }
    if(i % 7 != 0)
        dst[i++] = left_num;
    dst[i] = '\0';
    return i;
}
/**********************************
函数：gsmdecode7bit()
用7bit编码方式编解码pdu消息

************************************/
static int gsmdecode_7bit( uchar *src,uchar *dst, int length)
{
    int src_num;
    int dst_num;
    int byte_num;
    uchar left_num;
    src_num = 0;
    dst_num = 0;

    byte_num = 0;
    left_num = 0;
    for(src_num = 0; src_num < length;)
    {
        dst[dst_num] = ((*src << byte_num) | left_num) & 0x7f;
        left_num = *src >> (7-byte_num);
        dst_num++;
        byte_num++;
        if(byte_num == 7)
        {
            dst[dst_num] = left_num;
            dst_num++;
            byte_num = 0;
            left_num = 0;
        }
        src++;
        src_num++;
    }
    dst[dst_num] = 0;
    return dst_num;
}
/**********************************
函数：gsmcode_8bit()
以8bit的编码方式编码pdu消息
************************************/
static int gsmcode_8bit(uchar *src,uchar *dst, int length)
{
	memcpy(dst,src,length);
	return length;
}

/**********************************
函数：gsmdecode_8bit()
以8bit的编码方式解码pdu消息
************************************/
static int gsmdecode_8bit(uchar* src,uchar *dst, int length)
{
	memcpy(dst,src,length);
//	printf("%s\n",dst);
	*dst = '\0';
	return length;
}

/**********************************
函数：gsmcode_unic()
以unic码方式编码pdu
************************************/
static int gsmcode_unic(uchar* src,uchar *dst, int length)
{
    int length_dst;
    int i,j;
    wchar_t pwc[MXA_SIZE] = {'\0'};
    setlocale(LC_ALL, "chs");
    length_dst = mbstowcs (pwc,(const char *)src,length);
//    wprintf(pwc);
    for(i = 0,j = 0;i < length_dst;i++)
    {
        dst[j++] = pwc[i] >> 8;
        dst[j++] = pwc[i] & 0xff;
    }
//    printf("%s\n",dst);
    return length_dst;
}

/**********************************
函数：gsmdecode_unic()
以unic方式解码pdu消息
************************************/
static int gsmdecode_unic(uchar* src,uchar *dst, int length)
{
    size_t i,j;
    size_t length_dst;
    wchar_t pwc[MXA_SIZE];
    setlocale(LC_ALL, "chs");
//     printf("%s\n",src);
    for(i = 0,j = 0;i < length;i++)
    {
        pwc[i] = src[j++] << 8;
        pwc[i] = src[j++] | pwc[i];
    }
    length_dst = wcstombs((char *)dst,(const wchar_t *)pwc,i);
    dst[length_dst] = '\0';
//    printf("%s\n",dst);
 //  wprintf(pwc);
    return 0;
}

/*************************
函数：gsmstring_byte()
将字节序数据转换为字符序数据
***************************/
static int gsmbyte_string(uchar *src, uchar *dst, int length)
{
    int i;
    char table[] = "0123456789ABCDEF";
    for(i = 0;i < length * 2;i += 2)
    {
        *dst++ = table[*src >> 4];
    //  *dst++ = table[(*src << 4) >> 4]; 为什么这样有问题呢？？？
        *dst++ = table[*src & 0x0f]  ;
        src++;
    }
    *dst = '\0';
    return i;
}

/********************************
函数：gsmstring_byte()

将字符串型的数据转换为，可以十六
进制的字节型数据。
*********************************/
static int gsmstring_byte(uchar *src, uchar *dst, int length)
{
	int i,j;

    for(i = 0,j = 0; i < length; i++,j++)
    {
        if(src[i] >= '0' && src[i] <= '9')
            dst[j] = (src[i] - '0') << 4;
        else
            dst[j] = (src[i] - 'A' + 10) << 4;
        i++;
        if(src[i] >= '0' && src[i] <= '9')
            dst[j]  = dst[j] + (src[i] - '0');
        else
            dst[j]  = dst[j] + (src[i] - 'A' + 10);
    }
    return j;
}

/*********************************
函数:gsmbynormal()
将正常顺序的字符串转换为颠倒的
字符串。
**********************************/
static int gsmby_normal(uchar *src,uchar *dst,int length)
{
    int i;
	int length_dst;
	char tmp;
	length_dst = length;

	for(i = 0; i< length;i += 2)
	{
	   tmp = *src++;
	   *dst++ = *src++;
	   *dst++ = tmp;
	}
	if(length % 2 == 1)
	{
	   *(dst-2) = 'F';
	   length_dst++;
	}
	*dst = '\0';

return length_dst;
}



/********************************************
函数:gsmtonormal()
将颠倒的字符转换为正常的字符串
********************************************/
static int gsmto_normal(uchar *src,uchar *dst, int length)
{
    int length_dst;
    char tmp;
    int i;
    length_dst = length;
    for(i = 0; i < length;i += 2)
    {
        tmp = *src++;
        *dst++ = *src++;
        *dst++ = tmp;
    }
    if(*(dst-1) == 'F')
    {
        dst--;
        length_dst--;
    }

    *dst = '\0';

    return length_dst;
}
/******************************************
函数：gsmdecodepdu()
将pdu消息根据不同的编码格式进行编码
******************************************/

int gsmcodepdu(uchar *dst, gsmtp *src)
{
    int length = 0;
    int length_dst = 0;
    uchar buf[MXA_SIZE] = {'\0'};
    uchar arr[MXA_SIZE] = {'\0'};
    length = strlen((const char*)src->smsc);
    if((length & 1) == 0)
        buf[0] = (char )length / 2 + 1;
    else
        buf[0] =  (char )(length+1) / 2 + 1;
    buf[1] = (char)0x91;
    length_dst = gsmbyte_string(buf,dst,2);
    length = gsmby_normal(src->smsc, buf,length);
    strcat((char *)dst,(const char *)buf);
    length_dst += length;
    buf[0] = 0x11; //发送模式
    buf[1] = 0;
      length = strlen((const char *)src->rete);
    if((length & 1)== 0)
        buf[2] = length / 2 + 1;
    else
        buf[2] =  (length+1) / 2 + 1;
    buf[3] = 0x91;

    length_dst += gsmbyte_string(buf,&dst[length_dst],4);
    length_dst += gsmby_normal(src->rete,&dst[length_dst],length);

    length_dst += gsmbyte_string(&src->tppid,&dst[length_dst],1);
    length_dst += gsmbyte_string(&src->tpdcs,&dst[length_dst],1);
    buf[0] = 0;
    length = strlen((const char *)src->tpdata);

    buf[1] = length;
 //   printf("length:::%x\n",length);
 //   sprintf(&buf[1],"%d",length);
//    printf("buf:::%x\n",buf[1]);
    length_dst += gsmbyte_string(buf,&dst[length_dst],2);
    if(src->tpdcs == GSM_7BIT)
    {
        length = gsmcode_7bit(src->tpdata,arr,(int)buf[1]-1);
        gsmbyte_string(arr,&dst[length_dst],length);
    }
    else if(src->tpdcs == GSM_8BIT)
    {
        length = gsmcode_8bit(src->tpdata,arr,(int)buf[1]-1);
        gsmbyte_string(arr,&dst[length_dst],length);
    }
    else if(src->tpdcs == GSM_UNICODE)
    {
        length = gsmcode_unic(src->tpdata,arr,(int)buf[1]-1);
        gsmbyte_string(arr,&dst[length_dst],length);
    }
    return length_dst;
}
/*****************************************
函数:gsmdecodepdu()
将pdu信息根据编码格式进行解码。
******************************************/

int gsmdecodepdu(uchar *src, gsmtp *gsm_data)
{
    uchar tmp;
    uchar ntmp;
    uchar length;
  //  unsigned char ncheap;
    int length_dst;
    uchar buf[MXA_SIZE] = {'\0'};
    uchar arr[MXA_SIZE] = {'\0'};

    gsmstring_byte(src,&tmp,2);
    src += 4;

    tmp = (tmp -1) * 2;
    length = gsmto_normal(src,buf,tmp);
    gsm_data->smsc[0] = '+';
    strncat((char *)gsm_data->smsc,(const char *)buf,length);

    src += tmp ;
    gsmstring_byte(src,&tmp,2);
    src += 2;

    gsm_data->tp_udhi = tmp & 0x40;

    gsmstring_byte(src,&tmp,2);
    if(tmp & 1)
        tmp += 1;

    src += 4;
    length = gsmto_normal(src,buf,tmp);
    gsm_data->rete[0] = '+';
    strncat((char *)gsm_data->rete,(const char *)buf,length);

    src += tmp;

    gsmstring_byte(src,&tmp,2);
  gsm_data->tppid = tmp ;
    src += 2;

    gsmstring_byte(src,&tmp,2);
    gsm_data->tpdcs = tmp;
    src += 2;
    gsmto_normal(src,buf,14);

    gsmstring_byte(buf,&gsm_data->timestamp.year,2);
    gsmstring_byte(buf+2,&gsm_data->timestamp.mouth,2);
    gsmstring_byte(buf+4,&gsm_data->timestamp.day,2);
    gsmstring_byte(buf+6,&gsm_data->timestamp.hour,2);
    gsmstring_byte(buf+8,&gsm_data->timestamp.minu,2);
    gsmstring_byte(buf+10,&gsm_data->timestamp.sec,2);
    gsmstring_byte(buf+12,&gsm_data->timestamp.timz,2);
    src += 14;
    gsmstring_byte(src,&tmp,2);
    ntmp = tmp;
    gsm_data->tplongth = ntmp;
    src += 2;
    if(gsm_data -> tp_udhi )
    {
        gsmstring_byte(src,&tmp,2);
        tmp = tmp + 1;
        src += tmp * 2;
        if(gsm_data->tpdcs == GSM_7BIT)
        {
            gsmstring_byte(src,&tmp,2);
            tmp = tmp >> 1;
            arr[0] = tmp;
            src += 2;
            memset(buf,0,MXA_SIZE);
            length_dst = gsmstring_byte(src,buf,(ntmp * 2 / 8) * 7 - 14);
            gsmdecode_7bit(buf,arr+1,length_dst);
            memset(gsm_data->tpdata,0,length_dst);
            memcpy(gsm_data->tpdata,arr,sizeof(arr));
        }
        else if(gsm_data->tpdcs == GSM_8BIT)
        {
            length_dst = gsmstring_byte(src,buf,(ntmp  - 12));
            memset(gsm_data->tpdata,0,length_dst);
            gsmdecode_8bit(buf,gsm_data->tpdata,length_dst);
        }
        else if(gsm_data->tpdcs == GSM_UNICODE)
        {
            length_dst = gsmstring_byte(src,buf,(ntmp * 2 - 12));
            memset(gsm_data->tpdata,0,length_dst);
            gsmdecode_unic(buf,gsm_data->tpdata,length_dst);
        }

    }
    else
    {
        if(gsm_data->tpdcs == GSM_7BIT)
        {
            memset(buf,0,MXA_SIZE);
            length_dst = gsmstring_byte(src,buf,(ntmp * 2 / 8) * 7 );
            memset(gsm_data->tpdata,0,length_dst);
            gsmdecode_7bit(buf,gsm_data->tpdata,length_dst);
        }
        else if(gsm_data->tpdcs == GSM_8BIT)
        {
            length_dst = gsmstring_byte(src,buf,ntmp);
            memset(gsm_data->tpdata,0,length_dst);
            gsmdecode_8bit(buf,gsm_data->tpdata,length_dst);
        }
        else if(gsm_data->tpdcs == GSM_UNICODE)
        {
            length_dst = gsmstring_byte(src,buf,(ntmp * 2));
    //        for(i = 0;i < length_dst;i++)
	//			printf("%x\n",buf[i]);
            memset(gsm_data->tpdata,0,length_dst);
            gsmdecode_unic(buf,gsm_data->tpdata,length_dst);
   //         printf("%s\n",gsm_data->tpdata);
        }
    }
    return 0;
}
/********************************
函数：stamp_check()
服务中心时间戳的数据的大致校验
正确与否
*********************************/
static void stamp_check(uchar *src, const struct tm *p)
{
    uchar nsrc;
    gsmstring_byte(src,&nsrc,2);
    if(p->tm_year - 100 < conversion(nsrc))
    {
        perror("数据有误，请核对！！\n");
        exit(0);
    }
    else if((p->tm_year - 100) > conversion(nsrc))
    {
        if(p->tm_mon < 0 || p->tm_mon >11)
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
        if(p->tm_mday < 1 || p->tm_mday >31)
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
        if((p->tm_hour+8) < 0 || (p->tm_hour+8) > 23 )
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
        if(p->tm_min < 0 || p->tm_min > 59)
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
        if(p->tm_sec < 0 || p->tm_sec > 59)
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
    }
    else
    {
        gsmstring_byte(src+2,&nsrc,2);
        if(p->tm_mon < conversion(nsrc))
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
        else if(p->tm_mon > conversion(nsrc))
        {
            if(p->tm_mday < 1 || p->tm_mday >31)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            if((p->tm_hour+8) < 0 || (p->tm_hour+8) > 23 )
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            if(p->tm_min < 0 || p->tm_min > 59)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            if(p->tm_sec < 0 || p->tm_sec > 59)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            gsmstring_byte(src+12,&nsrc,2);
        }
        else
        {
            gsmstring_byte(src+4,&nsrc,2);
            if(p->tm_mday < conversion(nsrc))
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            else if(p->tm_mday > conversion(nsrc))
            {
                if((p->tm_hour+8) < 0 || (p->tm_hour+8) > 23 )
                {
                    perror("数据有误，请核对！！\n");
                    exit(0);
                }
                if(p->tm_min < 0 || p->tm_min > 59)
                {
                    perror("数据有误，请核对！！\n");
                    exit(0);
                }
                if(p->tm_sec < 0 || p->tm_sec > 59)
                {
                    perror("数据有误，请核对！！\n");
                    exit(0);
                }
                gsmstring_byte(src+12,&nsrc,2);
            }
            else
            {
                gsmstring_byte(src+6,&nsrc,2);
                if(p->tm_hour < conversion(nsrc))
                {
                    perror("数据有误，请核对！！\n");
                    exit(0);
                }
                else if(p->tm_hour > conversion(nsrc))
                {
                    if(p->tm_min < 0 || p->tm_min > 59)
                    {
                        perror("数据有误，请核对！！\n");
                        exit(0);
                    }
                    if(p->tm_sec < 0 || p->tm_sec > 59)
                    {
                        perror("数据有误，请核对！！\n");
                        exit(0);
                    }
                }
                else
                {
                    gsmstring_byte(src+8,&nsrc,2);
                    if(p->tm_min < conversion(nsrc))
                    {
                        perror("数据有误，请核对！！\n");
                        exit(0);
                    }
                    else if(p->tm_min < conversion(nsrc))
                    {
                        if(p->tm_sec < 0 || p->tm_sec > 59)
                        {
                            perror("数据有误，请核对！！\n");
                            exit(0);
                        }
                    }
                    else
                    {
                        gsmstring_byte(src+10,&nsrc,2);
                        if(p->tm_sec < conversion(nsrc))
                        {
                            perror("数据有误，请核对！！\n");
                            exit(0);
                        }
                    }
                }
            }
        }
    }
}
/****************************
函数:conversion()
将单个16进制数转换为字面相同的
10进制数（比如:0x18---->18）;
*******************************/
static int conversion(uchar nsrc)
{
    return ((nsrc >> 4) * 10 + (nsrc & 0x0f));
}
/**********************************
函数：gsm_error()
大致的检测进行pdu解码的数据是否
正确;

*********************************/
static void gsm_error(uchar *src)
{
    uchar tmp;
    uchar ntmp;
    uchar dcs_flag;
    uchar udhi_flag;
    time_t timep;
    struct tm *p;
    uchar arr[MXA_SIZE] = {'\0'};
    time(&timep);
    p = gmtime(&timep);

    gsmstring_byte(src+2,&tmp,2);
    if(tmp != 0x91)
    {
         printf("%d\n",__LINE__);
        perror("数据有误，请核对！！\n");
        exit(0);
    }
    gsmstring_byte(src,&tmp,2);
    tmp = (tmp -1) * 2;
    src += (4 + tmp);
    gsmstring_byte(src,&tmp,2);
    udhi_flag = tmp & 0x40;
    src += 2;
    gsmstring_byte(src,&ntmp,2);
    if(ntmp & 1)
        ntmp += 1;
    src += 2;
    gsmstring_byte(src,&tmp,2);
//  printf("tmp:%x\n",tmp);
    if(tmp != 0x91)
    {
        printf("%d\n",__LINE__);
        perror("数据有误，请核对！！\n");
        exit(0);
    }
    src += (ntmp+2);
    gsmstring_byte(src,&tmp,2);
//     printf("tmp:%x\n",tmp);
    if(tmp != 0x00)
    {
        printf("%d\n",__LINE__);
        perror("数据有误，请核对！！\n");
        exit(0);
    }
    src += 2;
    gsmstring_byte(src,&tmp,2);
//   printf("tmp:%x\n",tmp);
    src += 2;
    dcs_flag = tmp;//判断doc
    if(dcs_flag != GSM_7BIT)
        if(dcs_flag != GSM_8BIT)
            if(dcs_flag != GSM_UNICODE)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
    gsmto_normal(src,arr,14);
    stamp_check(arr,p);//判断时间戳是否大致正确
    src += 14;
    gsmstring_byte(src,&tmp,2);
    src += 2;

    if(udhi_flag)
    {
        if(dcs_flag == GSM_7BIT)
        {
            gsmstring_byte(src,&tmp,2);
            if(tmp < 5)
            {
                printf("%d\n",__LINE__);
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            src += (tmp+1) * 2;
            ntmp = strlen((const char*)src);
            if(ntmp <= 0)
            {
                printf("%d\n",__LINE__);
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            src += (ntmp/2 - 1);
            gsmstring_byte(src,&tmp,2);
            if(tmp & 0x80)
            {
                printf("%d\n",__LINE__);
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
        else if(dcs_flag == GSM_8BIT)
        {
            ntmp = strlen((const char*)src);
            if(tmp*2 != ntmp)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
        else if(dcs_flag == GSM_UNICODE)
        {
            gsmstring_byte(src,&tmp,2);
            src += (tmp+1)*2;
            ntmp = strlen((const char*)src);
            if(ntmp <= 0 ||ntmp & 1 )
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
    }
    else
    {
        if(dcs_flag == GSM_7BIT)
        {
            ntmp = strlen((const char*)src);
            if(ntmp <= 0)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            src += (ntmp-2);
            gsmstring_byte(src,&tmp,2);
            if(tmp & 0x80)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
        else if(dcs_flag == GSM_8BIT)
        {
            ntmp = strlen((const char*)src);
            if(ntmp <= 0 || tmp*2 != ntmp)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
        else if(dcs_flag == GSM_UNICODE)
        {
            ntmp = strlen( (const char*)src);
            if(ntmp <= 0 || ntmp & 1)
            {
                printf("%d\n",__LINE__);
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
    }
}

int main()
{
   gsmtp gsm_data;
    uchar buf[] = "0891683108200075F1600D91683197701609F7000081400300803123A0050003BF020192CD625237ABE16C381C0C0683C160B11A8E059A16A5D6A2941563B5D1E4F0D985BEE3C761B96BFC6EB372B99C8C056381363C6B513A4D3E9D1B5F33584DBA62B1A1EB1AA3B96238D714EAB2CE5CC65A43B1E109AB492671E393C162B8178CF692E14032994E566B2892CD625237ABE16C381C0C0683C160B11A8E059A16A5D6A2941563B5D1";
    uchar dst[MXA_SIZE * 2] = {'\0'};
    gsm_error(buf);
    memset(&gsm_data,0,sizeof(gsmtp));
    gsmdecodepdu(buf,&gsm_data);
    printf("tpdata:%s\n",gsm_data.tpdata);

    printf("rete:%s\n",gsm_data.rete);
    printf("smsc:%s\n",gsm_data.smsc);
    printf("tppid:%d\n",gsm_data.tppid);
    printf("tpdcs:%d\n",gsm_data.tpdcs);
    printf("tplongth:%d\n",gsm_data.tplongth);

    memset(gsm_data.rete,0,sizeof(gsm_data.rete));
    memset(gsm_data.smsc,0,sizeof(gsm_data.smsc));
    strcpy((char *)gsm_data.rete,"8613546874589");
    strcpy((char *)gsm_data.smsc,"8613546874589");
    gsmcodepdu(dst,&gsm_data);

    printf("PDU:%s\n",dst);
   /*     int i;
    unsigned char *src = "92CD625237ABE1";
    unsigned char buf[16] = {'\0'};
    unsigned char arr[16] = {'\0'};
    gsmstring_byte(src,buf,14);
    for(i = 0;i < 7; i++)
        printf("buf:%x\n",buf[i]);
    gsmdecode_7bit(buf,arr,7);
    for(i = 0;i < 7; i++)
        printf("arr:%x\n",arr[i]);
    printf("%s\n",arr);
    char buf[5] = {'\0'};
    char *arr = buf;
    arr[0] = 1;
    arr[1] = 0;
    arr[2] = 0x03;
    arr[3] = 0x03;
    arr[4] = 0x03;
    printf("%c\n",arr[3]);
    int i;

    char *rete = "8613886215486";
    char *smsc = "8613569745862";
    char *data = "helloworld";
    gsmtp gsm_data;
    for(i = 0;i < 16;i++)
    {
        gsm_data.rete[i] = (uchar *)rete[i];
        gsm_data.smsc[i] = (uchar *)smsc[i];
        gsm_data.tpdata[i] = (uchar *)data[i];
    }
    gsm_data.tppid = 0x00;
    gsm_data.tpdcs = 0x00;


    uchar *arr = "你好！";
    uchar buf[20] = {'\0'};
    uchar tab[20] = {'\0'};
	int i;
    gsmcode_unic(arr,tab,20);
    for(i = 0;i< 6; i++)
		  printf("%x\n",tab[i]);
    gsmdecode_unic(tab,buf,20);
    printf("buf:%s\n",buf);*/
    while(1);

    return 0;
}
