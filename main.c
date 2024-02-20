
#include <stdio.h>
#include "gd32e23x.h"

#include "drv_time.h"
#include "drv_ADC.h"
#include "drv_uart.h"
#include "UARTRoutine.h"
#include "drv_FreqMet.h"

int main (void)
{
	
	InitTime ();
	InitADC ();
	InitFreqMet ();
//	UartInit (115200);
	UartInit (9600);
	
	while (1)
	{
		RoutineADC ();
		UARTRoutine ();
		RoutineFreqMet ();
	}
}
