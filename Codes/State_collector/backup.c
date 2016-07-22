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
unsigned char sen_dat[31];
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


//This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
void send(void *pvparam)
{  
 
 while(1)
 {
//________________ IR PROXIMITY STATUS COMMANDS _____________________
 
/*  UART0_SendByte(MEGA8_ADCRead(6));		//IR 1
  UART0_SendByte(MEGA8_ADCRead(13));UART0_SendByte(',');	//IR 2
  UART0_SendByte(MEGA8_ADCRead(9));	UART0_SendByte(',');	//IR 3
  UART0_SendByte(MEGA8_ADCRead(8));UART0_SendByte(',');		//IR 4
  UART0_SendByte(MEGA8_ADCRead(15));	UART0_SendByte(',');//IR 5
  UART0_SendByte(MEGA8_ADCRead(5));	 UART0_SendByte(',');  //IR 6
  UART0_SendByte(MEGA8_ADCRead(4));	UART0_SendByte(',');   //IR 7
  UART0_SendByte(MEGA8_ADCRead(0));	  UART0_SendByte(','); //IR 8
  UART0_SendByte(battery_voltage);	 UART0_SendByte(',');//Battery voltage Value
  
  UART0_SendByte(ADC_Data[3]);UART0_SendByte(',');	//WL Left
  UART0_SendByte(ADC_Data[4]);UART0_SendByte(',');	//WL Center
  UART0_SendByte(ADC_Data[6]);UART0_SendByte(',');	//WL Right
  
  UART0_SendByte(ADC_Data[2]);	UART0_SendByte(',');//Sharp 4
  UART0_SendByte(MEGA8_ADCRead(7));UART0_SendByte(',');//Sharp 1
  UART0_SendByte(ADC_Data[0]);UART0_SendByte(',');	//Sharp 2
  UART0_SendByte(ADC_Data[1]); UART0_SendByte(',');//Sharp 3 (Front)
  UART0_SendByte(MEGA8_ADCRead(14));UART0_SendByte(',');//Sharp 5
  //UART0_SendByte(Robot_ID);//Identity tag
  */
   sen_dat[0]=MEGA8_ADCRead(6);
   sen_dat[2]=MEGA8_ADCRead(13);
   sen_dat[4]=MEGA8_ADCRead(9);
   sen_dat[6]=MEGA8_ADCRead(8);
   sen_dat[8]=MEGA8_ADCRead(15);
   sen_dat[10]=MEGA8_ADCRead(4);
   sen_dat[12]=MEGA8_ADCRead(0);
  /* 
   sen_dat[14]=ADC_Data[3];
   sen_dat[16]=ADC_Data[4];
   sen_dat[18]=ADC_Data[6];

   sen_dat[20]=ADC_Data[2];
   sen_dat[22]=MEGA8_ADCRead(7);
   sen_dat[24]=ADC_Data[0];
   sen_dat[26]=ADC_Data[1];
   sen_dat[28]=MEGA8_ADCRead(14);
  // sen_dat[30]='\n';
  */
  UART0_SendStr(sen_dat);
  UART0_SendStr("\n");		   
  VICVectAddr = 0x00;
 }
}		
void line(void *pvparam)
{
while(1)
{
 UART0_SendStr("jhmb");
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
 SPI1_Init();
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


int main()
{
 PINSEL0 = 0x00000000;
 PINSEL1 = 0x00000000;
 //PINSEL2 = 0x00000000;
 Init_Peripherals();
 DelaymSec(40);
 LCD_4Bit_Mode();
 LCD_Init();
 LCD_Command(0x01);
 DelaymSec(15);
 

 xTaskCreate(line,"line", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
 xTaskCreate(send,"send", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
 vTaskStartScheduler();

  while(1);
}
