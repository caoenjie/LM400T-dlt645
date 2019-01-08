/*
 * dlt645.c
 *
 *  Created on: 2019年1月4日
 *      Author: Black
 */
#include "dlt645.h"
#include "typedef.h"
#include "am_vdebug.h"
#include <string.h>

#include "ametal.h"
#include "am_fsl_uart.h"
#include "am_uart_rngbuf.h"
#include "am_kl26_inst_init.h"
#include "am_board.h"

#include "am_kl26.h"
#include "amhw_fsl_uart.h"
/*
 * meter address
 */
static uint8_t MeterAddress[DLT645_ADDRESS_LEN];

/*
 *
 */
#define UART1_RX_BUF_SIZE  128  /**< \brief 接收环形缓冲区大小，应该为2^n  */
#define UART1_TX_BUF_SIZE  128  /**< \brief 发送环形缓冲区大小，应该为2^n  */
static am_uart_rngbuf_handle_t  handle;
static am_uart_handle_t         uart_handle;   /**< \brief 串口标准服务句柄      */
static am_uart_rngbuf_dev_t     uart2_ringbuf_dev; /** \brief 串口缓冲区设备      */
static uint8_t uart1_rxbuf[UART1_RX_BUF_SIZE]; /** \brief UART接收环形缓冲区  */
static uint8_t uart1_txbuf[UART1_TX_BUF_SIZE]; /** \brief UART发送环形缓冲区  */


//int strncmp(const uint8_t *s1,const uint8_t *s2,int n)
//{
////	assert((s1!=NULL)&&(s2!=NULL));
//	if(s1 == NULL || s2 == NULL)
//		return -1;
//
//	while(*s1!='\0'&&*s2!='\0'&&n)//字符串前面部分都同样
//	{
//		if(*s1-*s2>0)
//			return 1;
//		if(*s1-*s2<0)
//			return -1;
//		s1++;
//		s2++;
//		n--;
//	}
//	if(*s1=='\0'&&*s2!='\0')//谁先为'\0',谁就小
//		return -1;
//	if(*s2=='\0'&&*s1!='\0')
//		return 1;
//	return 0;              //同一时候为'\0'
//}

void DLT645_UartInit()
{
    /* UART初始化 */
    uart_handle = am_kl26_uart2_inst_init();
    amhw_fsl_uart_parity_set(AMHW_KL26_UART2,  AMHW_FSL_UART_C1_PARITY_EVEN);
	amhw_fsl_uart_data_mode_set(AMHW_KL26_UART2, AMHW_FSL_UART_C1_M_9BIT);

    /* UART初始化为环形缓冲区模式 */
    handle = am_uart_rngbuf_init(&uart2_ringbuf_dev,
                                 uart_handle,
                                 uart1_rxbuf,
                                 UART1_RX_BUF_SIZE,
                                 uart1_txbuf,
                                 UART1_TX_BUF_SIZE);
    uint32_t timeout= 30;
    am_uart_rngbuf_ioctl(handle,AM_UART_RNGBUF_TIMEOUT,(void*)(timeout));

}


void DLT645_InitMeterAddress(void)
{
	if(!IsMeterAddress)
		memset(MeterAddress,0xaa,sizeof(MeterAddress));
}

int DLT645_IsMeterAddress(void)
{
	return IsMeterAddress;
}

void DLT645_Init()
{
	DLT645_UartInit();
	if(!IsMeterAddress)
		DLT645_InitMeterAddress();
}

uint8_t Dlt645_GetChecksum(uint8_t data[], int len)
{
	uint8_t sum = 0;

	for(int i=0; i<len; ++i)
	{
		sum += data[i];
	}

	return sum;
}


uint8_t* Dlt645_CreateSendPkt(int type, uint32_t dataTag, int *Ptklen)
{
	int datalen;
	int pktLen;
	uint8_t data[DLT645_MAX_DATA_LEN];

	switch(type){
	case DLT645_PKT_TYPE_READ_DATA:

		datalen = DLT645_DATA_TAG_LEN;
		memcpy(data, &dataTag, sizeof(dataTag));
		break;
	case DLT645_PKT_TYPE_READ_ADDRESS:
		datalen = 0;
		break;

	default:
		am_kprintf("what the hell type?\n");
	}

	pktLen = datalen + DLT645_HEAD_TAIL_LEN;
	*Ptklen = pktLen;
	uint8_t *pkt = (uint8_t *)malloc(pktLen);

	/*add 4 leading bytes*/
	memset(pkt, DLT645_LEADING_BYTE, 4);

	pkt[4] = DLT645_START_BYTE;		// start byte

	memcpy(&pkt[5], MeterAddress, 6);

	pkt[11] = DLT645_START_BYTE;
	pkt[12] = DLT645_MASTER_QUERY | type;		//function code
	pkt[13] = datalen;

	if(datalen)
	{
		for(int i=0; i<datalen; ++i)
		{
			pkt[14+i] = data[i] + 0x33;
		}
	}

	pkt[pktLen-2] = Dlt645_GetChecksum(pkt+4, pktLen-6); //get the checksum excluding the leading bytes and end byte
	pkt[pktLen-1] = 0x16;
//	am_kprintf("datelen=%d,pktlen=%d\n",datalen,pktLen);
//	for(int i=0;i<pktLen;i++)
//	{
//		am_kprintf("%x ",pkt[i]);
//	}
//	am_kprintf("\n");
	return pkt;
}

int Dlt645_SendPkt(uint8_t *pkt, int len)
{
	return am_uart_poll_send(uart_handle, pkt, len);
}

int Dlt645_GetReply(uint8_t *pkt,int len)
{
	return  am_uart_rngbuf_receive(handle, pkt, len);
}

void Dlt645_Flush(void)
{
	am_uart_rngbuf_ioctl(handle,AM_UART_RNGBUF_WFLUSH,NULL);
	am_uart_rngbuf_ioctl(handle,AM_UART_RNGBUF_RFLUSH,NULL);
}


int Dlt645_DecodePkt(int type,uint8_t *Pkt, int pktLen, double *reading,uint32_t match_data)
{
	int ret;
	uint8_t *recvPkt = Pkt;
	char recvinfo[256]={0};
	/*delete all leading bytes*/
	int check_len;
	while(*recvPkt != DLT645_START_BYTE && pktLen > 0)
	{
		++recvPkt;
		--pktLen;
	}

	if(recvPkt>=Pkt+pktLen || *recvPkt != DLT645_START_BYTE)
	{
		am_kprintf("DLT645 Decode: receive all FE packet or the start byte is not 68!");
		return -1;
	}
	check_len=DLT645_FIXED_LEN+recvPkt[9];
	ret = Dlt645_GetChecksum(recvPkt, check_len) != recvPkt[check_len];	//excluding cs and end byte
	if(ret)
	{
		am_kprintf("DLT645 Decode: Checksum mismatch!");
		return -2;
	}
	/*check if the receive pkt and the send pkt types match*/
//	printf("send control code = %x,receive control code = %x\n",type&DLT645_FUNC_CODE_MASK,recvPkt[8]&DLT645_FUNC_CODE_MASK);
	if((type&DLT645_FUNC_CODE_MASK) != (recvPkt[8]&DLT645_FUNC_CODE_MASK))
	{
		am_kprintf("DLT645 Decode: Send and receive package types mismatch!");
		return -3;
	}
	if((check_len-DLT645_FIXED_LEN)!=recvPkt[9])
	{
		am_kprintf("DLT645 Decode: receive data len  mismatch!");
		return -3;
	}

	for(int i=0; i<recvPkt[9]; ++i)		// data area should subtract 0x33 to get the real values
	{
		recvPkt[DLT645_FIXED_LEN+i] -= 0x33;
//		am_kprintf("change buf=%x\n",recvPkt[DLT645_FIXED_LEN+i]);
	}

	uint8_t Type_Match[4];
	memcpy(Type_Match,&match_data,sizeof(match_data));
	switch(type)
	{
	case DLT645_PKT_TYPE_READ_ADDRESS:
		if(recvPkt[9] != DLT645_ADDRESS_LEN)
		{
			am_kprintf("DLT645 Decode: receive read address data len mismatch!");
			return -8;
		}
		if(strncmp((char *)&recvPkt[1],(char *)&recvPkt[10],6)!=0)
		{
			am_kprintf("DLT645 receive read address data  mismatch!");
			return -8;
		}

		memcpy(MeterAddress, &recvPkt[10], DLT645_ADDRESS_LEN);
		IsMeterAddress = true;
		am_kprintf("get meter address.\n");
		break;

	case DLT645_PKT_TYPE_READ_DATA:
		if((strncmp((char *)MeterAddress,(char *)&recvPkt[1],6))!=0)
		{
			am_kprintf("DLT645 Decode: receive reply reading meter address mismatch!");
			return -8;
		}
		if((recvPkt[8]&DLT645_COMM_TYPE_MASK) == DLT645_SLAVE_REPLY_ERROR)
		{
			am_kprintf("DLT645 Decode: receive reply reading control code is D1!");
			return DLT645_FAILED;
		}
		if((strncmp((char *)Type_Match,(char *)&recvPkt[10],4))!=0)
		{
			am_kprintf("DLT645 Decode: receive reply reading data identification mismatch!");
			return -8;
		}
		if(!((recvPkt[8]&DLT645_COMM_TYPE_MASK) == DLT645_SLAVE_REPLY_NORMAL))
		{
			am_kprintf("DLT645 Decode: receive reply reading control code is not read function!");
			return -8;
		}
		if(recvPkt[9]<DLT645_MIN_DATA_LEN || recvPkt[9]>DLT645_MAX_DATA_LEN )
		{
			am_kprintf("DLT645 Decode: receive reply reading data length mismatch!");
			return -9;
		}
		*reading = Dlt645_TanslateReadingResult(&recvPkt[DLT645_FIXED_LEN+DLT645_DATA_TAG_LEN],recvPkt[9]);	//4 bytes reading follows 4 bytes data tag in the data area
		break;

	default:
		am_kprintf("DLT645 Decode: control code type unknown!");
		break;
	}

	return DLT645_SUCCESS;
}

int Dlt645_QueryMeterReading(uint32_t dataTag, double *reading)
{
	int sendLen = 0;
	int expectRecvLen = DLT645_HEAD_TAIL_LEN + DLT645_DATA_TAG_LEN + DLT645_POWER_READING_LEN+DLT645_EXTRA_LEN;
	int ret;
	int func_code;
	uint8_t *sendPkt;
	uint8_t *recvPkt = (uint8_t *)malloc(expectRecvLen);	//allocate extra bytes for
	if(recvPkt==NULL)
	{
		am_kprintf("malloc failed\n");
		return -1;
	}

	if(IsMeterAddress)
		func_code=DLT645_PKT_TYPE_READ_DATA;
	else
		func_code=DLT645_PKT_TYPE_READ_ADDRESS;

	int s_len = 0;
	sendPkt = Dlt645_CreateSendPkt(func_code, dataTag, &sendLen); //format of message
	ret = Dlt645_SendPkt(sendPkt, sendLen);
	am_kprintf("send ret=%d\n",ret);
	ret = Dlt645_GetReply(recvPkt, expectRecvLen);
//	am_mdelay(500);
	am_kprintf("get reply=%d\n",ret);
	if(ret <= 0)
	{
		am_kprintf("DLT645 Receive meter reading reply failed!,ret=%d\n",ret);
		DLT645_ClearBuff(sendPkt, recvPkt);
		return -3;
	}
	for(int i=0;i<ret;i++)
	{
		am_kprintf("%x ",recvPkt[i]);
	}
	printf("\n");
	if((ret = Dlt645_DecodePkt(func_code, recvPkt, ret, reading,dataTag))<0) //decode message
	{
		DLT645_ClearBuff(sendPkt, recvPkt);
		am_kprintf("DLT645 Decode meter reading message failed!\n");
		return -4;
	}
	DLT645_ClearBuff(sendPkt, recvPkt); //free
	if(ret!=DLT645_SUCCESS)
			return ret;
	return DLT645_SUCCESS;
}

int Dlt645_TanslateReadingResult(uint8_t result[],int data_len)
{
	int reading = 0;
	float coef = 1;
	int datalen=data_len-DLT645_DATA_TAG_LEN;

	/*result is in BCD format XXXXXX.XX, little endian*/
	for(int i=0; i<datalen; ++i)
	{
		reading += (result[i]&0x0f)*coef;
		reading += (result[i]>>4)*10*coef;
		coef *= 100;
	}
	am_kprintf("reading=%d\n",reading);

	return reading;
}


void DLT645_ClearBuff(uint8_t* sendPkt, uint8_t* recvPkt)
{
	free(recvPkt);
//	recvPkt = NULL;
	free(sendPkt);
//	sendPkt = NULL;

}
void dlt645_test()
{
	int sendLen = 0;
	int expectRecvLen = DLT645_HEAD_TAIL_LEN + DLT645_DATA_TAG_LEN + DLT645_POWER_READING_LEN+DLT645_EXTRA_LEN;
	int ret;
	int func_code;

	DLT645_Init();

	while(1)
	{
		uint8_t *sendPkt;
		uint8_t *recvPkt = (uint8_t *)malloc(expectRecvLen);
		sendPkt = Dlt645_CreateSendPkt(0x13, 0, &sendLen);
		ret = Dlt645_SendPkt(sendPkt, sendLen);
		am_kprintf("send ret=%d\n",ret);
		ret = Dlt645_GetReply(recvPkt, expectRecvLen);
		am_kprintf("get reply ret=%d\n",ret);
		for(int i=0;i<ret;i++)
		{
			am_kprintf("%x ",recvPkt[i]);
		}
		printf("\n");
		am_mdelay(200);
		DLT645_ClearBuff(sendPkt, recvPkt);
	}
}

