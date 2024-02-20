/*=============================================================================
2	  Project: 
3	  Platform: GD32F230K8
4	  Filename: drv_FreqMet.h
5	  Description:
6	  Version: 0.0
7	  Created: 2023.09.19
8   Last modified: 2023.09.19
9	=============================================================================*/
#ifndef drv_FreqMet_H
#define drv_FreqMet_H

//#include <stdbool.h>
//	#include <stdint.h>
	//------------------------------------------------------------------------//
	void InitFreqMet (void);
	void RoutineFreqMet (void);
	
	float ResultFreqMet (void);
	//------------------------------------------------------------------------//
	void TIMER0_BRK_UP_TRG_COM_IRQHandler (void);
	void TIMER0_Channel_IRQHandler (void);

	//------------------------------------------------------------------------//

#endif 
