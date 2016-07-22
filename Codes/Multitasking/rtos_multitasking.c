/*
Written by: Kartikeyan  V, K V S Sumakar, ERTS Lab, IIT Bombay. Date: 23 June 2014
These demo codes have been prepared using LPC@21xx's Keil RVDS port
availabale in 'ARM7_LPC2129_Keil_RVDS' file in FreeRTOSv9.0.0
 
 IDE: Keil uVision4
 Tool Chain: Realview MDK_ARM Ver 4.10
 C Compiler: ARMCC.exe


This example is meant to help beginners with FreeRTOS on Fire Bird V. 
It is highly recommended that before starting these one should go through the practice experiments present for ARM-Fire Bird V.

New Concepts covered:	
MultiTasking, Creation of Tasks.


for more details on task  :	http://www.freertos.org/implementing-a-FreeRTOS-task.html

for details about FreeRTOS functions used in the example refer to API documentation on FreeRTOS website.
	:http://www.freertos.org/a00106.html

 Note:
 
 1. Do don't use the default Startup.s	provided by Keil. 
 	Instead follow the procedure given below :
	a. Copy the Startup.S file from the demo folder which you get after downloading the FreeRTOS file.
	b. Right click on the group icon in the project window and click on 'Add files to Group <group name>'
	c. Select and add the Startup.s file which was previously added.

 2. Make sure that in the Target options following settings are 
 	done for proper operation of the code

 	Device -> Microcontroller	: LPC2148
 	Target -> Frequency			: 12 Mhz
 	Output -> Create Hex File	: Checked (For more information read section 4.3.1 "Setting up Project in Keil uVision" in the hardware manual)
	C/C++  -> Include paths 	: ..\..\Source\include;..\..\Source\portable\RVDS\ARM7_LPC21xx;..\<project-folder-name>
	ASM    -> Include paths 	: ..\..\Source\portable\RVDS\ARM7_LPC21xx


 3. Ensure that following settings are done in Startup.s configuration wizard:

 	Clock Settings:
		
	PLL Steup	 >>	 MSEL=5, PSEL=2
	VPBDIV Setup >>  VPBCLK = CPU Clock
	
 4. Include the following FreeRTOS C files in the project either in the same group, or by creating a new group in the project.
	
	 ( These will be available in the FreeRTOS source files. )
	
	FILE		SOURCE (may change according to location of FreeRTOS source code)
	list.c	 	FreeRTOSv8.0.1\FreeRTOS\Source\list.c
	queue.c		FreeRTOSv8.0.1\FreeRTOS\Source\queue.c
	tasks.c		FreeRTOSv8.0.1\FreeRTOS\Source\tasks.c
	heap_2.c	FreeRTOSv8.0.1\FreeRTOS\Source\portable\MemMang\heap_2.c
	port.c		FreeRTOSv8.0.1\FreeRTOS\Source\portable\RVDS\ARM7_LPC21xx\port.c
	portASM.s	FreeRTOSv8.0.1\FreeRTOS\Source\portable\RVDS\ARM7_LPC21xx\portASM.s
	
 5. Copy FreeRTOSConfig.h file into	the project folder. Configure it according to the application. 
 	use this link for reference : http://www.freertos.org/a00110.html
 


*/

/****************************************************************************************************/



#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"


#define DATA_PORT() IO1SET=(1<<19)	
#define READ_DATA() IO1SET=(1<<18)
#define EN_HI() IO1SET=(1<<17)

#define COMMAND_PORT() IO1CLR=(1<<19)	
#define WRITE_DATA() IO1CLR=(1<<18)
#define EN_LOW() IO1CLR=(1<<17)


TaskHandle_t  xTask1Handle,xTask2Handle,xTask3Handle;

void Init_Motion_Pin(void)
{
 PINSEL0&=0xFF0F3FFF;		
 PINSEL0|=0x00000000;		//Set Port pins P0.7, P0.10, P0.11 as GPIO
 PINSEL1&=0xFFFFF0FF;
 PINSEL1|=0x00000000;		//Set Port pins P0.21 and 0.22 as GPIO
 IO0DIR&=0xFF9FF37F;
 IO0DIR|= (1<<10) | (1<<11) | (1<<21) | (1<<22) | (1<<7) | (1<<25); 	//Set Port pins P0.10, P0.11, P0.21, P0.22, P0.7 as Output pins
 IO1DIR&=0xFFDFFFFF;
 IO1DIR|= (1<<21);		// Set P1.21 as output pin				
 IO0SET = 0x00200080;
  IO1DIR|=(1<<25) | (1<<24) | (1<<23) | (1<<22) | (1<<19) | (1<<18) | (1<<17);	// Set PWM pins P0.7/PWM2 and P0.21/PWM5 to logic 1
}

//Stop left motor
void L_Stop(void)
{
 IO1CLR = 0x00200000;		//Set P1.21 to logic '0'
 IO0CLR = 0x00400000;		//Set P0.22 to logic '0'
}

//Stop Right motor
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
//Move Left motor forward
void L_Forward(void)
{
 IO1SET = 0x00200000;		//Set P1.21 to logic '1'
}

//Function to move Left motor backward
void L_Back(void)
{
 IO0SET = 0x00400000;		//Set P0.22 to logic '1'
}

//Move Right motor forward
void R_Forward(void)
{
 IO0SET = 0x00000400;		//Set P0.10 to logic '1'
}

//Move Right motor backward
void R_Back(void)
{
 IO0SET = 0x00000800;		//Set P0.11 to logic '1'
}

//Function to move robot in forward direction
void Forward(void)
{
 Stop();
 L_Forward();
 R_Forward();
}

void BUZZER_ON(void)
{
IO0SET |= (1<<25);
}
void BUZZER_OFF(void)
{
IO0CLR |= (1<<25);
}



//Pin Initialisations
void Init_Ports(void)
{
 Init_LCD_Pin();
 Init_Motion_Pin();
}

void Init_Peripherals(void)
{
 Init_Ports();
}

//Task Functions
void vbuzzer(void *);
void vmotion(void *);
void lcdprint(void *);


// Buzzer Task
void vbuzzer(void *p)
{
while(1)
{
  	  BUZZER_ON();		  
	  vTaskDelay(200); 
	  BUZZER_OFF();
	  vTaskDelay(1200);
}
}

//Motion Task
void vmotion(void *p)
{
while(1)
{
 Forward();
 vTaskDelay(150);
 Stop();
 vTaskDelay(1000);

}
}
 
 //LCD Display Task
void lcdprint(void *p)
{
unsigned char count = 0;//Initialised a variable
while(1)
{
  
  if (count == 100)
  {
	count = 0;
  }
 LCD_Print(1,2,count++,3);  
 vTaskDelay(200);
 
 
 }
}
 


int main ()

{
Init_Peripherals();
 while(1)
{
   LCD_Init();
  
/*If the priorities are same then make the #define  configUSE_TIME_SLICING  0*/ 
xTaskCreate(vbuzzer, "noise", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1,&xTask1Handle);
xTaskCreate(vmotion, "forward", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &xTask2Handle);
xTaskCreate(lcdprint, "display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &xTask3Handle);
                                      /*stack_depth, priority=1 , Null handle. */

vTaskStartScheduler();

}
}



