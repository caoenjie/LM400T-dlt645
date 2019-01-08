/*
 * meter.h
 *
 *  Created on: 2019Äê1ÔÂ3ÈÕ
 *      Author: Black
 */

#ifndef USER_CODE_METER_H_
#define USER_CODE_METER_H_

#include <stdint.h>


void MeterAddress_Get(void);
double MeterReading_Get(uint32_t meterdata);
uint8_t * MeterData_Get();
void Meter_Init();
#endif /* USER_CODE_METER_H_ */
