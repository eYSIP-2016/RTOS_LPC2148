#include<stdlib.h>

#include "LCD.h"
#include "motion_control.h"
#include "type.h"
#include "spi.h"
#include <math.h>
/**************Macros*****************/


// SPI communication
#define	SPI1_SLAVE_SELECT	0x00100000

#define SENSOR_OFF() IO1SET=(1<<16)		//Macro to turn OFF Sensors
#define SENSOR_ON() IO1CLR=(1<<16)		//Macro to turn ON Sensors 
/*************************************/


/************Prototypes***************/
 void Init_ADC0(void);
void Init_ADC1(void);
unsigned int AD0_Conversion(unsigned char channel);
unsigned int AD1_Conversion(unsigned char channel);
 void Init_Sensor_Switch_Pin(void);
 void Init_PWM(void);
void UpdateLeftPWM(unsigned int vel);
void UpdateRightPWM(unsigned int vel);

 BYTE MEGA8_ADCRead(BYTE channel);
 void setServoAngle(BYTE servoNumber, BYTE angle);
/*************************************/


/***********Global Variables**********/
unsigned char sen_dat[8];
unsigned int ADC_Data[8];
/*************************************/


void Init_Ports(void)
{
 Init_ADC_Pin();
 Init_Motion_Pin();
 Init_LCD_Pin();
}


void Init_Peripherals(void)
{
 Init_Ports();
 Init_ADC_Pin();
 Init_ADC0();
 Init_ADC1();
Init_Sensor_Switch_Pin();
 SPI1_Init();
 Init_PWM();
}
/*****************************************
SPI Data transmission and reception
******************************************/
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
/**********************************************
Taking Sensor values in a Variable
**********************************************/
void adc_data()
{ 
   sen_dat[0]=MEGA8_ADCRead(6);	       //IR 1
   ADC_Data[0] = AD0_Conversion(6);	   //IR 2
   sen_dat[1]=MEGA8_ADCRead(9);		   //IR 3
   ADC_Data[2] = AD0_Conversion(7);    //IR 4
   sen_dat[2]=MEGA8_ADCRead(15);	   //IR	5
   sen_dat[4]=MEGA8_ADCRead(5);	   	   //IR	6
   sen_dat[3]=MEGA8_ADCRead(4);		   //IR 7 
   sen_dat[5]=MEGA8_ADCRead(0);	       //IR	8
  
   /*sen_dat[6]=MEGA8_ADCRead(7);        //Sharp 1
   sen_dat[6]=Sharp_GP2D12_Estimation(sen_dat[6]);

   sen_dat[7]=MEGA8_ADCRead(14);       //Sharp 5
   sen_dat[7]=Sharp_GP2D12_Estimation(sen_dat[7]);
   */
  
    
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
  UpdateLeftPWM(225);	//Velocity Setting
 UpdateRightPWM(225);	//Velocity Setting
 DelaymSec(15);
 
 
  while(1)
  {
	adc_data();

   LCD_Print(1,1,sen_dat[0],3);	    //IR 1
   LCD_Print(1,5,ADC_Data[2],3);    //IR 2
   LCD_Print(1,9,sen_dat[1],3);     //IR 3
   LCD_Print(1,13,ADC_Data[0],3);   //IR 4 
   LCD_Print(2,1,sen_dat[2],3);	    //IR 5
   LCD_Print(2,5,sen_dat[4],3);		//IR 6
   LCD_Print(2,9,sen_dat[3],3);	    //IR 7
   LCD_Print(2,13,sen_dat[5],3);    //IR 8

   //LCD_Print(2,9,sen_dat[6],3);	 //Sharp 1
   //LCD_Print(2,13,sen_dat[7],3);  //sharp 5
if (sen_dat[0]<190) //IR 1
  {

     Stop();
	 if (ADC_Data[2]<180)
	 {
     //DelaymSec(150);
     Soft_Right();
     DelaymSec(100);
	 }
	 else if(sen_dat[2]>170)
	 {
	  Right();
	  DelaymSec(100);
	 }
	 else
	 {
	  Back();
	  DelaymSec(100);
	 }
	 if(sen_dat[1]>190)
	 {
	    Forward();
	 }
  }




if (ADC_Data[0]>170) //IR 2
  {


     Stop();
	 if( sen_dat[5]>180)
	 {
     //DelaymSec(150);
     Soft_Left2();
     DelaymSec(100);
	 }
	 else if(ADC_Data[2]<180)
	 {
	  Soft_Right();
	  DelaymSec(100);
	 }
	 else 
	 {
	  Back();
	  DelaymSec(100);
	 }

	 if(ADC_Data[0]<180)
	 {
	    Forward();
	 }
}





 if(sen_dat[1]<210) //IR 3
   {


     Stop();
	 if( sen_dat[2]>170)
	 { 
     //DelaymSec(200);
     Right();
	 DelaymSec(100);
	 }
	 else if (sen_dat[0]>190)
	 {
	  Left();
	  DelaymSec(100);
	 }
	 else if(sen_dat[2]>170)
	 {
	   Right();
	   DelaymSec(100);
	 }
	 else
	 {
	  Back();
	  DelaymSec(100);
	 }
    
	 if(sen_dat[1]>200)
	 {
	   Forward();
	 } 
}




if( ADC_Data[2]>170) //IR 4
   {


     Stop();
	 if(sen_dat[4]>215)
	 {
     //DelaymSec(200);
     Soft_Right2();
     DelaymSec(100);
	 }
	 else if(ADC_Data[0]<170)
	 {
	  Soft_Left();
	  DelaymSec(100);
	 }
	 else
	 {
	 Back();
	 DelaymSec(100);
	 }
	 if(ADC_Data[2]<180)
	 {
	   Forward();
	 } 
 }
 


 if( sen_dat[2]<170) //IR 5
   {


     Stop();
	 if( ADC_Data[2]<180)
	 {
     //DelaymSec(200);
     Soft_Left();
     DelaymSec(100);
	 }
	 else if(sen_dat[0]<190)
	 {
	 Right();
	 DelaymSec(100);
	 }
	 else
	 {
	 Back();
	 DelaymSec(100);
	 }
	 if(sen_dat[2]>170)
	 {
	   Forward();
	 } 
   }



if(sen_dat[4]<200) //IR 6
   {


     Stop();
	 if( ADC_Data[2]<180)
	 {
     //DelaymSec(200);
     Soft_Right();
     DelaymSec(100);
	 }
	 else if(ADC_Data[0]<150)
	 {
	 Soft_Left();
	 DelaymSec(100);
	 }
	 else
	 {
	 Stop();
	 DelaymSec(300);
	 }
	 if(sen_dat[4]>200)
	 {
	  Forward();
	 } 
   }


if(sen_dat[3]<200) //IR 7
   {

     Stop();
	 if( sen_dat[1]>200)
	 { 
     //DelaymSec(200);
     Forward();
	 DelaymSec(100);
	 }
	 else if (sen_dat[0]>190)
	 {
	  Left();
	  DelaymSec(100);
	 }
	 else if (sen_dat[2]>170) 
	 {
	  Right();
	  DelaymSec(100);
	 }
	 else
	 {
	  Back();
	  DelaymSec(100);
	 }
    
	 if(sen_dat[3]>200)
	 {
	   Forward();
	 } 
}



if(sen_dat[5]<180) //IR 7
   {

     Stop();
	 if( ADC_Data[2]<190)
	 {
     //DelaymSec(200);
     Soft_Right();
     DelaymSec(100);
	 }
	 else if(ADC_Data[0]<150)
	 {
	 Soft_Left();
	 DelaymSec(100);
	 }
	 else
	 {
	 Stop();
	 DelaymSec(500);
	 }
	 if(sen_dat[5]>190)
	 {
	   Forward();
	 } 
}

if((ADC_Data[0]>180) &&(sen_dat[1]<180)) //IR 2 and IR 3
   {

     Stop();
	 Back();
     DelaymSec(100);
     Soft_Left2();
     DelaymSec(100);
	 if((ADC_Data[0]<180)&&(sen_dat[1]>180))
	 {
	   Forward();
	 } 
 }
  


if(( ADC_Data[2]>150) && (sen_dat[1]<190) && (ADC_Data[0]>120) ) //IR 3 and IR 4
   {

     Stop();
	 Back();
     DelaymSec(100);
     Soft_Right2();
     DelaymSec(100);
	 if(( ADC_Data[2]<150)&&(sen_dat[1]>190) && (ADC_Data[0]<120))
	 {
	   Forward();
	 } 
   }



if(( ADC_Data[2]>180) && (sen_dat[1]<180)) //IR 3 and IR 4
   {
     Stop();
	 Back();
     DelaymSec(100);
     Soft_Left2();
     DelaymSec(100);
	 if(( ADC_Data[2]<180)&&(sen_dat[1]>180))
	 {
	   Forward();
	 } 
}}}  
