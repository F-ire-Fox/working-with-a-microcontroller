/*=============================================================================
2	  Project: SmartBox PeriferialUnit
3	  Platform: GD32F130K8
4	  Filename: drv_time.c
5	  Description:
6	  Version: 0.0
7	  Created: 2022.08.20
8   Last modified: 2022.08.20
9	=============================================================================*/
#include <stdint.h>
#include "gd32e23x.h"
//------------------------------------------------------------------------------
//#include "Global.h"
#include "drv_time.h"
//------------------------------------------------------------------------------
static volatile uint32_t TimeCounter;
//sbit Test = P2^6;
//------------------------------------------------------------------------------
void TimeCounter_Handler(void);
//------------------------------------------------------------------------------
void InitTime (void)
{
	TimeCounter = 0;
	
//  SystemCoreClockUpdate ();                      /* Get Core Clock Frequency   */
  SysTick_Config (0x00ffffff);
}
//------------------------------------------------------------------------------
void SysTick_Handler (void) {TimeCounter++;}
//------------------------------------------------------------------------------
TTime GetTime (void)
{
	TTime t;
	uint32_t ui, tt;
	do {
		tt = TimeCounter;
		ui = SysTick->VAL;
	} while (tt != TimeCounter);
	t = tt;
	t <<= 24;
	t |= 0x00ffffff - ui;
	return (t);
}
//------------------------------------------------------------------------------
uint32_t GetTimeSec (void)
{
	TTime t;
	uint64_t ui;

	t = GetTime ();

	ui = t / SystemCoreClock;
	
	return ((uint32_t)ui);
}
//------------------------------------------------------------------------------
float DiffTime (TTime time1, TTime time2)
{
  float f;
  uint64_t t;
  t = time2;
  t -= time1;
  f = (float)(t);
	f = f / (float)SystemCoreClock;
  return (f);
}
//------------------------------------------------------------------------------
float fGetTime (void)
{
	TTime t = GetTime ();
	float f = (float)t;
	f = f / (float)SystemCoreClock;
	return (f);
}
//------------------------------------------------------------------------------
TTime SetTime_ms (uint32_t Time)
{
	TTime t = GetTime ();
	uint64_t i = SystemCoreClock;
	i *= Time;
	i /= 1000;
	t += i;
	return (t);
}
//------------------------------------------------------------------------------
TTime SetTime_us (uint32_t Time)
{
	TTime t = GetTime ();
	uint64_t i = SystemCoreClock;
	i *= Time;
	i /= 1000;
	i /= 1000;
	t += i;
	return (t);
}
//------------------------------------------------------------------------------
TTime SetRelativTime_ms (TTime T, uint32_t Time)
{
	uint64_t i = SystemCoreClock;
	i *= Time;
	i /= 1000;
  return (T + i);
}
//------------------------------------------------------------------------------
int32_t EndTime (TTime Time)
{
	TTime t = GetTime();
	if (t > Time)
		return (1);
	else
		return (0);
}
//------------------------------------------------------------------------------
/*
TTime ClrTime (void)
{
	idata TTime t;
	t = GetTime();
	return (t);
}
//------------------------------------------------------------------------------
*/
void Delay_ms (uint32_t ms)
{
	TTime t = SetTime_ms (ms);
	while (!EndTime (t));
}
//------------------------------------------------------------------------------
void Delay_us (uint32_t us)
{
	TTime t = SetTime_us (us);
	while (!EndTime (t));
}
//------------------------------------------------------------------------------
