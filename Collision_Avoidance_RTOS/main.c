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

/*******************************************************
ADC Sensor initialization
********************************************************/

void Init_ADC_Pin(void)
{
 PINSEL0&= 0xF0FFC0FF;
 PINSEL0|= 0x0F003F00;		//Set pins P0.4, P0.5, P0.6, P0.12, P0.13 as ADC pins
 PINSEL1&= 0xF0FFFFFF;		
 PINSEL1|= 0x05000000;	    //Set pins P0.28, P0.29 as ADC pins
}

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
 while((AD0GDR&0x80000000)==0);
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
 while((AD1GDR&0x80000000)==0);
 Temp = AD1GDR;						
 Temp = (Temp>>8) & 0xFF;
 return Temp;
}

/*******************************************************
Velocity Control
********************************************************/

void Init_PWM(void)
{
 PINSEL0&=0xFFFF3FFF;
 PINSEL0|=0x00008000;	//Enabling P0.7 as PWM2
 PINSEL1&=0xFFFFF3FF;		
 PINSEL1|=0x00000400;	//Enabling P0.22 as PWM5

 PWMPR	= 30;	//PWM Prescaler PCLK/30 = 500KHz
 PWMPC	= 0;	//PWMPC increments on every PCLK
 PWMTC	= 0;	//PWMTC increments on every PWMPC=PWMPR
 PWMMR0 = 500;	//PWM base frequency 500KHz/500=1KHz	 
 PWMMR1 = 0;
 PWMMR2 = 0;
 PWMMR3 = 0;
 PWMMR4 = 0;
 PWMMR5 = 0;
 PWMMR6 = 0;
 PWMMCR = 0x00000002;
 PWMPCR	= 0x2600;
 PWMLER	= 0x7F;
 PWMTCR = 0x01;
}  


void UpdateLeftPWM(unsigned int vel)
{
 PWMMR2 = vel;
 PWMLER = 0x04;
}

void UpdateRightPWM(unsigned int vel)
{
 PWMMR5 = vel;
 PWMLER = 0x20;
}



/*******************************************************
Motor initialization
********************************************************/


void Init_Motion_Pin(void)
{
 PINSEL0&=0xFF0F3FFF;		
 PINSEL0|=0x00000000;		//Set Port pins P0.7, P0.10, P0.11 as GPIO
 PINSEL1&=0xFFFFF0FF;
 PINSEL1|=0x00000000;		//Set Port pins P0.21 and 0.22 as GPIO
 IO0DIR&=0xFF9FF37F;
 IO0DIR|= (1<<10) | (1<<11) | (1<<21) | (1<<22) | (1<<7); 	//Set Port pins P0.10, P0.11, P0.21, P0.22, P0.7 as Output pins
 IO1DIR&=0xFFDFFFFF;
 IO1DIR|= (1<<21);		// Set P1.21 as output pin				// Stop both the motors on start up
 IO0SET = 0x00200080;	// Set PWM pins P0.7/PWM2 and P0.21/PWM5 to logic 1
}
void L_Forward(void)
{
 IO1SET = 0x00200000;		//Set P1.21 to logic '1'
}

//Function to move Left motor backward
void L_Back(void)
{
 IO0SET = 0x00400000;		//Set P0.22 to logic '1'
}

//Function to move Right motor forward
void R_Forward(void)
{
 IO0SET = 0x00000400;		//Set P0.10 to logic '1'
}

//Function to move Right motor backward
void R_Back(void)
{
 IO0SET = 0x00000800;		//Set P0.11 to logic '1'
}

//Function to stop left motor
void L_Stop(void)
{
 IO1CLR = 0x00200000;		//Set P1.21 to logic '0'
 IO0CLR = 0x00400000;		//Set P0.22 to logic '0'
}

//Function to stop Right motor
void R_Stop(void)
{
 IO0CLR = 0x00000400;		//Set P0.10 to logic '0'
 IO0CLR = 0x00000800;		//Set P0.11 to logic '0'
}

void Stop(void)
{
 L_Stop();
 R_Stop();
}

//Function to move robot in forward direction
void Forward(void)
{
 Stop();
 L_Forward();
 R_Forward();
}

//Function to move robot in backward direction
void Back(void)
{
 Stop();
 L_Back();
 R_Back();
}

//Function to turn robot in Left direction
void Left(void)
{ 
 Stop();
 L_Back();
 R_Forward();
}

//Function to turn robot in right direction
void Right(void)
{ 
 Stop();
 L_Forward();
 R_Back();
}

//Function to turn robot in Left direction by moving right wheel forward
void Soft_Left(void)
{
 Stop();
 R_Forward();
}

//Function to turn robot in right direction by moving left wheel forward
void Soft_Right(void)
{
 Stop();
 L_Forward();
}

//Function to turn robot in left direction by moving left wheel backward
void Soft_Left2(void)
{
 Stop();
 L_Back();
}

//Function to turn robot in right direction by moving right wheel backward 
void Soft_Right2(void)
{
 Stop();
 R_Back();
}

//Function to stop the robot at its current location


void adc_data()
{ 
        //IR 1
   ADC_Data[0] = AD0_Conversion(6);	   //IR 2
   ADC_Data[2] = AD0_Conversion(7);    //IR 4
   
  
   /*sen_dat[6]=MEGA8_ADCRead(7);        //Sharp 1
   sen_dat[6]=Sharp_GP2D12_Estimation(sen_dat[6]);

   sen_dat[7]=MEGA8_ADCRead(14);       //Sharp 5
   sen_dat[7]=Sharp_GP2D12_Estimation(sen_dat[7]);
   */
}


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
if (ADC_Data[0]>170) //IR 2
  {
     Stop();
	 vTaskSuspend(xforward);
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
 if( ADC_Data[2]>170) //IR 4
   {
     Stop();
	 vTaskSuspend(xforward);
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
 if(sen_dat[4]<200) //IR 6
   {
     Stop();
	 vTaskSuspend(xforward);
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
 if(sen_dat[3]<200) //IR 7
   {
     Stop();
	 vTaskSuspend(xforward);
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
	   vTaskResume(xforward);
	 } 
}

}
}

void IR8(void *p)
{
while(1)
{
 UpdateLeftPWM(300);	//Velocity Setting
 UpdateRightPWM(300);	//Velocity Setting
 if(sen_dat[5]<180) //IR 7
   {
     Stop();
	 vTaskSuspend(xforward);
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
	   vTaskResume(xforward);
	 } 
}
}
}

void IR2_IR3(void *p)
{
 while(1)
{
UpdateLeftPWM(300);	//Velocity Setting
UpdateRightPWM(300);	//Velocity Setting
if((ADC_Data[0]>180) &&(sen_dat[1]<180)) //IR 2 and IR 3
   {
     Stop();
	 vTaskSuspend(xforward);
	 Back();
     DelaymSec(100);
     Soft_Left2();
     DelaymSec(100);
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
UpdateLeftPWM(300);	//Velocity Setting
UpdateRightPWM(300);	//Velocity Setting
if(( ADC_Data[2]>150) && (sen_dat[1]<190) && (ADC_Data[0]>120) ) //IR 3 and IR 4
   {
     Stop();
	 vTaskSuspend(xforward);
	 Back();
     DelaymSec(100);
     Soft_Right2();
     DelaymSec(100);
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
UpdateLeftPWM(300);	//Velocity Setting
UpdateRightPWM(300);	//Velocity Setting
if(( ADC_Data[2]>180) && (sen_dat[1]<180)) //IR 3 and IR 4
   {
     Stop();
	 vTaskSuspend(xforward);
	 Back();
     DelaymSec(100);
     Soft_Left2();
     DelaymSec(100);
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