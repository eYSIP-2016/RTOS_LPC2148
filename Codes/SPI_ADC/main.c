
/*

LCD Output

******************
*IR1 IR3 IR5 IR7 *
*IR6 IR8 SS1 SS5 *
******************

*/
#include<stdlib.h>
#include "lcd.h"
#include "type.h"
#include "spi.h"
#include <math.h>

/**************Macros*****************/


// SPI communication
#define	SPI1_SLAVE_SELECT	0x00100000

#define SENSOR_OFF() IO1SET=(1<<16)		//Macro to turn OFF Sensors
#define SENSOR_ON() IO1CLR=(1<<16)		//Macro to turn ON Sensors 
unsigned int sen_dat[8];

/*************************************/


/************Prototypes***************/

// void Init_Buzzer_Pin(void);
 void Init_Sensor_Switch_Pin(void);

 BYTE MEGA8_ADCRead(BYTE channel);
/************************************************************

	Function 		: Init_Sensor_Switch_Pin
	Return type		: None
	Parameters		: None
	Description 	: Initialises Sensor switching pin
************************************************************/
                               
void Init_Sensor_Switch_Pin(void)
{
 IO1DIR&=0xFFFEFFFF;
 IO1DIR|=(1<<16);   //Set P1.16 direction as Output
 SENSOR_OFF();		   //Initially turn OFF Sensor
}



BYTE MEGA8_ADCRead(BYTE channel)
{
	BYTE	adcVal = 0;
	DWORD i = 0;

	IOCLR0 |= SPI1_SLAVE_SELECT;		// slave select enable
	SPI1_SendByte(channel);

	//Delay for settling down  (80 uS)
	for (i=0; i<1000; i++);

	adcVal = SPI1_ReceiveByte();
	IOSET0 |= SPI1_SLAVE_SELECT;		// slave select disable

	//Delay for settling down  (80 uS)
	for (i=0; i<1000; i++);

	return adcVal;

}
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
            
void adc_data()
{ 
  sen_dat[0]=MEGA8_ADCRead(6);	       //IR 1
   sen_dat[1]=MEGA8_ADCRead(9);		   //IR 3
   sen_dat[2]=MEGA8_ADCRead(15);	   //IR	5
   sen_dat[4]=MEGA8_ADCRead(5);	   	   //IR	6
   sen_dat[3]=MEGA8_ADCRead(4);		   //IR 7 
   sen_dat[5]=MEGA8_ADCRead(0);	       //IR	8
  
   sen_dat[6]=MEGA8_ADCRead(7);        //Sharp 1
  sen_dat[6]=Sharp_GP2D12_Estimation(sen_dat[6]);

   sen_dat[7]=MEGA8_ADCRead(14);       //Sharp 5
   sen_dat[7]=Sharp_GP2D12_Estimation(sen_dat[7]);
     

}
 void Init_Ports(void)
{

 Init_LCD_Pin();
}


void Init_Peripherals(void)
{
 Init_Ports();
Init_Sensor_Switch_Pin();

 SPI1_Init();
}

int main()
{
 PINSEL0 = 0x00000000;
 PINSEL1 = 0x00000000;
 //PINSEL2 = 0x00000000;
 Init_Peripherals();
 DelaymSec(40);
 Init_Peripherals();
 LCD_4Bit_Mode();
 LCD_Init();
 LCD_Command(0x01);
 DelaymSec(15);
   while(1)
  {
	adc_data();

   LCD_Print(1,1,sen_dat[0],3);	  // IR 1
   LCD_Print(1,5,sen_dat[1],3);	  //using channel 9 for IR 3 values but no O/P
   LCD_Print(1,9,sen_dat[2],3);	  // IR 5
   LCD_Print(1,13,sen_dat[3],3);	 //IR 7
   LCD_Print(2,1,sen_dat[4],3);		  //IR 6
   LCD_Print(2,5,sen_dat[5],3);		 //IR 8

   LCD_Print(2,9,sen_dat[6],3);	 //Sharp 1
   LCD_Print(2,13,sen_dat[7],3);  //sharp 5
   DelaymSec(100);
  }
}
