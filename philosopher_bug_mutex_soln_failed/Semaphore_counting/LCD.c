#include  <lpc214x.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lcd.h"

static QueueHandle_t Queue_Handle_Data =0, Queue_Handle_String=0;

void Init_LCD_Pin(void);
void Delay(unsigned int j);
void LCD_Write(unsigned int data4);
void LCD_Command(unsigned int data);
void LCD_4Bit_Mode(void);
void LCD_Init(void);
void LCD_String( char *data);
void LCD_Home(void);
void LCD_Cursor(unsigned char Row,unsigned char Col);
void LCD_Print(unsigned char Row, char Col,unsigned int Val, unsigned int Digits);
void LCD_Print_String(unsigned char Row, char Col, char *data);

/* 
function to print a string of maximum size 16(size of LCD)
This function encapsulates the usage of queues for the front user. 
The user is using queues without the hassle of queue initialization and usage.
For the front user, this function is similar to the function previously created 
****void LCD_Print_String(unsigned char, char Col, char *data);*** 

*/
void print_LCD_String_using_queue(unsigned char row, unsigned char col,char *print_data);

/* 
function to print an unsigned char data
This function encapsulates the usage of queues for the front user. 
The user is using queues without the hassle of queue initialization and usage.
For the front user, this function is similar to the function previously created 
****void LCD_Print(unsigned char Row, char Col,unsigned int Val, unsigned int Digits);
It is meant to replace the above function so as to provide reentrancy in the code 
*/
void print_LCD_Data_using_queue(unsigned char row, unsigned char col, unsigned int print_data, unsigned int Digits)	;


/*
This task is waiting on the queue 'Queue_Handle_String' for a data of type string to be written on it.
As soon as data is written in to the queue, this task is removed from the blocked state  
and it prints the string on the LCD. 
*/

void vLCD_Display_String(void *pvparam);

/*
This task is waiting on the queue 'Queue_Handle_Data' for data to be written on it.
As soon as data is written in the queue, this task is removed from the blocked state 
and it prints the data on the LCD. 
*/
void vLCD_Display_Data(void *pvparam) ;

/*
this structure is used to pass data of type String along with other parameters(row,col) from the the
'print_LCD_String_using_queue' function  on the queue 'Queue_Handle_String' which is read by the task 
'vLCD_Display_String'
*/

typedef struct
{
unsigned char Row;
unsigned char Col;
char Print_Data[16];
} print_LCD_String;

/*
this structure is used to pass data of type unsigned int, along with other parameters(row,col,digits) from the the
'print_LCD_Data_using_queue' function  on the queue 'Queue_Handle_Data' which is read by the task 
'vLCD_Display_Data'
*/

typedef struct
{
unsigned char Row;
unsigned char Col;
unsigned int Print_Data;
unsigned int Digits;
} print_LCD_Data;


void print_LCD_String_using_queue(unsigned char row, unsigned char col,char *print_data)
{
	print_LCD_String u, *p; 
	u.Row=row;
   	u.Col=col;
	strcpy(u.Print_Data,print_data); 
	p=&u;													  
	xQueueSend(Queue_Handle_String,(void *)&p,1000);
}

void print_LCD_Data_using_queue(unsigned char row, unsigned char col, unsigned int print_data, unsigned int Digits)
{
	print_LCD_Data u, *p; 
	u.Row=row;
	u.Col=col;
	u.Digits=Digits;
	u.Print_Data=print_data;
	p=&u;												  
	xQueueSend(Queue_Handle_Data,(void *)&p,1000);
}

void vLCD_Display_Data(void *pvparam)
{
	print_LCD_Data *y_data;
	while(1)
	{
	   if(xQueueReceive(Queue_Handle_Data,&y_data,500))
	   {
			LCD_Print(y_data->Row,y_data->Col,y_data->Print_Data,y_data->Digits);
	   }
	}
}

void vLCD_Display_String(void *pvparam)
{
	print_LCD_String *y_string;
	while(1)
	{

	   if(xQueueReceive(Queue_Handle_String,&y_string,500))
	   {
			LCD_Print_String(y_string->Row,y_string->Col,y_string->Print_Data);
	   }
	}
}



void Init_LCD_Pin(void)
{								
 IO1DIR|= LCD_IO; 	// Set P1.17, P1.18, P1.19 P1.22, P1.23, P1.24, P1.25   as Output		
}

//This Function provides delay of app. 1mSec
void Delay(unsigned int j)		  // delay
{  
 unsigned int  i;
 for(;j>0;j--)
 {
  for(i=0; i<500; i++);
 } 
}

//This function writes the data on the data PORT using IOSET and IOCLR
void LCD_Write(unsigned int data4)
{
  IO1PIN&= ~DATA_BUS;
  IO1PIN|=(data4) << 22;
}
//This function sends commands to LCD
void LCD_Command(unsigned int data8)
{
 EN_LOW();
 COMMAND_PORT();
 WRITE_DATA();
 
 LCD_Write((data8&0xF0)>>4);
 
 EN_HI();
 Delay(1);
 EN_LOW();
 
 LCD_Write((data8&0x0F));
 
 EN_HI();
 Delay(1);
 EN_LOW();
 
} 

//This Functions switches LCD panel in 4-bit interfacing mode
void LCD_4Bit_Mode(void)
{
 EN_LOW();
 COMMAND_PORT();
 WRITE_DATA();

 LCD_Write(0x03);

 EN_HI();
 Delay(5);
 EN_LOW();
 Delay(5);

 LCD_Write(0x03);
 
 EN_HI();
 Delay(5);
 EN_LOW();
 Delay(5);

 LCD_Write(0x03);
 
 EN_HI();
 Delay(5);
 EN_LOW();
 Delay(5);

 LCD_Write(0x02);
 
 EN_HI();
 Delay(5);
 EN_LOW();
 Delay(5);

} 


//This function writes data on the LCD display
void LCD_Data(unsigned int data8)
{
 EN_LOW();
 DATA_PORT();
 WRITE_DATA();
 
 LCD_Write((data8&0xF0)>>4);

 EN_HI();
 Delay(1);
 EN_LOW();
 
 LCD_Write((data8&0x0F));

 EN_HI();
 Delay(1);
 EN_LOW();
}


//This function enables LCD's internal funcions
void LCD_Init(void)
{
 Init_LCD_Pin();
 LCD_4Bit_Mode();
 LCD_Command(0x28);
 LCD_Command(0x0C);
 LCD_Command(0x06);
 LCD_Command(0x01);
 Delay(20);
// LCD_Queue_Handle = xQueueCreate(10,sizeof(print_LCD_String *));
Queue_Handle_Data = xQueueCreate(10,sizeof(print_LCD_Data *));
Queue_Handle_String = xQueueCreate(10,sizeof(print_LCD_String *));
}

//This function writes string on LCD display
void LCD_String( char *data)
{
 while(*data)
 {
  LCD_Data(*data);
  data++;
 } 
}  


//This Function puts LCD cursor to Home position
void LCD_Home(void)
{
 LCD_Command(0x80);
}

//This function sets cursor position
void LCD_Cursor(unsigned char Row,unsigned char Col)
{
 switch(Row)
 {
  case 1: LCD_Command(0x80+ Col -1);break;
  case 2: LCD_Command(0xC0+ Col -1);break;
  default: break;
 }
}


//This function displays any data upto 5 digits. It also requires row and column address
void LCD_Print(unsigned char Row, char Col,unsigned int Val, unsigned int Digits)
{
 unsigned char Flag=0;
 unsigned int Temp,Mi,Th,Hu,Te,Un=0;

 if(Row==0 || Col==0)
 {
  LCD_Home();
 }
 else
 {
  LCD_Cursor(Row,Col);
 }
 if(Digits==5 || Flag==1)
 {
  Mi=Val/10000+48;
  LCD_Data(Mi);
  Flag=1;
 }
 if(Digits==4 || Flag==1)
 {
  Temp = Val/1000;
  Th = (Temp % 10) + 48;
  LCD_Data(Th);
  Flag=1;
 }
 if(Digits==3 || Flag==1)
 {
  Temp = Val/100;
  Hu = (Temp%10) + 48;
  LCD_Data(Hu);
  Flag=1;
 }
 if(Digits==2 || Flag==1)
 {
  Temp = Val/10;
  Te = (Temp%10) + 48;
  LCD_Data(Te);
  Flag=1;
 }
 if(Digits==1 || Flag==1)
 {
  Un = (Val%10) + 48;
  LCD_Data(Un);
 }
 if(Digits>5)
 {
  LCD_Data('E');
 }
	
}

void LCD_Print_String(unsigned char Row, char Col,char *data)
{
LCD_Cursor(Row,Col);
LCD_String(data);
}


