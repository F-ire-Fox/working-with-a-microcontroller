/*=============================================================================
2	  Project: SmartBox PeriferialUnit
3	  Platform: GD32F230K8
4	  Filename: drv_ADC.h
5	  Description:
6	  Version: 0.0
7	  Created: 2022.08.23
8   Last modified: 2023.09.16
9	=============================================================================*/
#ifndef drv_ADC_H
#define drv_ADC_H

#include <stdbool.h>
//	#include <stdint.h>
	typedef enum 
	{
		_Current_In 			= 0,
		_Voltage_In 			= 1,
		_Voltage_RefInt 	= 2,
		_TempSensor				= 3,
		_Power_In 				= 4,
		_ADCNumberChannel = 5
	} TADCChannel;
	//------------------------------------------------------------------------//
	void InitADC (void);
	void RoutineADC (void);
	bool GetADCDataReady (void);
	void DMA_Channel0_IRQHandler (void);
	
	float ResultDataADCAVG (TADCChannel Chan);
	float ResultDataADCMax (TADCChannel Chan);
	float ResultDataADCMin (TADCChannel Chan);
	//------------------------------------------------------------------------//

#endif 
