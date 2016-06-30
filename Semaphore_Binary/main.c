/*

Replaced contenets in lpc21xx with that of lpc214x to remove warnings 
Include startup.s from experiments in case of data abort
*/

/************************************************************************ 
Written by: K V S SUMAKAR and KARTIKEYAN V, ERTS Lab, IIT Bombay. Date: 23 June 2016
 IDE: Keil uVision4


Problem: To synchronise two Tasks by using Binary Semaphore
		
		 Two Tasks of Forward and Backward motion can be synchronised to use resources bby using another function,
		 The function control_switcher periodically switches between two tasks.
		 This way unlike Mutexes the processor/resources(Motors in this case) wouldn't remain unusedif one task finishes before other task
		 starts waiting for the processor/resources	   

Concepts covered:	
Binary semaphores definition, creation and usage.


 Note:
 
 1. Do don't use the default Startup.s	provided by Keil. 
 	Instead follow the procedure given below :
	a. Copy the Startup.S file from any example project into the folder containing the project.
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
 

	For more details refer section 4.8 in the hardware manual.




Declaration of BinarySemaphore 
	xSemaphore=xSemaphoreCreateBinary( );
	
	
	xSemaphore : Shows if resource is available or not
*/
//****************************************************************************************************/

#include<stdlib.h>
#include"FreeRTOS.h"
#include"task.h"
#include"LCD.h"
#include"semphr.h"
#include<math.h>
SemaphoreHandle_t xSemaphore;
unsigned char Temp=0;
unsigned int T;

 
#define BIT(x) (1<<x)


#define Fosc            12000000                    //10MHz~25MHz
#define Fcclk           (Fosc * 5)                  //Fosc(1~32)<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO Fcclk 2񅦰�16�156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB(Fcclk / 4) 1񄿔
#define  UART_BPS	9600 		//Change Baud Rate Setting here

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
void  __irq IRQ_UART0(void);

void Init_UART0(void);
void UART0_SendByte(unsigned char data);	//For sending a byte of Data through serial monitor
void UART0_SendStr(const unsigned char *str);//For sending a string of Data through serial monitor


//This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
void  __irq IRQ_UART0(void)
{  
 Temp = U0RBR;			
 
 VICVectAddr = 0x00;
 UART0_SendByte(Temp);	//Echo Back received character
}		

//Initialises UART0 module. 
void Init_UART0(void)
{  
   unsigned int Baud16;
   PINSEL0&=0xFFFFFFF0;
   PINSEL0|=0x00000005;

   U0LCR = 0x83;		            // DLAB = 1
   Baud16 = (Fpclk / 16) / UART_BPS;  
   U0DLM = Baud16 / 256;							
   U0DLL = Baud16 % 256;						
   U0LCR = 0x03;
   U0IER = 0x00000001;		//Enable Rx interrupts

   VICIntSelect = 0x00000000;		// IRQ
   VICVectCntl0 = 0x20|6;			// UART0
   VICVectAddr0 = (int)IRQ_UART0; 	//UART0 Vector Address
   VICIntEnable = (1<<6);	// Enable UART0 Rx interrupt

}
				

//This function sends a single character on the serial port
void UART0_SendByte(unsigned char data)
{  
   U0THR = data;				    
   while( (U0LSR&0x40)==0 );	    
}

//This function sends a string of characters on the serial port
void UART0_SendStr(const unsigned char *str)
{  
   while(1)
   {  
      if( *str == '\0' ) break;
      UART0_SendByte(*str++);	    
   }
}

void DelaymSec(unsigned int j)		//Generates milli second delay
{  
 unsigned int  i;
 for(;j>0;j--)
 {
  for(i=0; i<10000; i++);
 } 
}
/*

PIN 1 
26 RTCK	 pin 11 center port
27 TDO	 pin 13 center port
28	TDI pin 3 servo pod
29,30
PIN 0

void Led_init_P1(int x)
{
   IO1DIR|=BIT(x);
}
void Led_on_P1(int x)
{
   IO1SET|=BIT(x);
}
void Led_off_P1(int x)
{
   IO1CLR|=BIT(x) ;
}

void Led_init()
{   
	Led_init_P1(26);
	Led_init_P1(27);
	Led_init_P1(28);
}
*/
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
{ 	Init_Motion_Pin();
//	Led_init();
	Init_UART0();
	
}
void Init_Peripherals(void)
{
 Init_Ports();
 Init_UART0();
 
}

void forward(void *pvparam)
{	 vTaskDelay(5);	//Added so that Back Task can occupy the resource first
	while(1)
	{
	 if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	 	{	Stop(); 
			Forward();
		  UART0_SendStr("Forward\n");
		vTaskDelay(5);//To avoid same Tasking Taking resources twice
		}

	
	}
}							  

void back(void *pvparam)
{	 //xSemaphoreGive(xSemaphore);
	while(1)
	{
	 if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	 	{	Stop();
			Back();
		 UART0_SendStr("Back\n");
		vTaskDelay(5);
		}

	}
}							  
/*
Synchronices The Tasks by "Giving" the semaphore periodically 
therefore no one Task occupies the resources for long time and 
at the same time doesn't have to release the resources simultaneously
*/
void control_switcher(void *pvparam)
{     
  while(1)
  {xSemaphoreGive(xSemaphore);
   UART0_SendStr("Semaphore given\n");
   vTaskDelay(1200);
   
   
   }
}


  int main()
{	
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 DelaymSec(40);
 	Init_Peripherals();
	 
	UART0_SendStr("\t\tBinary Semaphore\n");
    xSemaphore=xSemaphoreCreateBinary( );
		
	xTaskCreate(forward,"forward", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
	xTaskCreate(back,"back", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
	xTaskCreate(control_switcher,"control_switcher", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
	
	vTaskStartScheduler();  //Task Scheduling   					    
	
	while(1);


}	