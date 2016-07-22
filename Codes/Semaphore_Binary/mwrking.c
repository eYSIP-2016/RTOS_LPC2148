/*
PIN 1 
26 RTCK	 pin 11 center port
27 TDO	 pin 13 center port
28	TDI pin 3 servo pod
29,30
PIN 0
*/
#include<stdlib.h>
#include"FreeRTOS.h"
#include"task.h"
#include"LCD.h"
#include"semphr.h"
#include<math.h>
SemaphoreHandle_t xSemaphore;
unsigned char Temp=0;
unsigned int T;

 unsigned char a[3];
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

void Init_ADC_Pin(void);
void Init_ADC0(void);
void Init_ADC1(void);
unsigned int AD0_Conversion(unsigned char channel);
unsigned int AD1_Conversion(unsigned char channel);
unsigned int Sharp_GP2D12_Estimation(unsigned int Val);
unsigned int Batt_Voltage_Conversion(unsigned int Val);

void send ( int n);
//This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
void  __irq IRQ_UART0(void)
{  
 Temp = U0RBR;			
 
 VICVectAddr = 0x00;
 UART0_SendByte(Temp);	//Echo Back received character
}		


/************************************************************

	Function 		: Init_UART0
	Return type		: None
	Parameters		: None
	Description 	: Initialises UART0 module. 
************************************************************/
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
	Led_init();
	Init_UART0();
	Init_ADC_Pin();
}
void Init_Peripherals(void)
{
 Init_Ports();
 Init_UART0();
 Init_ADC0();
 Init_ADC1();
}

void forward(void *pvparam)
{
	while(1)
	{
	 if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	 	{	 
			Forward();
		  UART0_SendStr("Frwd\n");
		}

	
	}
}							  

void back(void *pvparam)
{	 vTaskDelay(100);
	while(1)
	{
	 if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	 	{	Stop();
			Back();
		 UART0_SendStr("Back\n");
		}

	}
}							  

void control_switcher(void *pvparam)
{  xSemaphoreGive(xSemaphore);
   vTaskDelay(100);
  while(1)
  {
   vTaskDelay(1000);
   xSemaphoreGive(xSemaphore);
   
   
   }
}

//Function to convert t Numbers (Integers) to string and then send the string.
void send ( int n)
{
	int z = 0;
	int c = n;
	a[0] = 0;
	a[1] = 0;
	a[2] = 0;
	while (c)
	{ // loop till there's nothing left
		a[z++] = (char)(c % 10); // assign the last digit
		c /= 10; // "right shift" the number
	}
	
	UART0_SendByte(a[2] + 48);
	UART0_SendByte(a[1] + 48);
	UART0_SendByte(a[0] + 48);
	UART0_SendByte('\n');
	
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