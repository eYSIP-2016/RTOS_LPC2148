//Header files
// SPI communication
#define	SPI1_SLAVE_SELECT	0x00100000

#define SENSOR_OFF() IO1SET=(1<<16)		//Macro to turn OFF Sensors
#define SENSOR_ON() IO1CLR=(1<<16)		//Macro to turn ON Sensors 
unsigned char sen_dat[17];
int i=0;

BYTE MEGA8_ADCRead(BYTE channel);


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
//This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
void vsend(void *pvparam)
{  
 
 while(1)
 {
  UART1_SendStr(sen_dat);
  UART1_SendByte(0x00);
  UART1_SendByte(0xFF);
  vTaskDelay(1000);	   
  
 }
}
void vcalc(void *pvparam)
{  
 
 while(1)
 {
   sen_dat[0]=MEGA8_ADCRead(6);		// IR 1
   sen_dat[1]=MEGA8_ADCRead(13);	// IR 2
   sen_dat[2]=MEGA8_ADCRead(9);		// IR 3
   sen_dat[3]=MEGA8_ADCRead(8);		// IR 4
   sen_dat[4]=MEGA8_ADCRead(15);	// IR 5
   sen_dat[5]=MEGA8_ADCRead(4);	// IR 6
   sen_dat[6]=MEGA8_ADCRead(0);	// IR 7
   sen_dat[7]=MEGA8_ADCRead(7);	//sharp 1
   sen_dat[8]=AD0_Conversion(6);	 //sharp 2
   sen_dat[9]=AD1_Conversion(0);	 //sharp 3
   sen_dat[10]=AD0_Conversion(7);	 //sharp 4
   sen_dat[11]=MEGA8_ADCRead(14);	 //sharp 5
   sen_dat[12]=AD1_Conversion(3);	 //WL left
   sen_dat[13]=AD0_Conversion(1);	 //WL center
   sen_dat[14]=AD0_Conversion(2);	 //WL right
  	
   vTaskDelay(1000);
 }
}		
void vline(void *pvparam)
{
while(1)
{
UpdateVelocity(400,400);
Forward();
}
}

//Initialise Ports and pheripherals

int main()
{
 PINSEL0 = 0x00000000;
 PINSEL1 = 0x00000000;
 //PINSEL2 = 0x00000000;
 Init_Peripherals();

  
 xTaskCreate(vline,"line", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
 xTaskCreate(vcalc,"calc", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
 xTaskCreate(vsend,"send", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
 vTaskStartScheduler();

  while(1);
}