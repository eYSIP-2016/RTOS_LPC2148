/*//include uart files
//servo codes idk

unsigned char state = 0x00;//IDLE;	( for switch cases in UART ISR) i dont know i put it 0x00

*/
#include <LPC214X.H> /* LPC214x definitions */
#include "type.h"
#include "spi.h"
#include "LCD.h"
#include "pwm.h"
#include "motion_control.h"
//#include "uart.h"
#include "adc.h"  

unsigned char Temp=0,Temp2=0;
unsigned char battery_voltage=0;

extern volatile unsigned int Left_Shaft_Count;
 extern volatile unsigned int Right_Shaft_Count;

#define SERVO_COMMAND_START	21						// command for first servo starts from this number
#define SERVO_COUNT	20
#define PAN_TILT_SERVO_MIN	0x15
#define PAN_TILT_SERVO_MAX	0x65

// SPI communication
#define	SPI1_SLAVE_SELECT	0x00100000

#define BUZZER_OFF() IO0CLR=(1<<25)		   //Macro to turn OFF buzzer
#define BUZZER_ON() IO0SET=(1<<25)		   //Macro to turn ON buzzer
#define SENSOR_OFF() IO1SET=(1<<16)		//Macro to turn OFF Sensors
#define SENSOR_ON() IO1CLR=(1<<16)		//Macro to turn ON Sensors 


#define Fosc            12000000                    //10MHz~25MHz
#define Fcclk           (Fosc * 5)                  //Fosc(1~32)<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO Fcclk 2???6?56MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB(Fcclk / 4) 1??
#define  UART_BPS	115200 		//Change Baud Rate Setting here

void Init_UART0(void);
void  __irq IRQ_UART0(void);
void UART0_SendByte(unsigned char data);
void UART0_SendStr(const unsigned char *str);

void Init_UART1(void);
void  __irq IRQ_UART1(void);
void UART1_SendByte(unsigned char data);
void UART1_SendStr(const unsigned char *str);

void Ext_INT_Setup(void);
 void  __irq IRQ_Eint0(void);
 void  __irq IRQ_Eint3(void);
 
unsigned char rec_data = 0;						// single byte received at UDR2 is stored in this variable 
unsigned char uart_data_buff[25] = {0};			// storing uart data in this buffer
unsigned char copy_packet_data[25] ;//= {0x01,0x01,0x00,0x00};		// storing uart data into another packet data for operation
unsigned char state = 0x00;//IDLE;						// for switch cases in UART ISR
unsigned char end_char_rec = 0; 
unsigned char i = 0 , j = 0;					//
unsigned char data_packet_received = 1;			// flag to check if all data_packet is received- goes high when '\!' is received
unsigned char data_copied = 1;					// flag to check if uart_data_buff is copied into packet_data

unsigned char device_id = 01;
unsigned char device_type = 01;
unsigned char function_type = 0;
unsigned char param_count = 0;
unsigned char param_count_upper_nibbel;
unsigned char param_count_lower_nibbel;
unsigned char temp_1;							//for data > 8 bit
unsigned char temp_2;							//for data > 8 bit
unsigned int param_1 = 0, param_2 = 0, param_3 = 0;

volatile unsigned long int ShaftCountLeft = 0; //to keep track of left position encoder
volatile unsigned long int ShaftCountRight = 0; //to keep track of right position encoder
volatile unsigned int Degrees; //to accept angle in degrees for turning

unsigned char data;			//to store received data from UDR1
unsigned char ADC_flag;
unsigned char left_motor_velocity = 0x00;
unsigned char right_motor_velocity = 0x00;

void Init_Sensor_Switch_Pin(void)
{
 IO1DIR&=0xFFFEFFFF;
 IO1DIR|=(1<<16);   //Set P1.16 direction as Output
 SENSOR_OFF();		   //Initially turn OFF Sensor
}

void Init_Buzzer_Pin(void)
{
 PINSEL1&=0xFFF3FFFF;		
 PINSEL1|=0x00000000; 		//Set P0.25 as GPIO
 IO0DIR&=0xFDFFFFFF;
 IO0DIR|= (1<<25);  		//Set P0.25 as Output
 BUZZER_OFF();				//Initially turn OFF buzzer
}

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

void  __irq IRQ_UART0(void)
{
	rec_data = U0RBR; 				//making copy of data from U0THR in 'data' variable
 	VICVectAddr = 0x00;
	//while(!(UCSR2A && (1<<RXC2)));	// wait till data byte is received
	
	if (data_packet_received == 0) 
	{
		if (rec_data == '\n' )			// '\n' decimal value is 10
		{
			 //state = _second_last_byte 
			uart_data_buff[i] = rec_data;
			i++;
			end_char_rec = 1;
		//	U0THR = rec_data;
		}

		else 
		{
			if((end_char_rec == 1) && (rec_data == '\r'))		//'\r' indicates end of transmission. It should come after '\n'
			{
				uart_data_buff[i] = rec_data;
				i++;
				end_char_rec = 2;
				data_packet_received = 1;
				
				for (j = 0;j<i;j++)				// i value is stored in ISR
				{
					copy_packet_data[j] = uart_data_buff[j];
					//U0THR = copy_packet_data[j];
					uart_data_buff[j] = 0;
				}
			//	U0THR = rec_data;
			}
	
			else if((end_char_rec == 1) && (rec_data != '\r'))		//'\r' is expected after '\n'. If not received, discard the data. 
			{
			//	U0THR = 'x';
																	// discard the data and check 
			}
		
			else													// store other data bytes
			{
				uart_data_buff[i] = rec_data;
				i++;
			//	U0THR = rec_data;
			}
		}
	}

}	// end of ISR

//Edited for LPC2148
void send_sensor_data(void)
{//		battery_voltage =  (unsigned char)(((AD1_Conversion(4); * 2) / 3) + 0.5);
	if (device_id == 0x00)
	{
		if (function_type == 0x00)
		{
			U0THR = battery_voltage	;	// Battery Voltage
		}
		
		else if (function_type == 0x01)
		{
			U0THR = AD0_Conversion(2);		// right WL sensor
		}
		
		else if (function_type == 0x02)
		{
			U0THR = AD0_Conversion(1);		// Center WL sensor
		}
		
		else if (function_type == 0x03)
		{
			U0THR = AD1_Conversion(3);		// left WL sensor
		}
		
		else if (function_type == 0x04)
		{
			U0THR = MEGA8_ADCRead(6);		// IR Proximity sensor-1
		}
		
		else if (function_type == 0x05)
		{
			U0THR = MEGA8_ADCRead(13);		// IR Proximity sensor-2
		}
		
		else if (function_type == 0x06)
		{
			U0THR = MEGA8_ADCRead(9);		// IR Proximity sensor-3
		}
		
		else if (function_type == 0x07)
		{
			U0THR = MEGA8_ADCRead(8);		// IR Proximity sensor-4
		}
		
		else if (function_type == 0x08)
		{
			U0THR = MEGA8_ADCRead(15);		// IR Proximity sensor-5
		}
		
		else if (function_type == 0x09)
		{
			U0THR = MEGA8_ADCRead(7);		// Sharp Sensor-1
		}

		else if (function_type == 0x0A)
		{
			U0THR = AD0_Conversion(6);		// Sharp Sensor-2
		}

		else if (function_type == 0x0B)
		{
			U0THR = AD1_Conversion(0);		// Sharp Sensor-3
		}

		else if (function_type == 0x0C)
		{
			U0THR = AD0_Conversion(7);		// Sharp Sensor-4
		}

		else if (function_type == 0x0D)
		{
			U0THR = MEGA8_ADCRead(14);		// Sharp Sensor-5
		}

		
	}
	
	if (device_id == 0x01)				// ATmega 8 ADC
	{
		if (function_type == 0x00)					// Whiteline sensor 4
		{
			U0THR = 0x42;//spi_master_tx_and_rx(0);
		} 
		
		else if (function_type == 0x01)				// Whiteline sensor 5
		{
			U0THR = 0x42;//spi_master_tx_and_rx(1);
		}
		
		else if (function_type == 0x02)				// Whiteline sensor 6
		{
			U0THR = 0x42;//spi_master_tx_and_rx(2);
		}
		
		else if (function_type == 0x03)				// Whiteline sensor 7
		{
			U0THR = 0x42;//spi_master_tx_and_rx(3);
		}
		
		else if (function_type == 0x04)				// Current Sensor
		{
			U0THR = 0x42;//spi_master_tx_and_rx(4);
		}
		
		else if (function_type == 0x05)				// IR proximity sensor 6
		{
			U0THR = MEGA8_ADCRead(5);
		}
		
		else if (function_type == 0x06)				// IR proximity sensor 7
		{
			U0THR = MEGA8_ADCRead(4);
		}
		
		else if (function_type == 0x07)				// IR proximity sensor 8
		{
			U0THR = MEGA8_ADCRead(0);
		}
	}
					    
   while( (U1LSR&0x40)==0 );
				
}
//edited for LPC2148
void actuate_devices(void)
{
	if (device_id == 0x01)				// Buzzer has device id = 1
	{
		if (function_type == 0x00)
		{
			BUZZER_ON();
		}
		else if (function_type == 0x01)
		{
			BUZZER_OFF();
		}
	}
	
	if (device_id == 0x02)				// Motor has device id = 2
	{
		if (function_type == 0x00)
		{
			//motor_enable();
			Forward();
		}
		else if (function_type == 0x01)
		{
			//motor_enable();
			Back();
		}
		else if (function_type == 0x02)
		{
			//motor_enable();
			Right();
		}
		else if (function_type == 0x03)
		{
			//motor_enable();
			Left();
		}
		else if (function_type == 0x04)
		{
			//motor_enable();
			Stop();
		}
		else if (function_type == 0x09)
		{
			Forward();
			//U0THR = param_1;
			UpdateVelocity(param_1,param_2);
			
		}	
	}

	if (device_id == 0x03)				// position encoder has device id = 3
	{
		if (function_type == 0x00)
		{
			Forward_mm(param_1);
		}
		
		if (function_type == 0x01 )
		{
			Back_mm(param_1);
		}
	}

	if (device_id == 0x04)				// servo has device id = 4
	{
		if (function_type == 0x00)
		{
			//servo_1(param_1);
		}
		
		if (function_type == 0x01 )
		{
			//servo_2(param_1);
		}
		
		if (function_type == 0x02 )
		{
		//	servo_3(param_1);
		}

		if (function_type == 0x03 )
		{
		//	servo_1_free();
		}

		if (function_type == 0x04 )
		{
		//	servo_2_free();
		}

		if (function_type == 0x05 )
		{
		//	servo_3_free();
		}
	}
	if (device_id == 0x05)
	{
		if (function_type == 0x00)
		{
		//	LED_bargraph_on(param_1);
		}

		if (function_type == 0x01)
		{
		//	LED_bargraph_off(param_1);
		}
	}
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
void Ext_INT_Setup(void)
{
 //PINSEL0&= 0x3FFFFFFF;
 //PINSEL0|= 0x80000000; //Enabling P0.15 as EINT2
 PINSEL1&= 0xCFFFFFFC;
 PINSEL1|= 0x20000001; //Enabling P0.16 as EINT0  and P0.30 as EINT3


 EXTMODE = 0x09;	// EINT3 and EINT0 is edge sensitive
 EXTPOLAR = 0x00;					// EINT3 and EINT0 in triggered on falling edge

 VICIntSelect = 0x00000000;		// Setting EINT2 and EINt0 as IRQ(Vectored)
 VICVectCntl0 = 0x20|17;		// Assigning Highest Priority Slot to EINT3 and enabling this slot
 VICVectAddr0 = (int)IRQ_Eint3; // Storing vector address of EINT2

 VICVectCntl1 = 0x20|14;		// Assigning second Highest Priority Slot to EINT0 and enabling this slot
 VICVectAddr1 = (int)IRQ_Eint0; // Storing vector address of EINT0
 EXTINT = 0x09;	//Setting EINT2 & EINT0 interrupt flag
 	
 VICIntEnable = (1<<17) | (1<<14);	// Enable EINT3	& EINT0 flags
}

//ISR for EINT0
void  __irq IRQ_Eint0(void)
{  
   Right_Shaft_Count++;
   EXTINT = 0x01;				// Clear EINT0 flag
   VICVectAddr = 0x00;   		//Acknowledge Interrupt
}	


//ISR for EINT2
void  __irq IRQ_Eint3(void)
{  
   Left_Shaft_Count++;
   EXTINT = 0x08;				// Clear EINT2 flag
   VICVectAddr = 0x00;   		//Acknowledge Interrupt
}	

 //This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
 /*
void Init_UART1(void)
{  
   unsigned int Baud16;
   PINSEL0&=0xFFF0FFFF;
   PINSEL0|=0x00050000;

   U1LCR = 0x83;		            // DLAB = 1
   Baud16 = (Fpclk / 16) / UART_BPS;  
   U1DLM = Baud16 / 256;							
   U1DLL = Baud16 % 256;						
   U1LCR = 0x03;
   U1IER = 0x00000001;		//Enable Rx interrupts

   VICIntSelect = 0x00000000;		// IRQ
   VICVectCntl0 = 0x20|7;			// UART1
   VICVectAddr0 = (int)IRQ_UART1; 	//UART1 Vector Address
   VICIntEnable = (1<<7);	// Enable UART1 Rx interrupt

}
				
//This function sends a single character on the serial port
void UART1_SendByte(unsigned char data)
{  
   U0THR = data;				    
   while( (U1LSR&0x40)==0 );	    
}
void UART1_SendByte_int(int data)
{  
   U0THR = data;				    
   while( (U1LSR&0x40)==0 );	    
}

/*void  __irq IRQ_UART1(void)
{  
 Temp2 = U0RBR;			
  
 
 //write code here
 VICVectAddr = 0x00;
 UART1_SendByte(Temp2);	//Echo Back received character

} */

void decode_data(void)
{
	while (data_copied == 1)
	{
		device_id = copy_packet_data[0];
		device_type = copy_packet_data[1];
		function_type = copy_packet_data[2];
		param_count = copy_packet_data[3];
		param_count_upper_nibbel = param_count & 0x10;
		param_count_lower_nibbel = (unsigned char)(param_count & 0x0F);
		
		if ((param_count & 0x10) == 0x10)
		{
			temp_1 = copy_packet_data[4];
			temp_2 = copy_packet_data[5];
			
			param_1 = 256*temp_2 + temp_1;	
		}
		
		else
		{
			switch (param_count_lower_nibbel)
			{
				case 1:
				param_1 = copy_packet_data[4];
				break;
				case 2:
				param_1 = copy_packet_data[4];
				param_2 = copy_packet_data[5];
				break;
				default:
				break;
			}
		}
		data_copied = 0;
	U0THR = 'D';
	}
	
	if ((data_copied == 0) && (device_type == 0x00))	// input devices such as sensors, which will send back data
	{
		send_sensor_data();
	}
	
	else if ((data_copied == 0) && (device_type == 0x01)) // output devices such as buzzer, motors
	{
	U0THR = 'A';
		actuate_devices();
	}
}

//This function sends a string of characters on the serial port
/*
void UART1_SendStr(const unsigned char *str)
{  
   while(1)
   {  
      if( *str == '\0' ) break;
      UART1_SendByte(*str++);	    
  
   }


} */
void copy_data_packet()
{
	if (data_packet_received == 1)
	{
		
		//for (j = 0;j<i;j++)				// i value is stored in ISR
		//{
			//copy_packet_data[j] = uart_data_buff[j];
			////UDR2 = copy_packet_data[j];
			//uart_data_buff[j] = 0;
		//}
		i=0;
		j=0;
		data_packet_received = 0;
		end_char_rec = 0;
		data_copied = 1;
		
	U0THR = data_copied;
		decode_data();
		U0THR = 'I';
		//_delay_ms(1000);
	}
	U0THR = 'O';
	
}

void Init_Ports(void)
{
 Init_Motion_Pin();
 Init_Buzzer_Pin();
 Init_ADC_Pin();
 Init_LCD_Pin();
}


void Init_Peripherals(void)
{
 Init_Ports();
 Ext_INT_Setup();
 Init_Motion_Pin();
 Init_Sensor_Switch_Pin();
 Init_ADC0();
 Init_ADC1();
 Init_PWM();
 Init_UART0();
 //Init_UART1();
 SPI1_Init();
}


int main()
{
 PINSEL0 = 0x00000000;
 PINSEL1 = 0x00000000;
 //PINSEL2 = 0x00000000;	 
Init_Peripherals();
UART0_SendByte('a');
while(1)
	{
	copy_data_packet();
	}
}
