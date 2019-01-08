/*
 * meter.c
 *
 *  Created on: 2019年1月3日
 *      Author: Black
 */

#include "am_delay.h"
#include "dlt645.h"
#include "meter.h"
#include "am_vdebug.h"



static int All_Meter_Data[20]={
		DLT645_TAG_FORWARD_ACTIVE_POWER,
		DLT645_TAG_BACKWARD_ACTIVE_POWER,
		DLT645_TAG_INSTANT_ACTIVE_POWER,
		DLT645_TGA_GRID_PHASE_VOLTAGE_A,
		DLT645_TGA_GRID_PHASE_VOLTAGE_B,
		DLT645_TGA_GRID_PHASE_VOLTAGE_C,
		DLT645_TGA_GRID_PHASE_CURRENT_A,
		DLT645_TGA_GRID_PHASE_CURRENT_B,
		DLT645_TGA_GRID_PHASE_CURRENT_C,
		DLT645_TGA_GRID_PHASE_POWER_A,
		DLT645_TGA_GRID_PHASE_POWER_B,
		DLT645_TGA_GRID_PHASE_POWER_C,
		DLT645_TGA_FORWARD_ACTIVE_POWER_1,
		DLT645_TGA_FORWARD_ACTIVE_POWER_2,
		DLT645_TGA_FORWARD_ACTIVE_POWER_3,
		DLT645_TGA_FORWARD_ACTIVE_POWER_4,
		DLT645_TGA_BACKWARD_ACTIVE_POWER_1,
		DLT645_TGA_BACKWARD_ACTIVE_POWER_2,
		DLT645_TGA_BACKWARD_ACTIVE_POWER_3,
		DLT645_TGA_BACKWARD_ACTIVE_POWER_4
};

#define UART1_RX_BUF_SIZE  128  /**< \brief 接收环形缓冲区大小，应该为2^n  */
#define UART1_TX_BUF_SIZE  128  /**< \brief 发送环形缓冲区大小，应该为2^n  */

void Meter_Init()
{
	DLT645_Init();
}

void MeterAddress_Get()
{
	double reading=0;
	int ret;
	for(int i=0;i<3;i++)
	{
		ret=Dlt645_QueryMeterReading(0, &reading);
		if(ret >= 0)
		{
			break;
		}
	}
//	if(ret < 0)
//		IsMeterAddress = false;
}

double MeterReading_Get(uint32_t meterdata)
{
	double reading;
	int ret;
	int read_count=0;
	for(int i=0;i<3;i++)
	{
		ret = Dlt645_QueryMeterReading(meterdata, &reading); //read meter data
		if(ret>=0)
			break;
//		am_mdelay(5000);
	}

	return reading;
}

uint8_t * MeterData_Get()
{
	int i=0;
	double reading;
	uint8_t* pktData;
	Meter_Init();
	if(!DLT645_IsMeterAddress())
	{
//		DLT645_InitMeterAddress();
		MeterAddress_Get();
	}
	if(!DLT645_IsMeterAddress())
	{
		am_kprintf("Get meter address failed\n");
		return NULL;
	}

	while(i<20)
	{
		am_kprintf("i = %d\n",i);
		reading=MeterReading_Get(All_Meter_Data[i]);
		i++;

	}

	return pktData;
}
meter_test()
{
	uint32_t reading;
	while(1)
	{
		MeterData_Get();
		am_mdelay(1000);
	}
}




