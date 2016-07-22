#include<stdlib.h>
#include"FreeRTOS.h"
#include"task.h"
#include"LCD.h"
#include"semphr.h"

SemaphoreHandle_t xSemaphore=0;

void DelaymSec(unsigned int j);
void Forward(void);
void Back(void);
void Left(void);
void Right(void);
void Stop(void);
void Soft_Left(void);
void Soft_Right(void);
void Soft_Left2(void);
void Soft_Right2(void);
void L_Forward(void);
void L_Back(void);
void R_Forward(void);
void R_Back(void);
void L_Stop(void);
void R_Stop(void);
void Init_Motion_Pin(void);
void Init_Peripherals(void);
void Init_Ports(void);

void DelaymSec(unsigned int j)		//Generates milli second delay
{  
 unsigned int  i;
 for(;j>0;j--)
 {
  for(i=0; i<10000; i++);
 } 
}


void Init_Motion_Pin(void)
{
 PINSEL0&=0xFF0F3FFF;		
 PINSEL0|=0x00000000;		//Set Port pins P0.7, P0.10, P0.11 as GPIO
 PINSEL1&=0xFFFFF0FF;
 PINSEL1|=0x00000000;		//Set Port pins P0.21 and 0.22 as GPIO
 IO0DIR&=0xFF9FF37F;
 IO0DIR|= (1<<10) | (1<<11) | (1<<21) | (1<<22) | (1<<7); 	//Set Port pins P0.10, P0.11, P0.21, P0.22, P0.7 as Output pins
 IO1DIR&=0xFFDFFFFF;
 IO1DIR|= (1<<21);		// Set P1.21 as output pin
 Stop();				// Stop both the motors on start up
 IO0SET = 0x00200080;	// Set PWM pins P0.7/PWM2 and P0.21/PWM5 to logic 1
}

void L_Forward(void)
{
 IO1SET = 0x00200000;		//Set P1.21 to logic '1'
}
void L_Back(void)
{
 IO0SET = 0x00400000;		//Set P0.22 to logic '1'
}
void R_Forward(void)
{
 IO0SET = 0x00000400;		//Set P0.10 to logic '1'
}
void R_Back(void)
{
 IO0SET = 0x00000800;		//Set P0.11 to logic '1'
}
void L_Stop(void)
{
 IO1CLR = 0x00200000;		//Set P1.21 to logic '0'
 IO0CLR = 0x00400000;		//Set P0.22 to logic '0'
}
void R_Stop(void)
{
 IO0CLR = 0x00000400;		//Set P0.10 to logic '0'
 IO0CLR = 0x00000800;		//Set P0.11 to logic '0'
}
void Forward(void)
{
 Stop();
 L_Forward();
 R_Forward();
}
void Back(void)
{
 Stop();
 L_Back();
 R_Back();
}
void Left(void)
{ 
 Stop();
 L_Back();
 R_Forward();
}
void Right(void)
{ 
 Stop();
 L_Forward();
 R_Back();
}
void Soft_Left(void)
{
 Stop();
 R_Forward();
}
void Soft_Right(void)
{
 Stop();
 L_Forward();
}
void Soft_Left2(void)
{
 Stop();
 L_Back();
}
void Soft_Right2(void)
{
 Stop();
 R_Back();
}

void Stop(void)
{
 L_Stop();
 R_Stop();
}
void Init_Ports()
{  Init_Motion_Pin();
	Init_LCD_Pin();
}
void forward(void *pvparam)
{
	while(1)
	{													  // check if semaphore is available
		
		if(xSemaphoreTake(xSemaphore,1000) == pdTRUE )	  // if available then 
		{
			Forward();
			LCD_Print_String(1,1,"Forward");		 // user 1 has access over the shared data
			DelaymSec(3000);			// perform other shared  data tasks();
			xSemaphoreGive( xSemaphore );		    // after shared data task completed, return the semaphore
		}
		else
		{
		    LCD_Print_String(1,1,"SemB");		 // if semaphore not taken in 1000ms then error
		}
		//vTaskDelay(1000);							// try again after delay of 1000ms
	}
}							  

void back(void *pvparam)
{
	while(1)
	{												   // check if semaphore is available
		if(xSemaphoreTake(xSemaphore,1000) == pdTRUE )	   // if available then 
		{
			LCD_Print_String(2,1,"Back");	   // user 2 has access over the shared data
			Back();
			DelaymSec(5000);			// perform other shared  data tasks();
			xSemaphoreGive( xSemaphore );			   // after shared data task completed, return the semaphore
		}
		else
		{
		    LCD_Print_String(2,1,"SemF");	   // if semaphore not taken in 1000ms then error
		}
		vTaskDelay(1000);						// try again after delay of 1000ms
	}
}


int main()
{	PINSEL0 = 0x00000000;		// Enable GPIO on all pins
 	PINSEL1 = 0x00000000;
 	PINSEL2 = 0x00000000;
LCD_4Bit_Mode();
 LCD_Init();
 LCD_Command(0x01);
 
	Init_Ports();

	xSemaphore = xSemaphoreCreateMutex();		  

	xTaskCreate(forward,"forward", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);	
	xTaskCreate(back,"back", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);

	vTaskStartScheduler();     					    
	
	while(1);
}
