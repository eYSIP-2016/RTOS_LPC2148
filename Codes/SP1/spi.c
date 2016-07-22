#include <LPC214X.H> /* LPC214x definitions */
#include "type.h"
#include "spi.h"


BYTE MMCWRData[MMC_DATA_SIZE]={"\nNEX ROBOTICS PVT LTD\nARM7 LPC2148 DEVELOPMENT BOARD\nSD/MMC SPI INTERFACE TEST"};
BYTE MMCRDData[MMC_DATA_SIZE];
BYTE MMCCmd[MMC_CMD_SIZE];
BYTE MMCStatus = 0;


extern const unsigned char Test[5120];
extern unsigned char Demo[5120];
extern unsigned char Name[];
/*
* SPI and MMC commands related modules.
*
*/
void SPI0_Init( void )
{
 DWORD portConfig;
 BYTE i, Dummy;
 
 portConfig = PINSEL0;			  /* Configure PIN connect block */
 PINSEL0 = portConfig | 0x1500;   /* P0.4, P0.5, P0.6, P0.7 are set as SCK, MISO, MOSI and GPIO */
 
 IODIR0 = SPI0_SEL; 				  /* SSEL is output */
 IOSET0 = SPI0_SEL; 				  /* set SSEL to high */
 VPBDIV = 0x02;					  /* Set PCLK 1/2 of CCLK. CCLK=60MHz, PCLK=30MHz*/ 	
 //S0SPCCR=8;						  /* SPI clock prescale register minimum value is 8. */
 //S0SPCR=0x0030;					  /* Device select as master, Set data to 8-bit, CPOL = 0, CPHA = 0*/ 
 S0SPCCR=30;						  /* SPI clock prescale register minimum value is 8. */
 S0SPCR=0x0020;					  /* Device select as master, Set data to 8-bit, CPOL = 0, CPHA = 0*/ 
 
 for ( i = 0; i < 8; i++ )		  
 {
 	Dummy = S0SPDR; 				  /* clear the RxFIFO */
 }
 return;
}

 /*
 * SPI Send a block of data based on the length
 */
void SPI0_Send( BYTE *buf, DWORD Length )
{
 BYTE Dummy;
 if ( Length == 0 )
 return;
 while ( Length != 0 )
 {
  S0SPDR = *buf;
  while ( !(S0SPSR & 0x80) );	/* Wait until the SPIF bit is set to indicate trabsfer complete */
  Dummy = S0SPDR; 				/* Flush the RxFIFO */
  Length--;
  buf++;
 }
 return;
}


/*
* SPI receives a block of data based on the length
*/
void SPI0_Receive( BYTE *buf, DWORD Length )
{
 DWORD i;
 for ( i = 0; i < Length; i++ )
 {
  *buf = SPI0_ReceiveByte();
  buf++;
 }
 return;
}



/*
* SPI Receive Byte, receive one byte only, return Data byte
* used a lot to check the status.
*/
BYTE SPI0_ReceiveByte( void )
{
 BYTE data;
 S0SPDR = 0xCC;					/* wrtie dummy byte out to generate clock, then read data from  MISO */
 while (!(S0SPSR & 0x80) );		/* Wait until the SPIF bit is set to indicate trabsfer complete */
 data = S0SPDR;
 return ( data );
}



void SPI1_Init( void )
{
 BYTE i, Dummy;

 SSPCR1 =  0x00;						/* 0000, Slave output disable (0), Master mode (0), SSP disabled (0), Loopback mode (0) -> 0000 0010 */
 
 PINSEL1 &= 0xFFFFFC03;			  		/* Configure PIN connect block */
 PINSEL1 |= 0x000002A8;   				/* P0.17, P0.18, P0.19, P0.20 are set as SCK, MISO, MOSI and GPIO */
 
 IODIR0 |= (SPI1_SEL | 0x00120000);   	/* SSEL is output */
 IOSET0 |= SPI1_SEL; 				  	/* set SSEL to high */
 //VPBDIV = 0x02;					  	/* Set PCLK 1/2 of CCLK. CCLK=60MHz, PCLK=30MHz*/ 	
 //S0SPCCR=8;						  	/* SPI clock prescale register minimum value is 8. */
 //S0SPCR=0x0030;					  	/* Device select as master, Set data to 8-bit, CPOL = 0, CPHA = 0*/ 
 SSPCPSR = 2;						  	/* SSP clock prescale register minimum value is 2. */
 SSPCR0 = 0x0F07;					  	/* SCR (00001110) (14 for divisor of 30), CPHA = 0, CPOL = 0, Frame Format SPI (00), Set data to 8-bit(0111)  -> 0000 1110 0000 0111*/ 
 SSPCR1 = 0x02;						/* 0000, Slave output disable (0), Master mode (0), SSP Enable (1), Loopback mode (0) -> 0000 0010 */
 
 for ( i = 0; i < 8; i++ )		  
 {
 	Dummy = SSPDR; 				  /* clear the RxFIFO */
 }
 return;
}

void SPI1_SendByte( BYTE data )
{
	//while ( !(SSPSR & 0x02) );
	SSPDR = data;
	while ( (SSPSR & 0x10) );	/* Wait until the TFE bit is set to indicate trabsfer complete */
	data = SSPDR; 				/* Flush the RxFIFO */
}

BYTE SPI1_ReceiveByte( void )
{
 BYTE data;
 SSPDR = 0xCC;					/* wrtie dummy byte out to generate clock, then read data from  MISO */
 while ( (SSPSR & 0x10) );		/* Wait until the SPIF bit is set to indicate trabsfer complete */
 data = SSPDR;
 return ( data );
}

