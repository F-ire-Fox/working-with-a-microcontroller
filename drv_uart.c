/*=============================================================================
2	  Project: SmartBox PeriferialUnit
3	  Platform: GD32F130K8
4	  Filename: drv_uart.c
5	  Description:
6	  Version: 0.0
7	  Created: 2022.08.22
8   Last modified: 2022.08.22
9	=============================================================================*/
#include "gd32e23x.h"
//-----------------------------------------------------------------------------
#include "drv_uart.h"
//--------------------------------------------------------------------------//
#define _SizeFIFO 256
#define _InedexFIFOMask 0xFF
//--------------------------------------------------------------------------//
static volatile uint8_t TxFIFO [_SizeFIFO] __attribute__((aligned(_SizeFIFO)));
static volatile uint8_t RxFIFO [_SizeFIFO] __attribute__((aligned(_SizeFIFO)));
//--------------------------------------------------------------------------//
static volatile uint32_t NumberOfBytesTxFIFO = 0;
static volatile uint32_t PushIndexToTxFIFO = 0;
static volatile uint32_t PopIndexToTxFIFO = 0;
//--------------------------------------------------------------------------//
static volatile uint32_t NumberOfBytesRxFIFO = 0;
static volatile uint32_t PushIndexToRxFIFO = 0;
static volatile uint32_t PopIndexToRxFIFO = 0;
//-----------------------------------------------------------------------------------------------//
void UartInit (uint32_t Speed)
{

	NumberOfBytesTxFIFO = 0;
	PushIndexToTxFIFO = 0;
	PopIndexToTxFIFO = 0;

	NumberOfBytesRxFIFO = 0;
	PushIndexToRxFIFO = 0;
	PopIndexToRxFIFO = 0;

	nvic_irq_enable (USART0_IRQn, 0);
	rcu_periph_clock_enable (RCU_GPIOA);
	rcu_periph_clock_enable (RCU_USART0);
	/* connect port to USART0_Tx */
	gpio_af_set (GPIOA, GPIO_AF_1, GPIO_PIN_9);
	/* connect port to USART0_Rx */
	gpio_af_set (GPIOA, GPIO_AF_1, GPIO_PIN_10);
	/* configure USART Tx as alternate function push-pull */
	gpio_mode_set (GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
	gpio_output_options_set (GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
	/* configure USART Rx as alternate function push-pull */
	gpio_mode_set (GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
	gpio_output_options_set (GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
	/* configure USART RTS as alternate function push-cown */
	/* USART configure */
	usart_deinit (USART0);
	usart_baudrate_set (USART0, Speed);
//	usart_rs485_driver_enable (USART0);
	usart_depolarity_config (USART0, 0);
	usart_driver_assertime_config (USART0, 0x1f);
	usart_driver_deassertime_config (USART0, 0x1f);
	
//	usart_invert_config (USART0, USART_SWAP_ENABLE);
	
	usart_transmit_config (USART0, USART_TRANSMIT_ENABLE);
	usart_receive_config (USART0, USART_RECEIVE_ENABLE);
	usart_enable (USART0);
	usart_interrupt_enable (USART0, USART_INT_RBNE);
}
//--------------------------------------------------------------------------//
void USART0_IRQHandler (void)
{
	if(RESET != usart_interrupt_flag_get (USART0, USART_INT_FLAG_RBNE))
	{/* receive data */
		uint8_t rb = (uint8_t)usart_data_receive (USART0);
		if ((_SizeFIFO - 1) > NumberOfBytesRxFIFO)
		{
			RxFIFO [PushIndexToRxFIFO] = rb;
			NumberOfBytesRxFIFO++;
			PushIndexToRxFIFO++;
			PushIndexToRxFIFO &= _InedexFIFOMask;
		}
#ifdef _DebugFIFO
		else
			ErrorOverflowRxFIFO ();
#endif
	}
	if(RESET != usart_interrupt_flag_get (USART0, USART_INT_FLAG_TBE))
	{ /* transmit data */
		if (NumberOfBytesTxFIFO > 0)
		{
				usart_data_transmit (USART0, TxFIFO [PopIndexToTxFIFO]);
				NumberOfBytesTxFIFO--;
				PopIndexToTxFIFO++;
				PopIndexToTxFIFO &= _InedexFIFOMask;
		}
		else
			usart_interrupt_disable (USART0, USART_INT_TBE);
	}
	
		if (RESET != usart_interrupt_flag_get (USART0, USART_INT_FLAG_RBNE_ORERR))
			usart_interrupt_flag_clear (USART0, USART_INT_FLAG_RBNE_ORERR);
		
/*		
      \arg        USART_INT_FLAG_EB:   end of block interrupt and flag
      \arg        USART_INT_FLAG_RT:   receiver timeout interrupt and flag
      \arg        USART_INT_FLAG_AM:   address match interrupt and flag 
      \arg        USART_INT_FLAG_PERR: parity error interrupt and flag 
      \arg        :  transmitter buffer empty interrupt and flag 
      \arg        USART_INT_FLAG_TC:   transmission complete interrupt and flag
      \arg        : read data buffer not empty interrupt and flag
      \arg        USART_INT_FLAG_RBNE_ORERR: read data buffer not empty interrupt and overrun error flag
      \arg        USART_INT_FLAG_IDLE: IDLE line detected interrupt and flag
      \arg        USART_INT_FLAG_LBD:  LIN break detected interrupt and flag 
      \arg        USART_INT_FLAG_WU:   wakeup from deep-sleep mode interrupt and flag
      \arg        USART_INT_FLAG_CTS:  CTS interrupt and flag
      \arg        USART_INT_FLAG_ERR_NERR:  error interrupt and noise error flag
      \arg        USART_INT_FLAG_ERR_ORERR: error interrupt and overrun error
      \arg        USART_INT_FLAG_ERR_FERR:  error interrupt and frame error flag
*/	
}
//--------------------------------------------------------------------------//
void Start_trsUART (void)
{
	usart_interrupt_enable (USART0, USART_INT_TBE);
}
//--------------------------------------------------------------------------//
void PushTxFIFO (unsigned char b)
{
	uint32_t t;
	if ((_SizeFIFO - 1) > NumberOfBytesTxFIFO)
	{
		TxFIFO [PushIndexToTxFIFO] = b;
		do {
			t = __LDREXW (&NumberOfBytesTxFIFO);
			t++;
		} while (__STREXW (t, &NumberOfBytesTxFIFO));
		PushIndexToTxFIFO++;
		PushIndexToTxFIFO &= _InedexFIFOMask;
		usart_interrupt_enable (USART0, USART_INT_TBE);
	}
#ifdef _DebugFIFO
	else
		ErrorOverflowTxFIFO ();
#endif
}
//--------------------------------------------------------------------------//
unsigned char PopRxFIFO (void)
{
	uint8_t b = 0;
	uint32_t t;
	if (NumberOfBytesRxFIFO > 0)
	{
		b = RxFIFO [PopIndexToRxFIFO];
		do {
			t = __LDREXW (&NumberOfBytesRxFIFO);
			t--;
		} while (__STREXW (t, &NumberOfBytesRxFIFO));
		PopIndexToRxFIFO++;
		PopIndexToRxFIFO &= _InedexFIFOMask;
	}
#ifdef _DebugFIFO
	else
		ErrorUnderflowRxFIFO ();
#endif
	return (b);
}
//--------------------------------------------------------------------------//
uint32_t GetNumberOfByteRxFIFO (void)
{
	return (NumberOfBytesRxFIFO);
}
//--------------------------------------------------------------------------//
uint32_t GetNumberOfFreeByteTxFIFO (void)
{
	return (_SizeFIFO - NumberOfBytesTxFIFO);
}
//--------------------------------------------------------------------------//

