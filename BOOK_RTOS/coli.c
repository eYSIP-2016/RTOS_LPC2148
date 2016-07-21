#include<stdlib.h>
#include "FreeRTOS.h"
#include "LCD.h"
#include "task.h"
#include "type.h"
#include "spi.h"
#include <math.h>

/**************Macros*****************/


// SPI communication
#define	SPI1_SLAVE_SELECT	0x00100000

#define SENSOR_OFF() IO1SET=(1<<16)		//Macro to turn OFF Sensors
#define SENSOR_ON() IO1CLR=(1<<16)		//Macro to turn ON Sensors 
/*************************************/

/*******************************************
Task handles
*******************************************/
TaskHandle_t  xIR1,xIR2,xIR3,xIR4,xIR5,xIR6,xIR7,xIR8,xIR1_IR2,xIR2_IR3,xIR3_IR4,xIR4_IR5,xIR2_IR3_IR4,xforward,xdisplay;

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
 /***************************************
 RTOS Functions
 *****************************************/
 void IR1 (void *);
 void IR2 (void *);
 void IR3 (void *);
 void IR4 (void *);
 void IR5 (void *);
 void IR6 (void *);
 void IR7 (void *);
 void IR8 (void *);
  void IR1_IR2 (void *);
 void IR2_IR3 (void *);
 void IR3_IR4 (void *);
  void IR4_IR5 (void *);
 void IR2_IR3_IR4 (void *);
 void lcdprint(void *);
 void forward(void *);

/***********Global Variables**********/
unsigned char sen_dat[8];
unsigned int ADC_Data[8];
/*************************************/

void IR1(void *p)
{
while(1)
{
 UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
if (sen_dat[0]<190) //IR 1
  {
     Stop();
	 vTaskSuspend(xforward);
	 if (ADC_Data[2]<180)
	 {

     //DelaymSec(150);
     Soft_Right();
     DelaymSec(50);

	 }
	 else if(sen_dat[2]>170)
	 {
	  Right();
	  DelaymSec(50);
	 }
	 else
	 {
	  Back();
	  DelaymSec(50);
	 }
	 if(sen_dat[1]>190)
	 {
	    vTaskResume(xforward);
	 }
  }

}
}

void IR2(void *p)
{
while(1)
{
 UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
if (ADC_Data[0]>160) //IR 2
  {
     Stop();
	 vTaskSuspend(xforward);
	 if( sen_dat[5]>180)
	 {
     //DelaymSec(150);
     Soft_Left2();
     DelaymSec(50);
	 }
	 else if(ADC_Data[2]<180)
	 {
	  Soft_Right();
	  DelaymSec(50);
	 }
	 else 
	 {
	  Back();
	  DelaymSec(50);
	 }

	 if(ADC_Data[0]<180)
	 {
	    vTaskResume(xforward);
	 }
  }

}
}


void IR3(void *p)
{
while(1)
{
 UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
 if(sen_dat[1]<210) //IR 3
   {
     Stop();
	 vTaskSuspend(xforward);
	 if( sen_dat[2]>170)
	 { 
     //DelaymSec(200);
     Right();
   	DelaymSec(50);
	Stop();
	DelaymSec(50);
	 }
	 else if (sen_dat[0]>190)
	 {
	  Left();
	  DelaymSec(50);
	 }
	 else if(sen_dat[2]>170)
	 {
	   Right();
	   DelaymSec(50);
	 }
	 else
	 {
	  Back();
	  DelaymSec(50);
	 }
    
	 if(sen_dat[1]>200)
	 {
	   vTaskResume(xforward);
	 } 
}

}
}

void IR4(void *p)
{
while(1)
{
UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
 if( ADC_Data[2]>150) //IR 4
   {
     Stop();
	 vTaskSuspend(xforward);
	 if(sen_dat[4]>215)
	 {
     //DelaymSec(200);
     Soft_Right2();
     DelaymSec(50);
	 }
	 else if(ADC_Data[0]<170)
	 {
	  Soft_Left();
	  DelaymSec(50);
	 }
	 else
	 {
	 Back();
	 DelaymSec(50);
	 }
	 if(ADC_Data[2]<180)
	 {
	   vTaskResume(xforward);
	 } 
   }
   }
}

void IR5(void *p)
{
while(1)
{
UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
 if( sen_dat[2]<170) //IR 5
   {
     Stop();
	 vTaskSuspend(xforward);
	 if( ADC_Data[2]<180)
	 {
     //DelaymSec(200);
     Soft_Left();
     DelaymSec(50);
	 }
	 else if(sen_dat[0]<190)
	 {
	 Right();
	 DelaymSec(50);
	 }
	 else
	 {
	 Back();
	 DelaymSec(50);
	 }
	 if(sen_dat[2]>170)
	 {
	   vTaskResume(xforward);
	 } 
   }
   }
}

void IR6 (void *p)
{
while(1)
{
UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
 if(sen_dat[4]<210) //IR 6
   {
     Stop();
	 vTaskSuspend(xforward);
	 if( ADC_Data[2]<180)
	 {
     //DelaymSec(200);
     Soft_Right();
     DelaymSec(50);
	 }
	 else if(ADC_Data[0]<150)
	 {
	 Soft_Left();
	 DelaymSec(50);
	 }
	 else
	 {
	 Stop();
	 DelaymSec(100);
	 }
	 if(sen_dat[4]>200)
	 {
	   vTaskResume(xforward);
	 } 
   }
   }
}

void IR7(void *p)
{
while(1)
{
 UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
 if(sen_dat[3]<210) //IR 7
   {
     Stop();
	 vTaskSuspend(xforward);
	 if( sen_dat[1]>215)
	 { 
     //DelaymSec(200);
     Forward();
	 DelaymSec(50);
	 }
	 else if (sen_dat[0]>190)
	 {
	  Left();
	  DelaymSec(50);
	 }
	 else if (sen_dat[2]>170) 
	 {
	  Right();
	  DelaymSec(50);
	 }
	 else
	 {
	  Back();
	  DelaymSec(50);
	 }
    
	 if(sen_dat[3]>200)
	 {
	   vTaskResume(xforward);
	 } 
}

}
}

void IR8(void *p)
{
while(1)
{
 UpdateLeftPWM(400);	//Velocity Setting
 UpdateRightPWM(400);	//Velocity Setting
 if(sen_dat[5]<210) //IR 7
   {
     Stop();
	 vTaskSuspend(xforward);
	 if( ADC_Data[2]<190)
	 {
     //DelaymSec(50);
     Soft_Right();
     DelaymSec(50);
	 }
	 else if(ADC_Data[0]<150)
	 {
	 Soft_Left();
	 DelaymSec(50);
	 }
	 else
	 {
	 Stop();
	 DelaymSec(50);
	 }
	 if(sen_dat[5]>190)
	 {
	   vTaskResume(xforward);
	 } 
}
}
}

void IR2_IR3(void *p)
{
 while(1)
{
UpdateLeftPWM(400);	//Velocity Setting
UpdateRightPWM(400);	//Velocity Setting
if((ADC_Data[0]>60) &&(sen_dat[1]<230)) //IR 2 and IR 3
   {
     Stop();
	 vTaskSuspend(xforward);
	// Back();
     //DelaymSec(50);
     Soft_Right2();
     DelaymSec(50);
	 if((ADC_Data[0]<180)&&(sen_dat[1]>180))
	 {
	   vTaskResume(xforward);
	 } 
   }
  }
}

void IR2_IR3_IR4 (void *p)
{
 while(1)
{
UpdateLeftPWM(400);	//Velocity Setting
UpdateRightPWM(400);	//Velocity Setting
if(( ADC_Data[2]>150) && (sen_dat[1]<200) && (ADC_Data[0]>120) ) //IR 3 and IR 4
   {
     Stop();
	 vTaskSuspend(xforward);
	  Back();
     DelaymSec(50);
     //Soft_Right2();
     //DelaymSec(50);
	 if(( ADC_Data[2]<150)&&(sen_dat[1]>190) && (ADC_Data[0]<120))
	 {
	   vTaskResume(xforward);
	 } 
   }
  }			
}

void IR3_IR4 (void *p)
{
 while(1)
{
UpdateLeftPWM(400);	//Velocity Setting
UpdateRightPWM(400);	//Velocity Setting
if(( ADC_Data[2]>60) && (sen_dat[1]<230)) //IR 3 and IR 4
   {
     Stop();
	 vTaskSuspend(xforward);
	 Back();
     DelaymSec(50);
     Soft_Left2();
     DelaymSec(50);
	 if(( ADC_Data[2]<180)&&(sen_dat[1]>180))
	 {
	   vTaskResume(xforward);
	 } 
   }
  }
}
 


void lcdprint(void *p)
{
while(1)
{  
   sen_dat[1]=MEGA8_ADCRead(9);		   //IR 3
   sen_dat[0]=MEGA8_ADCRead(6);	  	   //IR 1	
   sen_dat[2]=MEGA8_ADCRead(15);	   //IR	5
   sen_dat[4]=MEGA8_ADCRead(5);	   	   //IR	6
   sen_dat[3]=MEGA8_ADCRead(4);		   //IR 7 
   sen_dat[5]=MEGA8_ADCRead(0);	       //IR	8
   ADC_Data[0] = AD0_Conversion(6);	   //IR 2
   ADC_Data[2] = AD0_Conversion(7);    //IR 4
   LCD_Print(1,5,ADC_Data[2],3);    //IR 4
   LCD_Print(1,9,sen_dat[1],3);     //IR 3
   LCD_Print(1,13,ADC_Data[0],3);   //IR 2 
   LCD_Print(2,1,sen_dat[2],3);	    //IR 5
   LCD_Print(2,5,sen_dat[4],3);		//IR 6
   LCD_Print(2,9,sen_dat[3],3);	    //IR 7
   LCD_Print(2,13,sen_dat[5],3);	//IR 8
   LCD_Print(1,1,sen_dat[0],3);    	//IR 1
  
 }
}

void forward(void *p)
{
while(1)
{
UpdateLeftPWM(300);	
 UpdateRightPWM(300);
 Forward();
 if((sen_dat[0]<190) || (ADC_Data[0]>160) || (sen_dat[1]<210) || ( ADC_Data[2]>170) || ( sen_dat[2]<170) || (sen_dat[4]<200) || (sen_dat[5]<180) || (sen_dat[5]<180) || (( ADC_Data[2]>60) && (sen_dat[1]<230)) || ((ADC_Data[0]>60) &&(sen_dat[1]<230))||(( ADC_Data[2]>150) && (sen_dat[1]<190) && (ADC_Data[0]>120) ) )
 {
 Stop();
 DelaymSec(30);
 }
 }
}

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

int main()
{
 PINSEL0 = 0x00000000;
 PINSEL1 = 0x00000000;
 //PINSEL2 = 0x00000000;
 Init_Peripherals();
 adc_data();
 DelaymSec(40);
 Init_Peripherals();
 LCD_4Bit_Mode();
 LCD_Init();
 LCD_Command(0x01);
 DelaymSec(15);
 
 
  while(1)
  {
  xTaskCreate(IR1,"IR1",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR1);
  xTaskCreate(IR2,"IR2",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR2);
  xTaskCreate(IR3,"IR3",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR3);
  xTaskCreate(IR4,"IR4",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR4);
  xTaskCreate(IR5,"IR5",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR5);
  xTaskCreate(IR6,"IR6",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR6);
  xTaskCreate(IR7,"IR7",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR7);
  xTaskCreate(IR8,"IR8",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR8);
  xTaskCreate(IR2_IR3,"IR2_IR3",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR2_IR3);
  xTaskCreate(IR3_IR4,"IR3_IR4",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR3_IR4); 
  xTaskCreate(forward,"forward",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xforward);
  xTaskCreate(IR2_IR3_IR4,"IR2_IR3_IR4",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xIR2_IR3_IR4);
  xTaskCreate(lcdprint,"Display",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&xdisplay);
 	
   
	vTaskStartScheduler();  
   	   

  }
}
