/*=============================================================================
2	  Project: 
3	  Platform: GD32F230K8
4	  Filename: drv_FreqMet.c
5	  Description:
6	  Version: 0.0
7	  Created: 2023.09.19
8   Last modified: 2023.09.19
9	=============================================================================*/
#include "gd32e23x.h"
//#include <string.h> 
//-----------------------------------------------------------------------------
#include "drv_time.h"
#include "drv_FreqMet.h"
//--------------------------------------------------------------------------//
//#define _ADC0Vref (3.3)
//--------------------------------------------------------------------------//
static volatile uint32_t Counter;
static float Freq;
//--------------------------------------------------------------------------//
void InitFreqMet (void)
{
	timer_oc_parameter_struct timer_ocintpara;
	timer_parameter_struct timer_initpara;

	rcu_periph_clock_enable (RCU_TIMER0);
	rcu_periph_clock_enable (RCU_GPIOA);
	
	gpio_mode_set (GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2);
	gpio_output_options_set (GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
	gpio_bit_reset (GPIOA, GPIO_PIN_2);
	
	
	
	gpio_mode_set (GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);
	gpio_af_set (GPIOA, GPIO_AF_2, GPIO_PIN_12);

	nvic_irq_enable (TIMER0_BRK_UP_TRG_COM_IRQn, 3);
	
	timer_deinit (TIMER0);
	/* TIMER0 configuration */
	timer_struct_para_init (&timer_initpara);
	timer_initpara.prescaler         = 0;
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = 0xffff;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init (TIMER0, &timer_initpara);
	timer_counter_up_direction (TIMER0);
	timer_master_slave_mode_config (TIMER0, TIMER_MASTER_SLAVE_MODE_DISABLE);
	timer_update_source_config (TIMER0, TIMER_UPDATE_SRC_REGULAR);
	timer_input_trigger_source_select (TIMER0, TIMER_SMCFG_TRGSEL_ETIFP);
	timer_external_clock_mode1_config (TIMER0, TIMER_EXT_TRI_PSC_OFF, TIMER_ETP_RISING, 3);
	timer_output_value_selection_config (TIMER0, TIMER_OUTSEL_DISABLE);
	
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_UP);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_CH0);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_CH1);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_CH2);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_CH3);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_CMT);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_TRG);
	timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_BRK);

	timer_interrupt_enable (TIMER0, TIMER_INT_UP);

	/* enable TIMER0 */
	timer_enable (TIMER0);
}
//--------------------------------------------------------------------------//
void TIMER0_BRK_UP_TRG_COM_IRQHandler (void)
{
	if (timer_interrupt_flag_get (TIMER0, TIMER_INT_FLAG_UP))
	{
		timer_interrupt_flag_clear (TIMER0, TIMER_INT_FLAG_UP);
		Counter++;
		gpio_bit_toggle (GPIOA, GPIO_PIN_2);
	}
}
//--------------------------------------------------------------------------//
void RoutineFreqMet (void)
{
  static TTime T = 0;
  static uint64_t PrevCounter = 0;
  TTime T1, T2;
  uint32_t c1, c2, c3;
  uint64_t i = 0, j;

  if (EndTime (T))
  {
    do
    {
      T1 = GetTime ();
      c1 = timer_counter_read (TIMER0);
      c2 = Counter;
      c3 = timer_counter_read (TIMER0);
      T2 = GetTime ();
    } while ((c3 > c1) || (DiffTime (T1, T2) > 5.0e-6));

    i = c2;
    i <<= 16;
    i += c1;
		
    j = PrevCounter;
    PrevCounter = i;

    if (i >= j) i -= j;
    else
    {
      j -= i;
      i = 0xFFFFFFFFFFFFFFFFLL - j;
    }

    Freq = (float)i;
    Freq /= DiffTime (T, T1) + 1.0;

    T =  SetRelativTime_ms (T1, 1000);
  }
}
//--------------------------------------------------------------------------//
float ResultFreqMet (void)
{
	return (Freq);
}
//--------------------------------------------------------------------------//

