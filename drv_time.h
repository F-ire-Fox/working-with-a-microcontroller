/*=============================================================================
2	  Project: SmartBox PeriferialUnit
3	  Platform: GD32F130K8
4	  Filename: drv_time.h
5	  Description:
6	  Version: 0.0
7	  Created: 2022.08.20
8   Last modified: 2022.08.20
9	=============================================================================*/
#ifndef drv_time_H
#define drv_time_H

#include <stdint.h>
typedef uint64_t TTime;
void SysTick_Handler (void);
void InitTime (void);
TTime GetTime (void);
uint32_t GetTimeSec (void);
float DiffTime (TTime time1, TTime time2);
float fGetTime (void);
TTime SetTime_ms (uint32_t Time);
TTime SetTime_us (uint32_t Time);
TTime SetRelativTime_ms (TTime T, uint32_t Time);
int32_t EndTime (TTime Time);
//TTime ClrTime (void);
void Delay_ms (uint32_t ms);
void Delay_us (uint32_t us);

#endif 

