/*
The program contains 3 tasks 
1. vline :the bot follows a program written by the user
2. vcalc :storing of adc values	(obtained as hex in a an array)
3. vsend :transmitting the stored values

the values are calculated every 1sec/1000 clock ticks and transmitted every 1 sec/1000 clock ticks

**while processing the serial monitor data,note that 00 FF is the delimiter and the space between 
***the data are Hex values and have not been converted to appropriate units
*/


#include<stdlib.h>
#include "FreeRTOS.h"
#include "lcd.h"
#include "uart.h"
#include "pwm.h"
#include "motion_control.h"
#include "type.h"
#include "spi.h"
#include "adc.h"
#include "task.h"
/**************Macros*****************/

// SPI communication
#define	SPI1_SLAVE_SELECT	0x00100000

#define SENSOR_OFF() IO1SET=(1<<16)		//Macro to turn OFF Sensors
#define SENSOR_ON() IO1CLR=(1<<16)		//Macro to turn ON Sensors 
unsigned char sen_dat[17];
int i=0;
/*************************************/


/************Prototypes***************/

 void Init_Sensor_Switch_Pin(void);
 BYTE MEGA8_ADCRead(BYTE channel);
/*************************************/


/***********Global Variables**********/

 
 unsigned char Robot_ID=0x07;

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
//This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
void vsend(void *pvparam)
{  
 
 while(1)
 {
  UART1_SendStr(sen_dat);
  UART1_SendByte(0x00);
  UART1_SendByte(0xFF);
  vTaskDelay(1000);	   
  
 }
}
void vcalc(void *pvparam)
{  
 
 while(1)
 {
   sen_dat[0]=MEGA8_ADCRead(6);		// IR 1
   sen_dat[1]=MEGA8_ADCRead(13);	// IR 2
   sen_dat[2]=MEGA8_ADCRead(9);		// IR 3
   sen_dat[3]=MEGA8_ADCRead(8);		// IR 4
   sen_dat[4]=MEGA8_ADCRead(15);	// IR 5
   sen_dat[5]=MEGA8_ADCRead(4);	// IR 6
   sen_dat[6]=MEGA8_ADCRead(0);	// IR 7
   sen_dat[7]=MEGA8_ADCRead(7);	//sharp 1
   sen_dat[8]=AD0_Conversion(6);	 //sharp 2
   sen_dat[9]=AD1_Conversion(0);	 //sharp 3
   sen_dat[10]=AD0_Conversion(7);	 //sharp 4
   sen_dat[11]=MEGA8_ADCRead(14);	 //sharp 5
   sen_dat[12]=AD1_Conversion(3);	 //WL left
   sen_dat[13]=AD0_Conversion(1);	 //WL center
   sen_dat[14]=AD0_Conversion(2);	 //WL right
  	
   vTaskDelay(1000);
 }
}		
void vline(void *pvparam)
{
while(1)
{
UpdateVelocity(400,400);
Forward();
}
}

void Init_Ports(void)
{
 Init_Motion_Pin();
 Init_ADC_Pin();
 Init_LCD_Pin();
}


void Init_Peripherals(void)
{
 Init_Ports();
 Init_Motion_Pin();
 Init_Sensor_Switch_Pin();
 Init_ADC0();
 Init_ADC1();
 Init_PWM();
 Init_UART0();
 Init_UART1();
 SPI1_Init();
}




int main()
{
 PINSEL0 = 0x00000000;
 PINSEL1 = 0x00000000;
 //PINSEL2 = 0x00000000;
 Init_Peripherals();

  
 xTaskCreate(vline,"line", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
 xTaskCreate(vcalc,"calc", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
 xTaskCreate(vsend,"send", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
 vTaskStartScheduler();

  while(1);
}
