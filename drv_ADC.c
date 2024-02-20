/*=============================================================================
2	  Project: 
3	  Platform: GD32E230K8
4	  Filename: drv_ADC.c
5	  Description:
6	  Version: 0.0
7	  Created: 2022.08.23
8   Last modified: 2023.09.16
9	=============================================================================*/
#include "gd32e23x.h"
#include <string.h> 
//-----------------------------------------------------------------------------
#include "drv_time.h"
#include "drv_ADC.h"
//--------------------------------------------------------------------------//
#define _ADC0NumMeans (1000)
#define _ADC0Vref (3.3)
//--------------------------------------------------------------------------//
static const float CalibADCCoeffMul [_ADCNumberChannel] = 
{
	2.0080f, //_Current_In = 0
	7.9800f, //_Voltage_In = 1
	1.0000f, //_Voltage_RefInt = 2
	1.0000f, //_TempSensor = 3
	(16.02384f * _ADC0Vref)  //_Power_In = 4
};
//--------------------------------------------------------------------------//
static float ADCResultAVG [_ADCNumberChannel];
static float ADCResultMax [_ADCNumberChannel];
static float ADCResultMin [_ADCNumberChannel];
static uint32_t SSumADC [_ADCNumberChannel];
static uint16_t SMaxADC [_ADCNumberChannel];
static uint16_t SMinADC [_ADCNumberChannel];
static uint32_t SumADC [_ADCNumberChannel];
static uint16_t MaxADC [_ADCNumberChannel];
static uint16_t MinADC [_ADCNumberChannel];
static uint32_t SCount;
static bool ADCReadyData;
static bool DataReady;
//--------------------------------------------------------------------------//
#define _ADCDMAChannSamplNum (32)
#define _ADCDMABuffSize ((_ADCNumberChannel - 1) * _ADCDMAChannSamplNum)
static uint16_t ADCDMABuff0 [_ADCDMABuffSize];
//--------------------------------------------------------------------------//
void InitADC (void)
{
	timer_oc_parameter_struct timer_ocintpara;
	timer_parameter_struct timer_initpara;
	dma_parameter_struct dma_init_parameter;
	int32_t i;
	
	for (i = _Current_In; i < _ADCNumberChannel; i++)
	{
		ADCResultAVG [i] = ADCResultMax [i] = ADCResultMin [i] = 0;
		SSumADC [i] = SumADC [i] = 0;
		SMaxADC [i] = MaxADC [i] = SMinADC [i] = 0;
		MinADC [i] = 0xffff;
	}
	ADCReadyData = false;
	DataReady = false;


	rcu_periph_clock_enable (RCU_DMA);
	rcu_periph_clock_enable (RCU_TIMER14);
	rcu_periph_clock_enable (RCU_GPIOA);
	rcu_periph_clock_enable (RCU_ADC);
	rcu_adc_clock_config (RCU_ADCCK_APB2_DIV8);

//	if (0)
//	{
	/* config the GPIO as analog mode */
	gpio_mode_set (GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_6);//_Current_In
	gpio_mode_set (GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_7);//_Voltage_In

	/* reset ADC */
	adc_deinit ();

	
	/* configure ADC resolution */
	adc_resolution_config (ADC_RESOLUTION_12B);
	
	/* configure ADC data alignment */
	adc_data_alignment_config (ADC_DATAALIGN_RIGHT);
	/* enable ADC external trigger */
	adc_external_trigger_config (ADC_REGULAR_CHANNEL, ENABLE);
	/* configure ADC external trigger source */
	adc_external_trigger_source_config (ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T14_CH0);
	
	/* enable or disable ADC special function */
	adc_special_function_config (ADC_SCAN_MODE, ENABLE);
	adc_special_function_config (ADC_CONTINUOUS_MODE, DISABLE);
	adc_special_function_config (ADC_INSERTED_CHANNEL_AUTO, DISABLE);

	/* enable the temperature sensor and Vrefint channel */
	adc_tempsensor_vrefint_enable ();

	/* configure ADC discontinuous mode */
	adc_discontinuous_mode_config (ADC_CHANNEL_DISCON_DISABLE, 0);
	/* configure the length of regular channel group or inserted channel group */
	adc_channel_length_config (ADC_REGULAR_CHANNEL, _ADCNumberChannel - 1);// - _Power_In
	/* configure ADC regular channel */
	adc_regular_channel_config ((uint8_t)_Current_In,		  ADC_CHANNEL_6,	ADC_SAMPLETIME_28POINT5);
	adc_regular_channel_config ((uint8_t)_Voltage_In,  		ADC_CHANNEL_7,	ADC_SAMPLETIME_28POINT5);
	adc_regular_channel_config ((uint8_t)_Voltage_RefInt,	ADC_CHANNEL_17,	ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config ((uint8_t)_TempSensor,  		ADC_CHANNEL_16,	ADC_SAMPLETIME_239POINT5);

	/* disable ADC oversample mode */
	adc_oversample_mode_disable ();

	/* enable DMA request */
	adc_dma_mode_enable ();
//if (0)
//{
	memset (ADCDMABuff0, 0, sizeof (ADCDMABuff0));
	
	dma_struct_para_init (&dma_init_parameter);
	/* deinitialize DMA1_CH0 */
	dma_deinit (DMA_CH0);
	nvic_irq_enable (DMA_Channel0_IRQn, 2);
	syscfg_dma_remap_disable (SYSCFG_DMA_REMAP_ADC);

	dma_init_parameter.periph_addr = (int32_t)&ADC_RDATA;					/*!< peripheral base address */
	dma_init_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;	/*!< transfer data size of peripheral */
	dma_init_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;	/*!< peripheral increasing mode */  
	dma_init_parameter.memory_addr = (uint32_t)ADCDMABuff0;				/*!< memory 0 base address */
	dma_init_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;			/*!< transfer data size of memory */
	dma_init_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;		/*!< memory increasing mode */
	dma_init_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;			/*!< channel data transfer direction */
	dma_init_parameter.number = _ADCDMABuffSize;									/*!< channel transfer number */
	dma_init_parameter.priority = DMA_PRIORITY_HIGH;							/*!< channel priority level */
	dma_init (DMA_CH0, &dma_init_parameter);

	dma_circulation_enable (DMA_CH0);

	dma_flag_clear (DMA_CH0, DMA_FLAG_G);
	dma_flag_clear (DMA_CH0, DMA_FLAG_ERR);
	dma_flag_clear (DMA_CH0, DMA_FLAG_HTF);
	dma_flag_clear (DMA_CH0, DMA_FLAG_FTF);

	dma_interrupt_enable (DMA_CH0, DMA_INT_FTF);
	dma_interrupt_enable (DMA_CH0, DMA_INT_HTF);

	/* enable DMA channel */
	dma_channel_enable (DMA_CH0);
//}

//	gpio_mode_set (GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2);
//	gpio_output_options_set (GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
//	gpio_bit_reset (GPIOA, GPIO_PIN_2);
//	gpio_bit_set (GPIOA, GPIO_PIN_2);
//	gpio_bit_reset (GPIOA, GPIO_PIN_2);


	timer_deinit (TIMER14);
	/* TIMER14 configuration */
	timer_struct_para_init (&timer_initpara);
	timer_initpara.prescaler         = 999;
//	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
//	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = 71;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init (TIMER14, &timer_initpara);

//	timer_update_source_config (TIMER14, TIMER_UPDATE_SRC_REGULAR);
//	timer_master_slave_mode_config (TIMER14, TIMER_MASTER_SLAVE_MODE_DISABLE);
//	timer_internal_clock_config (TIMER14);
	/* CH1 configuration in PWM mode1 */
	timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
	timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
	timer_channel_output_config (TIMER14, TIMER_CH_0, &timer_ocintpara);
	timer_channel_output_pulse_value_config (TIMER14, TIMER_CH_0, 1);
	timer_channel_output_mode_config (TIMER14, TIMER_CH_0, TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER14, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);	
	timer_auto_reload_shadow_enable (TIMER14);

//	gpio_af_set (GPIOA, GPIO_AF_0, GPIO_PIN_2);
//	gpio_mode_set (GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
//	gpio_output_options_set (GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
	timer_primary_output_config (TIMER14, ENABLE);
	
	/* enable TIMER1 */
	timer_enable (TIMER14);
	
	/* enable ADC interface */
	adc_enable ();
	
	Delay_ms (1);
	
/* ADC calibration and reset calibration */
//	adc_calibration_enable ();
}
//--------------------------------------------------------------------------//
//#define _ADCOffset (46) // ~ +37 mV
#define _ADCOffset (0)
//--------------------------------------------------------------------------//
static void DataProcessMon (uint16_t * p)
{
	static uint32_t Count = 0;
	int32_t N, S;
	uint16_t d;

	for (N = 0; N < _ADCNumberChannel; N++)
	{
		if (N == _Power_In)
		{
			for (S = 0; S < _ADCDMAChannSamplNum / 2; S++)
			{
				uint32_t t;
				t = p [S * (_ADCNumberChannel - 1) + _Current_In] - _ADCOffset;
				t *= p [S * (_ADCNumberChannel - 1) + _Voltage_In] - _ADCOffset;
				t += 0x000001ff;
				t = t >> 12; //12b * 12b = 24b >> 12 = 12b
				d = t;
				SumADC [N] += d;
				if (d > MaxADC [N]) MaxADC [N] = d;
				if (d < MinADC [N]) MinADC [N] = d;
			}
		}
		else
		{
			for (S = 0; S < _ADCDMAChannSamplNum / 2; S++)
			{
				d = p [S * (_ADCNumberChannel - 1) + N];
				d -= _ADCOffset;//Offset comp
				SumADC [N] += d;
				if (d > MaxADC [N]) MaxADC [N] = d;
				if (d < MinADC [N]) MinADC [N] = d;
			}
		}
	}
	Count += _ADCDMAChannSamplNum / 2;
	if (_ADC0NumMeans <= Count)
	{
		for (N = 0; N < _ADCNumberChannel; N++)
		{
			SSumADC [N] = SumADC [N];
			SMaxADC [N] = MaxADC [N];
			SMinADC [N] = MinADC [N];
			SumADC [N] = 0;
			MaxADC [N] = 0;
			MinADC [N] = 0xffff;
		}
		SCount = Count;
		Count = 0;
		ADCReadyData = true;
	}
}
//--------------------------------------------------------------------------//
void DMA_Channel0_IRQHandler (void)
{
//	gpio_bit_set (GPIOA, GPIO_PIN_2);

	if (SET == dma_interrupt_flag_get (DMA_CH0, DMA_INT_FLAG_HTF))
	{
		dma_interrupt_flag_clear (DMA_CH0, DMA_INT_FLAG_HTF);
		DataProcessMon (&(ADCDMABuff0 [0]));
	}
	if (SET == dma_interrupt_flag_get (DMA_CH0, DMA_INT_FLAG_FTF))
	{
		dma_interrupt_flag_clear (DMA_CH0, DMA_INT_FLAG_FTF);
		DataProcessMon (&(ADCDMABuff0 [_ADCDMABuffSize / 2]));
	}
	if (SET == dma_interrupt_flag_get (DMA_CH0, DMA_INT_FLAG_ERR))
	{
		dma_interrupt_flag_clear (DMA_CH0, DMA_INT_FLAG_ERR);
	}
	dma_interrupt_flag_clear (DMA_CH0, DMA_INT_FLAG_G);
	
//	gpio_bit_reset (GPIOA, GPIO_PIN_2);
}
//--------------------------------------------------------------------------//
#define _VrefInt (1.200)
#define _ADC0dVref (_ADC0Vref / 4096.0)
#define _TempSlope (4.3e-3)
#define _TempOfset (1.45)
//--------------------------------------------------------------------------//
static float VrefInt = _VrefInt;
//--------------------------------------------------------------------------//
static float MonADC0ScalVal (TADCChannel Chan, float val)
{
	static bool InitVrefInt = false;
	val *= _ADC0dVref;
	if (_Voltage_RefInt == Chan)
	{
		if (InitVrefInt) VrefInt = VrefInt * 0.99 + val * 0.01;
		else VrefInt = val;
		InitVrefInt = true;
	}
	val *= _VrefInt / VrefInt; 
//	val -= 0.035f;
	if (Chan != _TempSensor) val *= CalibADCCoeffMul [Chan];
	else val = ((_TempOfset - val) / _TempSlope) + 25.0f;
	return (val);
}
//--------------------------------------------------------------------------//
static void MinMaxSwap (TADCChannel Chan, float * Min, float * Max)
{
	float f;
	if (CalibADCCoeffMul [Chan] < 0.0)
	{
		f = *Min;
		*Min = *Max;
		*Max = f;
	}
}
//--------------------------------------------------------------------------//
void RoutineADC (void)
{
	float fCount;
	if (true == ADCReadyData)
	{
		TADCChannel i;
		for (i = 0; i < _ADCNumberChannel; i++)
		{
			ADCResultAVG [i] = (float)(SSumADC [i]);
			ADCResultMax [i] = SMaxADC [i];
			ADCResultMin [i] = SMinADC [i];
			fCount = (float)SCount;
		}
		ADCReadyData = false;
		for (i = 0; i < _ADCNumberChannel; i++)
		{
			ADCResultAVG [i] /= fCount;
			ADCResultMin [i] = MonADC0ScalVal (i, ADCResultMin [i]);
			ADCResultAVG [i] = MonADC0ScalVal (i, ADCResultAVG [i]);
			ADCResultMax [i] = MonADC0ScalVal (i, ADCResultMax [i]);
			MinMaxSwap (i, &(ADCResultMin [i]), &(ADCResultMax [i]));
		}
		DataReady = true;
	}
}
//--------------------------------------------------------------------------//
bool GetADCDataReady (void)
{
	bool r = DataReady;
	DataReady = false;
	return (r);
}
//--------------------------------------------------------------------------//
float ResultDataADCAVG (TADCChannel Chan)
{
	if (/*Chan < 0 || */Chan >= _ADCNumberChannel) return (0.0);
	else return (ADCResultAVG [Chan]);
}
//--------------------------------------------------------------------------//
float ResultDataADCMax (TADCChannel Chan)
{
	if (/*Chan < 0 || */Chan >= _ADCNumberChannel) return (0.0);
	else return (ADCResultMax [Chan]);
}
//--------------------------------------------------------------------------//
float ResultDataADCMin (TADCChannel Chan)
{
	if (/*Chan < 0 || */Chan >= _ADCNumberChannel) return (0.0);
	else return (ADCResultMin [Chan]);
}
//--------------------------------------------------------------------------//

