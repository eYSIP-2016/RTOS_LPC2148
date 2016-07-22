#include <LPC214x.h>
#include <math.h>
#include "adc.h"
#include "lcd.h"


extern void DelaymSec(unsigned int j);
/*************************************************************

ADC Connection:
 			  	Sensor									  ADC Channel No.

			  	Battery Voltage									AD1.4(P0.13)
			  	White line sensor 1								AD1.3(P0.12)
			  	White line sensor 2								AD0.1(P0.28)
			  	White line sensor 3								AD0.2(P0.29)
			  	IR Proximity analog sensor 2*****				AD0.6(P0.4)
			  	IR Proximity analog sensor 4*****				AD0.7(P0.5)
			  	Sharp IR range sensor 2							AD0.6(P0.4)
			  	Sharp IR range sensor 3							AD1.0(P0.6)
			  	Sharp IR range sensor 4							AD0.7(P0.5)
			  	

****************************************************************/


unsigned int ADC_Data[8];


/************************************************************

	Function 		: Init_ADC_Pin
	Return type		: None
	Parameters		: None
	Description 	: Initialises the required GPIOs as ADC pins
************************************************************/

void Init_ADC_Pin(void)
{
 PINSEL0&= 0xF0FFC0FF;
 PINSEL0|= 0x0F003F00;		//Set pins P0.4, P0.5, P0.6, P0.12, P0.13 as ADC pins
 PINSEL1&= 0xF0FFFFFF;		
 PINSEL1|= 0x05000000;	    //Set pins P0.28, P0.29 as ADC pins
}

/************************************************************

	Function 		: ADC0_Init
	Return type		: None
	Parameters		: None
	Description 	: This fuction initialises ADC 0
					  module of LPC2148 microcontroller. It also 
					  configures the required I/o pins to be used as 
					  ADC pins. 
************************************************************/
void Init_ADC0(void)
{
 AD0CR=0x00200E00;	// SEL = 1 	ADC0 channel 1	Channel 1
					// CLKDIV = Fpclk / 1000000 - 1 ;1MHz
					// BURST = 0 
					// CLKS = 0 
 					// PDN = 1 
 					// START = 1
  					// EDGE = 0 (CAP/MAT)
} 


/************************************************************

	Function 		: ADC1_Init
	Return type		: None
	Parameters		: None
	Description 	: This fuction initialises ADC 1
					  module of LPC2148 microcontroller. It also 
					  configures the required I/o pins to be used as 
					  ADC pins. 
************************************************************/
void Init_ADC1(void)
{
 AD1CR=0x00200E00;	// SEL = 1 	ADC0 channel 1	Channel 1
					// CLKDIV = Fpclk / 1000000 - 1 ;1MHz
					// BURST = 0 
					// CLKS = 0 
 					// PDN = 1 
 					// START = 1
  					// EDGE = 0 (CAP/MAT)
} 
                           

//This function converts ADC0 channels. Channel number is passed to this function as integer.
unsigned int AD0_Conversion(unsigned char channel)
{
 unsigned int Temp;
 if(channel!=0)
 {
  AD0CR = (AD0CR & 0xFFFFFF00) | (1<<channel);
 }
 else
 {
  AD0CR = (AD0CR & 0xFFFFFF00) | 0x01;
 }
 AD0CR|=(1 << 24);
 //while((AD0GDR&0x80000000)==0);
 DelaymSec(1);
 Temp = AD0GDR;						
 Temp = (Temp>>8) & 0xFF;
 return Temp;
}

//This function converts ADC1 channels. Channel number is passed to this function as integer.
unsigned int AD1_Conversion(unsigned char channel)
{
 unsigned int Temp;
 if(channel!=0)
 {
  AD1CR = (AD1CR & 0xFFFFFF00) | (1<<channel);
 }
 else
 {
  AD1CR = (AD1CR & 0xFFFFFF00) | 0x01;
 }
 AD1CR|=(1 << 24);
 //while((AD1GDR&0x80000000)==0);
 DelaymSec(1);
 Temp = AD1GDR;						
 Temp = (Temp>>8) & 0xFF;
 return Temp;
}

//This Function estimates the raw digital data of Sharp sensor in mm
unsigned int Sharp_GP2D12_Estimation(unsigned int Val)
{
 float Distance;
 unsigned int DistanceInt;
 Distance = (int)(10.00*(2799.6*(1.00/(pow(Val,1.1546)))));
 DistanceInt = (int)Distance;
 if(DistanceInt>800)
 {
  DistanceInt=800;
 }
 return DistanceInt;
}

//This function convetrs the raw digital data of battery to represent actual battery voltage
unsigned int Batt_Voltage_Conversion(unsigned int Val)
{
 float Batt_Voltage;
 unsigned int Batt_Voltage_Int=0;
 Batt_Voltage = ((float)Val* 0.05226 * 100.0) + 0.7;	//0.05226= (3.3/255)*(1/(3.3/13.3))
														//0.7 = Drop across diode
														//100.0 = Shifting decimal point by 2
 Batt_Voltage_Int = (unsigned int)Batt_Voltage;
 return Batt_Voltage_Int;
}
