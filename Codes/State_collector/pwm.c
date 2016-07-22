#include <LPC214x.h>
#include "pwm.h"

/************************************************************

	Function 		: Init_PWM
	Return type		: None
	Parameters		: None
	Description 	: Initialises PWM module for motion control
************************************************************/

void Init_PWM(void)
{
 PINSEL0&=0xFFFF3FFF;
 PINSEL0|=0x00008000;	//Enabling P0.7 as PWM2
 PINSEL1&=0xFFFFF3FF;		
 PINSEL1|=0x00000400;	//Enabling P0.22 as PWM5

 PWMPR	= 30;	//PWM Prescaler PCLK/30 = 500KHz
 PWMPC	= 0;	//PWMPC increments on every PCLK
 PWMTC	= 0;	//PWMTC increments on every PWMPC=PWMPR
 PWMMR0 = 500;	//PWM base frequency 500KHz/500=1KHz	 
 PWMMR1 = 0;
 PWMMR2 = 0;
 PWMMR3 = 0;
 PWMMR4 = 0;
 PWMMR5 = 0;
 PWMMR6 = 0;
 PWMMCR = 0x00000002;
 PWMPCR	= 0x2600;
 PWMLER	= 0x7F;
 PWMTCR = 0x01;
}  


void UpdateLeftPWM(unsigned int vel)
{
 PWMMR2 = vel;
 PWMLER = 0x04;
}

void UpdateRightPWM(unsigned int vel)
{
 PWMMR5 = vel;
 PWMLER = 0x20;
}

void UpdateVelocity(unsigned int Left,unsigned int Right)
{
 UpdateLeftPWM(Left);
 UpdateRightPWM(Right);
}
