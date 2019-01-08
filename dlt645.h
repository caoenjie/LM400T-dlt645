/*
 * dlt645.h
 *
 *  Created on: 2019年1月4日
 *      Author: Black
 */

#ifndef USER_CODE_DLT645_H_
#define USER_CODE_DLT645_H_
#include <stdint.h>
#include "typedef.h"
/*****data tags table, DI0 DI1 DI2 DI3*******/
#define DLT645_TAG_FORWARD_ACTIVE_POWER    0x00010000 //表读数-总（正向有功）
#define DLT645_TAG_BACKWARD_ACTIVE_POWER   0x00020000 //表读数-总（反向有功）
#define DLT645_TAG_INSTANT_ACTIVE_POWER    0x02030000 //瞬时总有功

#define DLT645_TGA_GRID_PHASE_VOLTAGE_A     0x02010100
#define DLT645_TGA_GRID_PHASE_VOLTAGE_B     0x02010200  //电网相电压[Max. 3路]
#define DLT645_TGA_GRID_PHASE_VOLTAGE_C     0x02010300

#define DLT645_TGA_GRID_PHASE_CURRENT_A     0x02020100
#define DLT645_TGA_GRID_PHASE_CURRENT_B     0x02020200  //电网相电流[Max. 3路]
#define DLT645_TGA_GRID_PHASE_CURRENT_C     0x02020300

#define DLT645_TGA_GRID_PHASE_POWER_A       0x02030100
#define DLT645_TGA_GRID_PHASE_POWER_B       0x02030200  //电网相功率[Max. 3路]
#define DLT645_TGA_GRID_PHASE_POWER_C       0x02030300

//表读数-电价1/2/3/4（正向有功）
#define DLT645_TGA_FORWARD_ACTIVE_POWER_1   0x00010100
#define DLT645_TGA_FORWARD_ACTIVE_POWER_2   0x00010200
#define DLT645_TGA_FORWARD_ACTIVE_POWER_3   0x00010300
#define DLT645_TGA_FORWARD_ACTIVE_POWER_4   0x00010400

//表读数-电价1/2/3/4（反向有功）
#define DLT645_TGA_BACKWARD_ACTIVE_POWER_1   0x00020100
#define DLT645_TGA_BACKWARD_ACTIVE_POWER_2   0x00020200
#define DLT645_TGA_BACKWARD_ACTIVE_POWER_3   0x00020300
#define DLT645_TGA_BACKWARD_ACTIVE_POWER_4   0x00020400

#define DLT645_ADDRESS_LEN			6       //meter address



/*****comm type*******/
#define DLT645_COMM_TYPE_MASK		0xE0
#define DLT645_MASTER_QUERY			0x00
#define DLT645_SLAVE_REPLY_NORMAL	0x80
#define DLT645_SLAVE_REPLY_ERROR	0xC0

/*****comm bytes*******/
#define DLT645_LEADING_BYTE			0xFE
#define DLT645_START_BYTE			0x68

/*****function code*******/
#define DLT645_FUNC_CODE_MASK				0x1F
#define DLT645_PKT_TYPE_TIME_SYNC			0x08
#define DLT645_PKT_TYPE_READ_DATA			0x11
#define DLT645_PKT_TYPE_READ_DATA_LEFT		0x12
#define DLT645_PKT_TYPE_WRITE_DATA			0x14
#define DLT645_PKT_TYPE_READ_ADDRESS		0x13
#define DLT645_PKT_TYPE_WRITE_ADDRESS		0x15
#define DLT645_PKT_TYPE_CHANGE_COMM_SPEED	0x17

/*****comm type*******/
#define DLT645_COMM_TYPE_MASK		0xE0
#define DLT645_MASTER_QUERY			0x00
#define DLT645_SLAVE_REPLY_NORMAL	0x80
#define DLT645_SLAVE_REPLY_ERROR	0xC0

/*****package length*******/
#define DLT645_HEAD_TAIL_LEN		16		// 4 leading bytes, 2 start bytes, 6 address, 1 func code, 1 data len, 1 cs, 1 end byte
#define DLT645_PRE_LEADING_LEN		4       //4 0xEF
#define DLT645_DATA_TAG_LEN			4       //data identification
#define DLT645_ADDRESS_LEN			6       //meter address
#define DLT645_MAX_DATA_LEN			12      //max data 4+8
#define DLT645_MIN_DATA_LEN			6       //min data 4+2
#define DLT645_POWER_READING_LEN	4       //power data len
#define DLT645_FIXED_LEN            10     //2leading bytes，6 meter address，1func code，1 data len
#define DLT645_gPHASE_VC_LEN 		2		//E V data len
#define DLT645_gPHASE_P_LEN 		3		//three-phase power len
#define DLT645_SEND_LEN				20      //
#define DLT645_RECV_LEN				30
#define	DLT645_ERROR_LEN			1		//error message
#define DLT645_EXTRA_LEN			4      //read message extra length

static int IsMeterAddress   = false;

/*
 * \brief Initialize the meter ID ,if the meter ID don't know
 */
void DLT645_InitMeterAddress(void);

uint8_t* Dlt645_CreateSendPkt(int type, uint32_t dataTag, int *Ptklen);

int Dlt645_SendPkt(uint8_t *pkt, int len);

int Dlt645_GetReply(uint8_t *pkt,int len);

int Dlt645_DecodePkt(int type,uint8_t *Pkt, int pktLen, double *reading,uint32_t match_data);

int Dlt645_QueryMeterReading(uint32_t dataTag, double *reading);

int Dlt645_TanslateReadingResult(uint8_t result[],int data_len);

uint8_t Dlt645_GetChecksum(uint8_t data[], int len);

int Dlt645_GetMeterID(char *meterid);
void DLT645_UartInit();
void Dlt645_DestroySendPkt(uint8_t *sendpkt,uint8_t *recvPkt);
void DLT645_Init(void);
void DLT645_ClearBuff(uint8_t* sendPkt, uint8_t* recvPkt);
int DLT645_IsMeterAddress(void);
//int strncmp1(const uint8_t *s1,const uint8_t *s2,int n);












#endif /* USER_CODE_DLT645_H_ */
