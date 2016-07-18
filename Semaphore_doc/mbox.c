#include<stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include"semphr.h"

TaskHandle_t xHandle = NULL;


void vnoticer( void * pvParameters )
 {		 
  uint32_t ulNotifiedValue=0x01;

    while(1)
    {	
	
      if( xTaskNotifyWait( 0x00,0xffff,&ulNotifiedValue,1000 )==pdTRUE)  
 {
        
	
		if( ( ulNotifiedValue | 0x01 ) == 0x01 )	   //checking if the received message is same as the sent
  {  UART0_SendStr("Received   MSG from N1 \n");	}

       else if( ( ulNotifiedValue | 0x02 ) == 0x02 )
  {  UART0_SendStr("Received   MSG from N2\n");		}
	   	 
		else if( ( ulNotifiedValue | 0x03 ) == 0x03 )
  {  UART0_SendStr("Received   MSG from N3\n"); 	}
       
       else if( ( ulNotifiedValue | 0x04 ) == 0x04 )
  {   UART0_SendStr("Received   MSG from N4\n");  }

         else
  {   UART0_SendStr("Learn Programming !\n");}
  		
}
		 
else
{ UART0_SendStr("No Notice\n"); }
   
 }	 
}

void vn1( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(4000);
	UART0_SendStr("N1 sent a Message\n");
	xTaskNotify(xHandle, 0x01, eSetBits);
	
	
	}
}
void vn2( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(5000);
	UART0_SendStr("N2 sent a Message\n");
	xTaskNotify(xHandle, 0x02, eSetBits);
	
	
	}
}
void vn3( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(6000);
	UART0_SendStr("N3 sent a MSG\n");
	xTaskNotify(xHandle, 0x03, eSetBits);
	
	
	}
}
void vn4( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(7000);
	UART0_SendStr("N4 sent a MSG\n");
	xTaskNotify(xHandle, 0x04, eSetBits);
	
	
	}
}


  int main()
{	
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 Init_Peripherals();
	
	
	 
	UART0_SendStr("\t\tMailBox using Task Notification\n");

		xTaskCreate(vn1,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn2,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn3,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn4,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
    	xTaskCreate(vnoticer,"Noticer", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
	
		
		
		
		vTaskStartScheduler();  //Task Scheduling   					    
		
	while(1)//Never reaches this Part of the main
	{
	UART0_SendStr("\t\tMailBox Bypassed\n");
	}

}

