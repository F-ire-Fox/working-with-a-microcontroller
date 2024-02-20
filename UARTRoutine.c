/*=============================================================================
2	    Project: 
3     Platform: GD32E230
4     Filename: UARTRoutine.c
5     Description:
6     Version: 0.0
7     Created: 2023.09.16
8     Last modified: 2023.09.16
9============================================================================*/
//#include <C8051F410.h>
#include "gd32e23x.h"
//------------------------------------------------------------------------------
//#include "Global.h"
#include "drv_time.h"
#include "drv_uart.h"
#include "drv_ADC.h"
#include "UARTRoutine.h"
#include "timestamp.h"
#include "crc8_CCITT.h"
#include "drv_FreqMet.h"
//--------------------------------------------------------------------------//
#define _Sinxr (0x80808080)
#define _DevAddr (0x03)
//-----------------------------------------------------------------------------
typedef struct
{
  uint32_t Sinxr;
  uint32_t UpTime;

	float Current_In_Max;
	float Current_In_Min;
	float Current_In_AVG;
	float Voltage_In_Max;
	float Voltage_In_Min;
	float Voltage_In_AVG;
	float Power_In_Max;
	float Power_In_Min;
	float Power_In_AVG;
	float Voltage_RefInt_Max;
	float Voltage_RefInt_Min;
	float Voltage_RefInt_AVG;
	float ConversionFrequency;
	float TempSensor;
	
  uint32_t BuildTime;
  uint16_t Reserv;
  uint16_t CRC_CCITT16;
} TMSPRAWStruct;
//-----------------------------------------------------------------------------
typedef union
{
  TMSPRAWStruct St;
  uint8_t Byte [sizeof(TMSPRAWStruct)];
} TMSPRAWUnion;
//--------------------------------------------------------------------------//
static  TMSPRAWUnion Data;
//-----------------------------------------------------------------------------
void InitUARTRoutine (void)
{
	uint16_t i;
	for (i = 0; i < sizeof (TMSPRAWStruct); i++) Data.Byte [i] = 0;
}
//--------------------------------------------------------------------------//
//-----------------------------------------------------------------------------
static void SendADCData (void)
{
  uint16_t i;

  Data.St.Sinxr = _Sinxr;
  Data.St.UpTime = GetTimeSec ();

	
	Data.St.Current_In_Min			= ResultDataADCMin (_Current_In);
	Data.St.Current_In_AVG			= ResultDataADCAVG (_Current_In);
	Data.St.Current_In_Max 			= ResultDataADCMax (_Current_In);
	Data.St.Voltage_In_Min  		= ResultDataADCMin (_Voltage_In);
	Data.St.Voltage_In_AVG   		= ResultDataADCAVG (_Voltage_In);
	Data.St.Voltage_In_Max   		= ResultDataADCMax (_Voltage_In);
	Data.St.Power_In_Min  			= ResultDataADCMin (_Power_In);
	Data.St.Power_In_AVG  			= ResultDataADCAVG (_Power_In);
	Data.St.Power_In_Max  			= ResultDataADCMax (_Power_In);
	Data.St.Voltage_RefInt_Min 	= ResultDataADCMin (_Voltage_RefInt);
	Data.St.Voltage_RefInt_AVG 	= ResultDataADCAVG (_Voltage_RefInt);
	Data.St.Voltage_RefInt_Max 	= ResultDataADCMax (_Voltage_RefInt);
	Data.St.ConversionFrequency = ResultFreqMet ();
	Data.St.TempSensor					=	ResultDataADCAVG (_TempSensor);

  Data.St.BuildTime = BUILD_TIME;
  Data.St.Reserv = 0;
  Data.St.CRC_CCITT16 = CRC8_CCITT (Data.Byte, sizeof(TMSPRAWStruct) - 2);

  for (i = 0; i < sizeof(TMSPRAWStruct); i++)
    PushTxFIFO (Data.Byte [i]);

	Start_trsUART ();
}
//--------------------------------------------------------------------------//
typedef struct {
		uint32_t Sinxr;
		uint8_t Addr;
		uint8_t Reserve8;
		uint16_t Reserve16;
		uint16_t Reserv;
		uint16_t CRC_CCITT16;
} TRequestStruct;

typedef union {
	TRequestStruct st;
	uint8_t u8 [sizeof (TRequestStruct)];
} TRequestUnion;
//--------------------------------------------------------------------------//
typedef enum {_RSIdle, _RSSync0, _RSSync1, _RSSync2, _RSRecieve} TRequestState;
#define _ByteTO (10)
//--------------------------------------------------------------------------//
static bool RequestAccepted (void)
{
	static TRequestUnion RS;
	static TRequestState Mode;
	static uint32_t Index;
	static TTime TO = 0;
	uint32_t t = GetNumberOfByteRxFIFO ();
	uint8_t b;
	bool ret = false;

	if (t == 0) return (ret);
	
	if (EndTime (TO)) Mode = _RSIdle;
	
	while (t > 0) 
	{
		b = PopRxFIFO ();
		t--;
		switch (Mode)
		{
			case _RSIdle:
				if (b == (_Sinxr & 0xff))
				{
					TO = SetTime_ms (_ByteTO);
					Index = 0;
					RS.u8 [Index] = b;
					Index++;
					Mode = _RSSync0;
				}
				break;
			case _RSSync0:
				if (b == ((_Sinxr >> 8) & 0xff))
				{
					TO = SetTime_ms (_ByteTO);
					RS.u8 [Index] = b;
					Index++;
					Mode = _RSSync1;
				}
				else Mode = _RSIdle;
				break;
			case _RSSync1:
				if (b == ((_Sinxr >> 16) & 0xff))
				{
					TO = SetTime_ms (_ByteTO);
					RS.u8 [Index] = b;
					Index++;
					Mode = _RSSync2;
				}
				else Mode = _RSIdle;
				break;
			case _RSSync2:
				if (b == ((_Sinxr >> 24) & 0xff))
				{
					TO = SetTime_ms (_ByteTO);
					RS.u8 [Index] = b;
					Index++;
					Mode = _RSRecieve;
				}
				else Mode = _RSIdle;
				break;
			case _RSRecieve:
					TO = SetTime_ms (_ByteTO);
					RS.u8 [Index] = b;
					Index++;
					if (sizeof (TRequestStruct) == Index)
					{
						if ( RS.st.CRC_CCITT16 == CRC8_CCITT (RS.u8, sizeof (TRequestStruct) - 2))
						{
							if (_DevAddr == RS.st.Addr) ret = true;
						}
						Mode = _RSIdle;
					}
				break;
		}
	}
	return (ret);
}
//--------------------------------------------------------------------------//
void UARTRoutine (void)
{
	uint8_t i;
	if (GetNumberOfFreeByteTxFIFO () >= sizeof (TMSPRAWStruct))
//		if (GetADCDataReady ())
		if (RequestAccepted ())
			SendADCData ();
}
