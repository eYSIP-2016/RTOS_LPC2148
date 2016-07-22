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
	  vTaskDelay(200);
}
}

//Motion Task
void vmotion(void *p)
{
while(1)
{
 Forward();
 vTaskDelay(250);


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
   LCD_Init();//LCD is initialised
  
/*If the priorities are same then make the #define  configUSE_TIME_SLICING  0*/
/*3 Tasks are created in the following function*/ 
xTaskCreate(vbuzzer, "noise", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1,&xTask1Handle);
xTaskCreate(vmotion, "forward", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &xTask2Handle);
xTaskCreate(lcdprint, "display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &xTask3Handle);
                                      /*stack_depth, priority=1 , Null handle. */


vTaskStartScheduler();

}
}

