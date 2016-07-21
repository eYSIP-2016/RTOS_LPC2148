#ifndef	UART0_H
#define UART0_H

/***************Macros*********************/ 
#define Fosc            12000000                    //10MHz~25MHz
#define Fcclk           (Fosc * 5)                  //Fosc(1~32)<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO Fcclk 2񅦰�16�156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB(Fcclk / 4) 1񄿔
#define  UART_BPS	115200 		//Change Baud Rate Setting here

/******************************************/

extern void  __irq IRQ_UART0(void);
void Init_UART0(void);
void UART0_SendByte(unsigned char data);
void UART0_SendStr(const unsigned char *str);

#endif
