/*=============================================================================
2	  Project: SmartBox PeriferialUnit
3	  Platform: GD32F130K8
4	  Filename: drv_uart.h
5	  Description:
6	  Version: 0.0
7	  Created: 2022.08.22
8   Last modified: 2022.08.22
9	=============================================================================*/
#ifndef drv_uart_H
#define drv_uart_H

	#include <stdint.h>
	//------------------------------------------------------------------------//
	void UartInit (uint32_t Speed);
	void Start_trsUART (void);

	void USART0_IRQHandler (void);

	//------------------------------------------------------------------------//
	#undef _DebugFIFO

	void PushTxFIFO (unsigned char b);
	unsigned char PopRxFIFO (void);

	uint32_t GetNumberOfByteRxFIFO (void);
	uint32_t GetNumberOfFreeByteTxFIFO (void);

	#ifdef _DebugFIFO
		void ErrorOverflowTxFIFO (void);
		void ErrorUnderflowTxFIFO (void);
		void ErrorOverflowRxFIFO (void);
		void ErrorUnderflowRxFIFO (void);
	#endif

#endif 
