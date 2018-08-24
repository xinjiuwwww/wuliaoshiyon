#include "main.h"
/**********************************
函数：gsmcode7bit()
用7bit编码方式编码pdu消息
************************************/

static int gsmcode_7bit(uchar* src,uchar  *dst,int length)
{
    int num;
    int i,j;
    uchar left_num;
    for(i = 0,j = 0;i <=length;i++)
    {
       num  = i & 7;
       if(num == 0)
            left_num = src[i];
       else{
        dst[j] = src[i] <<(8-num) | left_num;
        j++;
        left_num = src[i] >> num;
       }
    }

    return j;
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

    length_dst = mbstowcs(pwc,(const char *)src,length);
//    wprintf(pwc);
    for(i = 0,j = 0;i < length_dst;i++)
    {
        dst[j++] = pwc[i] >> 8;
        dst[j++] = pwc[i] & 0xff;
    }
    setlocale( LC_ALL, "C" );
    return length_dst*2;
}

/**********************************
函数：gsmdecode_unic()
以unic方式解码pdu消息
************************************/
static int gsmdecode_unic(uchar* src,uchar *dst, int length)
{
    wchar_t pwc[MXA_SIZE] = {'\0'};
    size_t i,j;
    size_t length_dst;
    setlocale(LC_ALL, "chs");
    for(i = 0,j = 0;i < length;i++)
    {
        pwc[i] = src[j++] << 8;
        pwc[i] = src[j++] | pwc[i];
    }
    memset(dst,0,MXA_SIZE);
    length_dst = wcstombs((char *)dst,(const wchar_t *)pwc,i);
    dst[length_dst] = '\0';
 //   printf("%s\n",dst);
    setlocale( LC_ALL, "C" );
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
    for(i = 0;i < length ;i ++)
    {
        *dst++ = table[*src >> 4];
    //  *dst++ = table[(*src << 4) >> 4]; 为什么这样有问题呢？？？
        *dst++ = table[*src & 0x0f]  ;
        src++;
    }
    *dst = '\0';
    return i*2;
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
    int ntmp = 0;
    uchar *aaa = NULL ;
    uchar temp[20] = {'\0'};
    uchar buf[MXA_SIZE] = {'\0'};
    uchar arr[MXA_SIZE*8] = {'\0'};
    if(src->tp_udhi == 1);
    else if(strlen((const char *)src->result_data) > LIMIT_SIZE)
    {
        src->tplongth = strlen((const char *)src->result_data);
        src->tp_udhi = 1;
    }
    length = strlen((const char*)src->smsc);
    if((length & 1) == 0)
        buf[0] = (char )length / 2 + 1;
    else
        buf[0] =  (char )(length+1) / 2 + 1;
    if(src->smsc[0] == '+')
    {
        length--;
        buf[1] = (char)0x91;
        memcpy(temp,&src->smsc[1],length);
    }
    else{
        buf[1] = (char)0xA0;
        memcpy(temp,src->smsc,length);
    }
    length_dst += gsmbyte_string(buf,dst,2);
    length_dst += gsmby_normal(temp, &dst[length_dst],length);
    if(src->tp_udhi == 1)
        buf[0] = 0x51; //发送模式
    else
        buf[0] = 0x11;
    buf[1] = 0;
    length = strlen((const char *)src->rete);
    if(src->rete[0] == '+')
    {
        buf[2] = length-1;
        length--;
        buf[3] = (char)0x91;
        memcpy(temp,&src->rete[1],length);
    }
    else
    {
        buf[2] = length;
        buf[3] = (char)0xA0;
        memcpy(temp,src->rete,length);
    }

    length_dst += gsmbyte_string(buf,&dst[length_dst],4);
    length_dst += gsmby_normal(temp,&dst[length_dst],length);
    length_dst += gsmbyte_string(&src->tppid,&dst[length_dst],1);
    length_dst += gsmbyte_string(&src->tpdcs,&dst[length_dst],1);
    buf[0] = 0;
    if(src->tp_udhi == 1)
    {
        if(src->merge_end == 0)
            src->merge_num++;
        src->merge_end++;
        if(src->tpdcs == GSM_7BIT)
        {

            ntmp = strlen((const char *)src->result_data);
            if(src->merge_flag[0] == '5')
            {
            	if (ntmp - (src->merge_end-1)* LIMIT_7BIT  > LIMIT_7BIT)
                length = LIMIT_7BIT+7;
                else if(src->merge_end == src->merge_num)
                length = ntmp - (src->merge_end-1) * LIMIT_7BIT + 7;
            }
            else if(src->merge_flag[0] == '6')
            {
                if(src->merge_end == 1 && src->merge_end != src->merge_num)
                length = LIMIT_7BIT+8;
                else if(src->merge_end == src->merge_num)
                length = ntmp - (src->merge_end-1) * LIMIT_7BIT + 8;
            }

            buf[1] = length;
            length_dst += gsmbyte_string(buf,&dst[length_dst],2);
            buf[0] = 0x05;
            buf[1] = 0x00;
            buf[2] = 0x03;
            buf[3] = ntmp % 0xff;
            if(src->merge_num-1 == 0)
            {
                aaa = (uchar *)malloc(sizeof(uchar) * ntmp * 2);
				memset(aaa, '\0', sizeof(uchar) * ntmp * 2);
				if(src->merge_flag[0] == '5')
                {
                    while( ntmp > LIMIT_7BIT)
                    {
                        src->tpdata[src->merge_num-1] = aaa + (src->merge_num-1)*LIMIT_7BIT*2;
                        src->tpdata[src->merge_num-1][0] = *(src->result_data+(src->merge_num-1) *LIMIT_7BIT) << 1;

                        gsmcode_7bit(src->result_data+(src->merge_num-1) *LIMIT_7BIT + 1,&src->tpdata[src->merge_num-1][1],LIMIT_7BIT-1);
                        ntmp -= LIMIT_7BIT;
                        src->merge_num++;
                    }
                    if(src->merge_num == 1)
                    {
                        arr[0] = src->result_data[0] << 1;
                        length = gsmcode_7bit(src->result_data+1,&arr[1],ntmp);
                        buf[4] = src->merge_num;
                        buf[5] = src->merge_end;
                        length_dst += gsmbyte_string(buf,&dst[length_dst],6);
                        gsmbyte_string(arr,&dst[length_dst],length);
                        return 0;
                    }
                    else {
                        src->tpdata[src->merge_num] = aaa + src->merge_num*LIMIT_7BIT*2;
                        src->tpdata[src->merge_num][0] = *(src->result_data+(src->merge_num) *LIMIT_7BIT) << 1;
                        gsmcode_7bit(src->result_data+(src->merge_num - 1) *LIMIT_7BIT + 1,&src->tpdata[src->merge_num - 1][1],ntmp - LIMIT_7BIT*(src->merge_num-1) - 1);
                    }
                    buf[4] = src->merge_num;
                    buf[5] = src->merge_end;
                    length_dst += gsmbyte_string(buf,&dst[length_dst],6);
                    gsmbyte_string(src->tpdata[src->merge_end - 1],&dst[length_dst],src->merge_end < src->merge_num ? LIMIT_7BIT : ntmp - LIMIT_7BIT*(src->merge_num-1));
                }
                if(src->merge_flag[0] == '6')
                {
                    while( ntmp > LIMIT_7BIT)
                    {
                        src->tpdata[src->merge_num-1] = aaa + (src->merge_num-1)*LIMIT_7BIT*2;
                        gsmcode_7bit(src->result_data+(src->merge_num-1) *LIMIT_7BIT ,src->tpdata[src->merge_num-1],LIMIT_7BIT);
                        ntmp -= LIMIT_7BIT;
                        src->merge_num++;
                    }
                    if(src->merge_num == 1)
                    {
                        length = gsmcode_7bit(src->result_data+1,arr,ntmp);
                        buf[4] = src->merge_num;
                        buf[5] = src->merge_end;
                        length_dst += gsmbyte_string(buf,&dst[length_dst],6);
                        gsmbyte_string(arr,&dst[length_dst],length-1);
                        return 0;
                    }
                    else {
                        src->tpdata[src->merge_num] = aaa + src->merge_num*LIMIT_7BIT*2;
                        gsmcode_7bit(src->result_data+(src->merge_num-1) *LIMIT_7BIT ,src->tpdata[src->merge_num-1],ntmp - LIMIT_7BIT*(src->merge_num-1));
                    }
                    buf[4] = src->merge_num;
                    buf[5] = src->merge_end;
                    length_dst += gsmbyte_string(buf,&dst[length_dst],6);
                    gsmbyte_string(src->tpdata[src->merge_end - 1],&dst[length_dst],src->merge_end < src->merge_num ? LIMIT_7BIT : ntmp - LIMIT_7BIT*(src->merge_num-1));
                }
            }
        }
        else if(src->tpdcs == GSM_8BIT)
        {
            if(src->merge_end == 1)
                length = LIMIT_SIZE;
            else if(src->merge_end == src->merge_num)
                length = src->tplongth - (src->merge_end-1) * LIMIT_SIZE;
            buf[1] = length+6;
            length_dst += gsmbyte_string(buf,&dst[length_dst],2);
            buf[0] = 0x05;
            buf[1] = 0x00;
            buf[2] = 0x03;
            buf[3] = src->tplongth % 0xff;
            if(src->merge_num == 0)
            {
                src->tplongth = strlen((const char *)src->result_data);
                ntmp = src->tplongth;
                while( ntmp > LIMIT_SIZE)
                {
                    ntmp -= LIMIT_SIZE;
                    src->merge_num++;
                }
                src->merge_num++;
            }
            buf[4] = src->merge_num;
            buf[5] = src->merge_end;
            length_dst += gsmbyte_string(buf,&dst[length_dst],6);
            length = gsmcode_8bit(src->result_data +((src->merge_end-1)*LIMIT_SIZE),arr,src->merge_end < src->\
                                  merge_num? LIMIT_SIZE :src->tplongth - (src->merge_end-1) * LIMIT_SIZE);
            gsmbyte_string(arr,&dst[length_dst],length);
        }
        else if(src->tpdcs == GSM_UNICODE)
        {

            ntmp = gsmcode_unic(src->result_data,arr,strlen((const char *)src->result_data));
            if(src->merge_end == 1 ||src->merge_end != src->merge_num)
            length = LIMIT_SIZE;
            else if(src->merge_end == src->merge_num)
                length = ntmp - (src->merge_end-1) * LIMIT_SIZE;
            buf[1] = length+6;
            length_dst += gsmbyte_string(buf,&dst[length_dst],2);
            buf[0] = 0x05;
            buf[1] = 0x00;
            buf[2] = 0x03;
            buf[3] = src->tplongth % 0xff;

            if(src->merge_num-1 == 0)
            {
                aaa = (uchar *)malloc(sizeof(uchar) * ntmp*3);
                memset(aaa,'\0',sizeof(uchar) * ntmp*3);
                while( ntmp > LIMIT_SIZE*src->merge_num)
                {
                    src->tpdata[src->merge_num-1] = aaa + (src->merge_num-1)*LIMIT_SIZE*2;
                    gsmbyte_string(arr+(src->merge_num-1)*LIMIT_SIZE,src->tpdata[src->merge_num-1],LIMIT_SIZE);
                    src->merge_num++;
                }
                src->tpdata[src->merge_num - 1 ] = aaa + (src->merge_num-1)*LIMIT_SIZE*2;
                gsmbyte_string(arr+(src->merge_num-1)*LIMIT_SIZE,src->tpdata[src->merge_num-1],ntmp - (src->merge_num-1) * LIMIT_SIZE);
                //free(aaa);
            }
            buf[4] = src->merge_num;
            buf[5] = src->merge_end;
            length_dst += gsmbyte_string(buf,&dst[length_dst],6);
            memcpy(&dst[length_dst],src->tpdata[src->merge_end-1],src->merge_end < src->merge_num ? LIMIT_SIZE*2 : (ntmp - (src->merge_end - 1) * LIMIT_SIZE)*2);
        }
    }
    else{
        ntmp = strlen((const char *)src->result_data);
        buf[1] = ntmp;
        length_dst += gsmbyte_string(buf,&dst[length_dst],2);
        if(src->tpdcs == GSM_7BIT)
        {
            length = gsmcode_7bit(src->result_data,arr,ntmp);
            gsmbyte_string(arr,&dst[length_dst],length);
        }
        else if(src->tpdcs == GSM_8BIT)
        {
            length = gsmcode_8bit(src->result_data,arr,ntmp);
            gsmbyte_string(arr,&dst[length_dst],length);
        }
        else if(src->tpdcs == GSM_UNICODE)
        {
            length = gsmcode_unic(src->result_data,arr,ntmp);
            gsmbyte_string(arr,&dst[length_dst],length);
        }
    }
    return length_dst;
}
/*****************************************
函数:gsmdecodepdu()
将pdu信息根据编码格式进行解码。
******************************************/

int gsmdecodepdu(uchar *src, gsmtp *gsm_data)
{
	uchar  index = 0;
    uchar  tmp = 0;
    int ntmp = 0;
    int length = 0;
    uchar phone_flag = 0;
  // unsigned char ncheap;
    int length_dst = 0;
    uchar buf[MXA_SIZE] = {'\0'};
    uchar arr[MXA_SIZE] = {'\0'};

    gsmstring_byte(src,&tmp,2);
    tmp = (tmp -1) * 2;
    src += 2;
    gsmstring_byte(src,&phone_flag,2);
    src += 2;

    length = gsmto_normal(src,buf,tmp);
    if(gsm_data->merge_end == 0)
    {
        if(!(phone_flag & 0x20))
        {
            gsm_data->smsc[0] = '+';
            strncat((char *)(&gsm_data->smsc[1]),(const char *)buf,length);
        }
        else
            strncat((char *)gsm_data->smsc,(const char *)buf,length);
    }
    src += tmp ;
    gsmstring_byte(src,&tmp,2);
    gsm_data->pdu_mti = tmp & 0x01;
    printf("%d\n",gsm_data->pdu_mti);

    gsm_data->tp_udhi = (tmp & 0x40) >> 6;
    if(gsm_data->pdu_mti)
        src += 4;
    else
        src += 2;
    gsmstring_byte(src,&tmp,2);
    if(tmp & 1)
        tmp += 1;

    src += 2;
    gsmstring_byte(src,&phone_flag,2);
    src += 2;
    length = gsmto_normal(src,buf,tmp);
    if(gsm_data->merge_end == 0)
    {
        if(!(phone_flag & 0x20))
        {
            gsm_data->rete[0] = '+';
            strncat((char *)(&gsm_data->rete[1]),(const char *)buf,length);
        }
        else
            strncat((char *)gsm_data->rete,(const char *)buf,length);
    }
    src += tmp;

    gsmstring_byte(src,&tmp,2);
    gsm_data->tppid = tmp ;
    src += 2;

    gsmstring_byte(src,&tmp,2);
    gsm_data->tpdcs = tmp;
    src += 2;
    if(gsm_data->pdu_mti == 0)
    {
        gsmto_normal(src,buf,14);
        gsmstring_byte(buf,&gsm_data->timestamp.year,2);
        gsmstring_byte(buf+2,&gsm_data->timestamp.mouth,2);
        gsmstring_byte(buf+4,&gsm_data->timestamp.day,2);
        gsmstring_byte(buf+6,&gsm_data->timestamp.hour,2);
        gsmstring_byte(buf+8,&gsm_data->timestamp.minu,2);
        gsmstring_byte(buf+10,&gsm_data->timestamp.sec,2);
        gsmstring_byte(buf+12,&gsm_data->timestamp.timz,2);
        gsm_data->timestamp.year = conversion(gsm_data->timestamp.year);
        gsm_data->timestamp.mouth = conversion(gsm_data->timestamp.mouth);
        gsm_data->timestamp.day = conversion(gsm_data->timestamp.day);
        gsm_data->timestamp.hour = conversion(gsm_data->timestamp.hour);
        gsm_data->timestamp.minu = conversion(gsm_data->timestamp.minu);
        gsm_data->timestamp.sec = conversion(gsm_data->timestamp.sec);
        gsm_data->timestamp.timz = conversion(gsm_data->timestamp.timz) - 24;
        src += 14;
    }
    else
        src += 2;
    gsmstring_byte(src,&tmp,2);
    ntmp = tmp;
    src += 2;

    if(gsm_data -> tp_udhi )
    {
      	gsmstring_byte(src,&tmp,2);
      	memcpy(gsm_data->udli_data,src,(tmp+1)*2);
	  	if(tmp == 0x05)
	  	{
			src += 6;
			gsmstring_byte(src,&tmp,2);
			if(gsm_data->merge_flag[0] == '\0')
			{
                gsm_data->merge_end = 0;
				gsm_data->merge_flag[0] = tmp;
				src += 2;
				gsmstring_byte(src,&tmp,2);
				gsm_data->merge_num= tmp;
				src += 2;
				gsmstring_byte(src,&index,2);
				index--;
				gsm_data->tpdata[index] = (uchar *)malloc(sizeof(uchar)*MXA_SIZE);
				if(gsm_data->tpdata[index] == NULL)
				{
					perror("gsm_data->tpdata的malloc()失败!");
					exit(0);
				}
				src += 2;
			}
			else if(gsm_data->merge_flag[0] != tmp)
			{
				perror("前后不是同一条短信！");
				exit(0);
			}
			if(gsm_data->tpdcs == GSM_7BIT)
	        {
	            gsmstring_byte(src,&tmp,2);
	            tmp = tmp >> 1;
	            arr[0] = tmp;
	            src += 2;
	            memset(buf,0,MXA_SIZE);

	            length_dst = gsmstring_byte(src,buf,(ntmp * 2 / 8) * 7 - 12);
	            gsmdecode_7bit(buf,arr+1,length_dst);
	            memset(gsm_data->tpdata[index],'\0',length_dst);
	            memcpy(gsm_data->tpdata[index],arr,sizeof(arr));
	            gsm_data->merge_end++;
	        }
	        else if(gsm_data->tpdcs == GSM_8BIT)
	        {
	            length_dst = gsmstring_byte(src,buf,(ntmp  - 12));
	            memset(gsm_data->tpdata[index],0,length_dst);
	            gsmdecode_8bit(buf,gsm_data->tpdata[index],length_dst);
	            gsm_data->merge_end++;
	        }
	        else if(gsm_data->tpdcs == GSM_UNICODE)
	        {
	            length_dst = gsmstring_byte(src,buf,(ntmp * 2 - 12));
	            memset(gsm_data->tpdata[index],0,length_dst);
	            gsmdecode_unic(buf,gsm_data->tpdata[index],length_dst);
	            gsm_data->merge_end++;
	        }
		}
	   	else if(tmp == 0x06)
	   	{
	   		src += 6;
			gsmstring_byte(src,buf,4);
			if(gsm_data->merge_flag[0] == 0 && gsm_data->merge_flag[1] == 0)
			{
			    gsm_data->merge_end = 0;
				gsm_data->merge_flag[0] = buf[0];
				gsm_data->merge_flag[1] = buf[1];
				src += 4;
				gsmstring_byte(src,&tmp,2);
				gsm_data->merge_num = tmp;
				src += 2;
				gsmstring_byte(src,&index,2);
				index--;
				gsm_data->tpdata[index] = (uchar *)malloc(sizeof(uchar)*MXA_SIZE);
				if(gsm_data->tpdata[index] == NULL)
				{
					perror("gsm_data->tpdata的malloc()失败!");
					exit(0);
				}
				src += 2;
				if(gsm_data->tpdcs == GSM_7BIT)
                {
                    memset(buf,0,MXA_SIZE);
                    length_dst = gsmstring_byte(src,buf,(ntmp * 2 / 8) * 7 - 14);
                    memset(gsm_data->tpdata[index],'\0',length_dst);
                    gsmdecode_7bit(buf,gsm_data->tpdata[index],length_dst);
                    gsm_data->merge_end++;
                }
                else if(gsm_data->tpdcs == GSM_8BIT)
                {
                    length_dst = gsmstring_byte(src,buf,(ntmp  - 14));
                    memset(gsm_data->tpdata[index],0,length_dst);
                    gsmdecode_8bit(buf,gsm_data->tpdata[index],length_dst);
                    gsm_data->merge_end++;
                }
                else if(gsm_data->tpdcs == GSM_UNICODE)
                {
                    length_dst = gsmstring_byte(src,buf,(ntmp * 2 - 14));
                    memset(gsm_data->tpdata[index],0,length_dst);
                    gsmdecode_unic(buf,gsm_data->tpdata[index],length_dst);
                    gsm_data->merge_end++;
                }
                gsm_data->tplongth += ntmp;
                return 0;
            }
            else if(buf[0] == gsm_data->merge_flag[0] && buf[1] == gsm_data->merge_flag[1])
            {
                src += 6;
                gsmstring_byte(src,&index,2);
                index--;
                gsm_data->tpdata[index] = (uchar *)malloc(sizeof(uchar)*MXA_SIZE);
				if(gsm_data->tpdata[index] == NULL)
				{
					perror("gsm_data->tpdata的malloc()失败!");
					exit(0);
				}
                src += 2;
                if(gsm_data->tpdcs == GSM_7BIT)
                {
                    memset(buf,0,MXA_SIZE);
                    length_dst = gsmstring_byte(src,buf,(ntmp * 2 / 8) * 7 - 14);
                    memset(gsm_data->tpdata[index],'\0',length_dst);
                    gsmdecode_7bit(buf,gsm_data->tpdata[index],length_dst);
                    gsm_data->merge_end++;
                }
                else if(gsm_data->tpdcs == GSM_8BIT)
                {
                    length_dst = gsmstring_byte(src,buf,(ntmp  - 14));
                    memset(gsm_data->tpdata[index],0,length_dst);
                    gsmdecode_8bit(buf,gsm_data->tpdata[index],length_dst);
                    gsm_data->merge_end++;
                }
                else if(gsm_data->tpdcs == GSM_UNICODE)
                {
                    length_dst = gsmstring_byte(src,buf,(ntmp * 2 - 14));
                    memset(gsm_data->tpdata[index],0,length_dst);
                    gsmdecode_unic(buf,gsm_data->tpdata[index],length_dst);
                    gsm_data->merge_end++;
                }
            }
			else
			{
			    printf("%d,%d\n",buf[0],buf[1]);
				perror("前后不是同一条短信！");
				exit(0);
			}
	   	}
    }
    else
    {
    	gsm_data->tpdata[0] = (uchar*)malloc(sizeof(uchar ) * MXA_SIZE);
    	if(gsm_data->tpdata[tmp] == NULL)
		{
			perror("gsm_data->tpdata的malloc()失败!");
			exit(0);
		}
		memset(buf,0,MXA_SIZE);
		memset(gsm_data->tpdata[0],0,MXA_SIZE);
        if(gsm_data->tpdcs == GSM_7BIT)
        {
            length_dst = gsmstring_byte(src,buf,(ntmp * 2 / 8) * 7 );
            gsmdecode_7bit(buf,gsm_data->tpdata[0],length_dst);
        }
        else if(gsm_data->tpdcs == GSM_8BIT)
        {
            length_dst = gsmstring_byte(src,buf,ntmp);
            gsmdecode_8bit(buf,gsm_data->tpdata[0],length_dst);
        }
        else if(gsm_data->tpdcs == GSM_UNICODE)
        {
            length_dst = gsmstring_byte(src,buf,(ntmp * 2));
            gsmdecode_unic(buf,gsm_data->tpdata[0],length_dst);
        }
    }
      gsm_data->tplongth += ntmp;
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
        if(((p->tm_hour+8) % 23) < 0 || ((p->tm_hour+8)% 23) > 23 )
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
        if(p->tm_mon+1 < conversion(nsrc))
        {
            perror("数据有误，请核对！！\n");
            exit(0);
        }
        else if(p->tm_mon+1 > conversion(nsrc))
        {
            if(p->tm_mday < 1 || p->tm_mday >31)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            if(((p->tm_hour+8) % 23) < 0 || ((p->tm_hour+8)% 23) > 23 )
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
                if(((p->tm_hour+8) % 23) < 0 || ((p->tm_hour+8)% 23) > 23 )
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
                if(((p->tm_hour+8)%23) < conversion(nsrc))
                {
                    perror("数据有误，请核对！！\n");
                    exit(0);
                }
                else if(((p->tm_hour+8)%23) > conversion(nsrc))
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
    src += (ntmp+2);
    gsmstring_byte(src,&tmp,2);
//     printf("tmp:%x\n",tmp);
    if(tmp != 0x00)
    {
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
            if(tmp < 0x05 || tmp > 0x06)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            src += (tmp+1) * 2;
            ntmp = strlen((const char*)src);
            if(ntmp <= 0)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            src += (ntmp/2 - 1);
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
            if(tmp*2 != ntmp)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        	gsmstring_byte(src,&tmp,2);
            if(tmp < 0x05 || tmp > 0x06)
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
        else if(dcs_flag == GSM_UNICODE)
        {
            ntmp = strlen((const char*)src);
            if(ntmp <= 0 || ntmp & 1 )
            {
                perror("数据有误，请核对！！\n");
                exit(0);
            }
            gsmstring_byte(src,&tmp,2);
            if(tmp < 0x05 || tmp > 0x06)
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
                perror("数据有误，请核对！！\n");
                exit(0);
            }
        }
    }
}

int main()
{
/*    int i;
    gsmtp gsm_data;
    uchar *arr[10];

    uchar buf1[] ="0891683108200075F16005A10180F60008818061010561238B06080419420301301079FB52A8597D7F514E0D965091CFFF0C603B67094E006B3E900254084F60FF0C731B6233002000640078002E00310030003000380036002E0063006E002F00620078006C006800300035002030115C0A656C76845BA26237FF1A60A85F53524D8D2662374F59989D003100340031002E003500395143FF0C4E0B4E004E2A67087ED3";
    uchar buf2[] ="0891683108200075F16405A10180F6000881806101056123850608041942030265E54E3A00320030003100385E740030003867080032003765E53002598297005145503C53EF70B951FB00200068007400740070003A002F002F00670064002E00310030003000380036002E0063006E002F0063007A002030023010003051439886526F536183B78D600031003000476D4191CFFF1A0020006800740074";
    uchar buf3[] ="0891683108200075F16405A10180F600088180610105612345060804194203030070003A002F002F00640078002E00310030003000380036002E0063006E002F0079004A004600760059003300710020301130104E2D56FD79FB52A83011";
    uchar dst[MXA_SIZE * 4] = {'\0'};

    //gsm_error(buf1);
    memset(&gsm_data,'\0',sizeof(gsmtp));

    gsmdecodepdu(buf1,&gsm_data);
    gsmdecodepdu(buf2,&gsm_data);
    gsmdecodepdu(buf3,&gsm_data);
    if(gsm_data.merge_num == gsm_data.merge_num)
    {
        strcat((char*)dst,(const char*)gsm_data.tpdata[0]);
        strcat((char*)dst,(const char*)gsm_data.tpdata[1]);
        strcat((char*)dst,(const char*)gsm_data.tpdata[2]);
        gsm_data.result_data = dst;
    }
    printf("RETE:%s\n",gsm_data.rete);
    printf("SMSC:%s\n",gsm_data.smsc);
    printf("TPPID:%d\n",gsm_data.tppid);
    printf("TPDCS:%d\n",gsm_data.tpdcs);
    printf("TPLongth:%d\n",gsm_data.tplongth);
    printf("time:%d-%d-%d,%d-%d-%d,时区:%d\n",gsm_data.timestamp.year+2000,gsm_data.timestamp.mouth,gsm_data.timestamp.day,\
           gsm_data.timestamp.hour,gsm_data.timestamp.minu,gsm_data.timestamp.sec,gsm_data.timestamp.timz);
    printf("%s\n",gsm_data.result_data);
    printf("tpdata:%d\n",strlen((const char*)gsm_data.result_data));
    for(i = 0;i < gsm_data.merge_num;i++)
        free(gsm_data.tpdata[i]);


    printf("\n");
    memset(&gsm_data,0,sizeof(gsm_data));
    strcpy((char *)gsm_data.rete,"+8613546874589");
    strcpy((char *)gsm_data.smsc,"+8615685487595");
    gsm_data.tpdcs = GSM_UNICODE;
    gsm_data.result_data = dst;

    arr[0] = malloc(sizeof(uchar) * MXA_SIZE*2);
    memset(arr[0],'\0',MXA_SIZE*2);
    gsmcodepdu(arr[0],&gsm_data);
    printf("PDU:%s\n",arr[0]);
    for( i = 1;i < gsm_data.merge_num;i++)
    {
        arr[i] = malloc(sizeof(uchar) * MXA_SIZE*2);
        memset(arr[i],'\0',MXA_SIZE*2);
        gsmcodepdu(arr[i],&gsm_data);
        printf("PDU:%s\n",arr[i]);
    }
    for(i = 0;i < gsm_data.merge_num;i++)
        free(arr[i]);
    fgets(dst,10,stdin);
*/
/*
    gsmtp gsm_data;
    uchar *arr[10];
    printf("\n");
    memset(&gsm_data,0,sizeof(gsm_data));
    strcpy((char *)gsm_data.rete,"+8613546874589");
    strcpy((char *)gsm_data.smsc,"+8615685487595");
    gsm_data.tpdcs = GSM_7BIT;
 //   gsm_data.result_data = dst;
    gsm_data.result_data = "asfkjaslkfhaoishfqoihfapslkhbfaoiehfas";
    arr[0] = malloc(sizeof(uchar) * MXA_SIZE*2);
    gsmcodepdu(arr[0],&gsm_data);
    printf("PDU:%s\n",arr[0]);
    for(int i = 1;i < gsm_data.merge_num;i++)
    {
        arr[i] = malloc(sizeof(uchar) * MXA_SIZE*2);
        memset(arr[i],'\0',MXA_SIZE*2);
        gsmcodepdu(arr[i],&gsm_data);
        printf("PDU:%s\n",arr[i]);
    }
    for(i = 0;i < gsm_data.merge_num;i++)

    uchar buf[] ="0891683108100065F9640DA0014698090115F000008170919002302315050003080101A6D420B53A65D96C369BCD3602";
    uchar arr[256] = {'\0'};
    gsmtp gsm_data;
    memset(&gsm_data,'\0',sizeof(gsmtp));
    gsmdecodepdu(buf,&gsm_data);
    printf("%s\n",gsm_data.tpdata[0]);
    printf("RETE:%s\n",gsm_data.rete);
    printf("SMSC:%s\n",gsm_data.smsc);
    printf("TPPID:%d\n",gsm_data.tppid);
    printf("TPDCS:%d\n",gsm_data.tpdcs);
    printf("TPLongth:%d\n",gsm_data.tplongth);
    printf("udli_data:%s\n",gsm_data.udli_data);
    printf("udhi:%d\n",gsm_data.tp_udhi);
    gsm_data.tp_udhi = 0;
    gsm_data.result_data = gsm_data.tpdata[0];
 //   gsmcode_7bit(gsm_data.result_data,arr,strlen(gsm_data.result_data));
 //   printf("%s\n",arr);
  //  while(1);
    printf("gsm_data.result_data%s\n",gsm_data.result_data);
    gsm_data.merge_end = 0;
    gsm_data.merge_num = 0;
    memset(gsm_data.merge_flag,'\0',2);
    gsm_data.merge_flag[0] = gsm_data.udli_data[1];
    memcpy(gsm_data.smsc,"+8613414624039",16);
    memcpy(gsm_data.rete,"+8613883071804",16);
//    memcpy(arr,gsm_data.smsc,sizeof(gsm_data.smsc));
 //   memcpy(gsm_data.smsc,gsm_data.rete,sizeof(gsm_data.rete));
//    memcpy(gsm_data.rete,arr,sizeof(arr));

    printf("RETE:%s\n",gsm_data.rete);
    printf("SMSC:%s\n",gsm_data.smsc);
    gsmcodepdu(arr,&gsm_data);
    printf("arr::%s\n",arr);*/

    int opt = 0;
    int i = 0;
    int End = 1;
    char FlagEed = 0;
    uchar *DstBuf[10];
    gsmtp gsm_data;
    while(End)
    {
        printf("1.编码，2.解码，3.退出\n");
        memset(&gsm_data,0,sizeof(gsm_data));
        scanf("%d",&opt);
        switch(opt)
        {
            case 1:
                opt = 0;
                printf("smsc:\n");
                fgetc(stdin);
                fgets((char*)gsm_data.smsc,PHONE_NUM,stdin);

                printf("rete:\n");
                fgets((char*)gsm_data.rete,PHONE_NUM,stdin);
                gsm_data.smsc[strlen((const char*)gsm_data.smsc)-1] = '\0';
                gsm_data.rete[strlen((const char*)gsm_data.rete)-1] = '\0';
                printf("%s\n",gsm_data.smsc);
                printf("%s\n",gsm_data.rete);
                while(End)
                {
                    printf("1.7BIT,2.8BIT,3.UCS2,4.Exit\n");
                    scanf("%d",&opt);
                    switch(opt)
                    {
                        case 1:
                            gsm_data.tpdcs = GSM_7BIT;
                            break;
                        case 2:
                            gsm_data.tpdcs = GSM_8BIT;
                            break;
                        case 3:
                            gsm_data.tpdcs = GSM_UNICODE;
                            break;
                        default:
                            End = 0;
                            continue;
                            break;
                    }
                    printf("是否需要带有数据头信息y/n\n");
                    fgetc(stdin);
                    scanf("%c", &FlagEed);
                    if (FlagEed == 'y')
                    {
                        gsm_data.tp_udhi = 1;
                        gsm_data.merge_flag[0] = '5';

                    }
                    else;
                    gsm_data.result_data = (uchar *)malloc(sizeof(uchar)* MXA_SIZE*2);
                    if(gsm_data.result_data == NULL)
                    {
                        perror("gsm_data.result_data() malloc is fail!!");
                        exit(0);
                    }
                    printf("请输入短信内容：\n");
                    fgetc(stdin);
                    fgets((char*)gsm_data.result_data,MXA_SIZE*8,stdin);
                    gsm_data.result_data[strlen((const char*)gsm_data.result_data)-1] = '\0';
                    printf("%s\n",gsm_data.result_data);
                    DstBuf[0] = (uchar *)malloc(sizeof(uchar) * MXA_SIZE*2);
                    if(DstBuf[0] == NULL)
                    {
                        perror("DstBuf[] malloc is fail!!");
                        exit(0);
                    }
                    memset(DstBuf[0],'\0',MXA_SIZE*2);
                    gsmcodepdu(DstBuf[0],&gsm_data);
                    printf("PDU:%s\n",DstBuf[0]);
                    if(gsm_data.merge_num)
                    {
                        for(i = 1;i < gsm_data.merge_num;i++)
                        {

                            DstBuf[i] = (uchar *)malloc(sizeof(uchar) * MXA_SIZE*2);
                            if(DstBuf[i] == NULL)
                            {
                                perror("DstBuf[] malloc is fail!!");
                                exit(0);
                            }
                            memset(DstBuf[i],'\0',MXA_SIZE*2);
                            gsmcodepdu(DstBuf[i],&gsm_data);
                            printf("PDU:%s\n",DstBuf[i]);
                        }
                    }
                    if(gsm_data.merge_num == 0)
                        gsm_data.merge_num = 1;
                    for(i = 0;i < gsm_data.merge_num;i++)
                        free(DstBuf[i]);
                }
                i = 0;
                End = 1;
            break;
        case 2:
            while(1)
            {
                printf("请输入需要解码的短信信息\n");
                DstBuf[i] = (uchar *)malloc(sizeof(uchar) * MXA_SIZE);
                if(DstBuf[i] == NULL)
                {
                    perror("DstBuf[] malloc is fail!!");
                    exit(0);
                }
                memset(DstBuf[i],'\0',MXA_SIZE);
                fgetc(stdin);
                fgets((char*)DstBuf[i],MXA_SIZE,stdin);

                DstBuf[i][strlen((const char*)DstBuf[i])-1] = '\0';
                gsm_error(DstBuf[i]);
                gsmdecodepdu(DstBuf[i],&gsm_data);
                printf("是否还需要输入短信y/n\n");
                scanf("%c",&FlagEed);

                if(FlagEed == 'y')
                {
                    i++;
                    continue;
                }
                else
                    break;
            }
            gsm_data.result_data = (uchar *)malloc(sizeof(uchar) * MXA_SIZE*16);
            if(gsm_data.result_data == NULL)
            {
                perror("解码:gsm_data.result_data malloc is fail!!");
                exit(0);
            }
            memset(gsm_data.result_data,'\0',MXA_SIZE*16);
            if(gsm_data.merge_num != 0)
                if(gsm_data.merge_num != gsm_data.merge_end)
                {
                    printf("解码信息不完整，是否打印！y/n\n");
                    fgetc(stdin);
                    scanf("%c",&FlagEed);
                    if(FlagEed == 'y')
                    {
                        for(i = 0;i < gsm_data.merge_end;i++)
                            strcat((char*)gsm_data.result_data,(const char*)gsm_data.tpdata[i]);
                        printf("**********************************\n");
                        if(gsm_data.pdu_mti == 0)
                        {
                            printf("这是一条接受的信息。\n");
                            printf("Time:%d-%d-%d,%d-%d-%d,时区:%d\n",gsm_data.timestamp.year+2000,gsm_data.timestamp.mouth,gsm_data.timestamp.day,\
                                                                    gsm_data.timestamp.hour,gsm_data.timestamp.minu,gsm_data.timestamp.sec,gsm_data.timestamp.timz);
                        }
                        else
                             printf("这是一条发送的信息。\n");
                        printf("RETE:%s\n",gsm_data.rete);
                        printf("SMSC:%s\n",gsm_data.smsc);
                        printf("Content:%s\n",gsm_data.result_data);
                        printf("**********************************\n");
                        for(i = 0;i < gsm_data.merge_end;i++)
                            free(gsm_data.tpdata[i]);
                        free(gsm_data.result_data);
                        i = 0;
                        End = 1;
                        break;
                    }
                    else{
                        perror("解码信息不完整，请查证！！已退出\n");
                        exit(0);
                    }
                }
            if(gsm_data.merge_num == 0)
                gsm_data.merge_num = 1;
            for(i = 0;i < gsm_data.merge_num;i++)
                strcat((char*)gsm_data.result_data,(const char*)gsm_data.tpdata[i]);
            printf("**********************************\n");

            if(gsm_data.pdu_mti == 0)
            {
                printf("这是一条接受的信息。\n");
                printf("Time:%d-%d-%d,%d-%d-%d,时区:%d\n",gsm_data.timestamp.year+2000,gsm_data.timestamp.mouth,gsm_data.timestamp.day,\
                                                                    gsm_data.timestamp.hour,gsm_data.timestamp.minu,gsm_data.timestamp.sec,gsm_data.timestamp.timz);
            }
            else
                printf("这是一条发送的信息。\n");
            printf("RETE:%s\n",gsm_data.rete);
            printf("SMSC:%s\n",gsm_data.smsc);
            printf("Content:%s\n",gsm_data.result_data);
            printf("**********************************\n");
            for(i = 0;i < gsm_data.merge_num;i++)
                free(gsm_data.tpdata[i]);
            free(gsm_data.result_data);
            i = 0;
            End = 1;
            break;
        default:
            End = 0;
            continue;
            break;
        }
    }
    return 0;
}
