/*
 LCD Connections:

 			  LCD	  Microcontroller Pins
 			  RS  --> P1.19
			  RW  --> P1.18
			  EN  --> P1.17
			  DB7 --> P1.25
			  DB6 --> P1.24
			  DB5 --> P1.23
			  DB4 --> P1.22

*/

#ifndef LCD_H
#define LCD_H
#include <lpc214x.h>

#define RS  1<<19
#define RW  1<<18
#define	EN  1<<17
#define DB7 1<<25
#define DB6 1<<24
#define DB5 1<<23
#define DB4 1<<22

#define DATA_BUS ((DB7)|(DB6)|(DB5)|(DB4))
#define LCD_IO ((RS)|(RW)|(EN)| DATA_BUS)	
	

#define DATA_PORT() IO1SET=RS	
#define READ_DATA() IO1SET=RW
#define EN_HI() IO1SET=EN

#define COMMAND_PORT() IO1CLR=RS	
#define WRITE_DATA() IO1CLR=RW
#define EN_LOW() IO1CLR=EN
/***********Prototypes****************/

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

void print_LCD_Data_using_queue(unsigned char row, unsigned char col, unsigned int print_data, unsigned int Digits);

void print_LCD_String_using_queue(unsigned char row,unsigned char col, char *print_data);

void vLCD_Display_String(void *pvparam);
void vLCD_Display_Data(void *pvparam) ;
/**************************************/
#endif 
