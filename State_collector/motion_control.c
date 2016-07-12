/*****************************************************************************************
There are two components to the motion control:
 1. Direction control using pins P0.22, P1.21, P0.10, P0.11
 2. Velocity control by PWM on pins P0.7/PWM2 and P0.21/PWM5.

 In this experiment for the simplicity P0.7 and P0.21 are kept at logic 1.
 
 
 Connection Details:  	L-1---->P1.21;		L-2---->P0.22;
   						R-1---->P0.10;		R-2---->P0.11;
   						P0.7 (PWM2) ----> Logic 1; 	P0.21 (PWM5) ----> Logic 1; 


******************************************************************************************/

#include <LPC214x.h>
#include "motion_control.h"



volatile unsigned int Left_Shaft_Count=0;
volatile unsigned int Right_Shaft_Count=0;

/************************************************************

	Function 		: Init_Motion_Pin
	Return type		: None
	Parameters		: None
	Description 	: Initialises Motion control pins
************************************************************/

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
 //IO0SET = 0x00200080;	// Set PWM pins P0.7/PWM2 and P0.21/PWM5 to logic 1
}

//Function to move Left motor forward
void L_Forward(void)
{
 IO1SET = 0x00200000;		//Set P1.21 to logic '1'
}

//Function to move Left motor backward
void L_Back(void)
{
 IO0SET = 0x00400000;		//Set P0.22 to logic '1'
}

//Function to move Right motor forward
void R_Forward(void)
{
 IO0SET = 0x00000400;		//Set P0.10 to logic '1'
}

//Function to move Right motor backward
void R_Back(void)
{
 IO0SET = 0x00000800;		//Set P0.11 to logic '1'
}

//Function to stop left motor
void L_Stop(void)
{
 IO1CLR = 0x00200000;		//Set P1.21 to logic '0'
 IO0CLR = 0x00400000;		//Set P0.22 to logic '0'
}

//Function to stop Right motor
void R_Stop(void)
{
 IO0CLR = 0x00000400;		//Set P0.10 to logic '0'
 IO0CLR = 0x00000800;		//Set P0.11 to logic '0'
}

//Function to move robot in forward direction
void Forward(void)
{
 Stop();
 L_Forward();
 R_Forward();
}

//Function to move robot in backward direction
void Back(void)
{
 Stop();
 L_Back();
 R_Back();
}

//Function to turn robot in Left direction
void Left(void)
{ 
 Stop();
 L_Back();
 R_Forward();
}

//Function to turn robot in right direction
void Right(void)
{ 
 Stop();
 L_Forward();
 R_Back();
}

//Function to turn robot in Left direction by moving right wheel forward
void Soft_Left(void)
{
 Stop();
 R_Forward();
}

//Function to turn robot in right direction by moving left wheel forward
void Soft_Right(void)
{
 Stop();
 L_Forward();
}

//Function to turn robot in left direction by moving left wheel backward
void Soft_Left2(void)
{
 Stop();
 L_Back();
}

//Function to turn robot in right direction by moving right wheel backward 
void Soft_Right2(void)
{
 Stop();
 R_Back();
}

//Function to stop the robot at its current location
void Stop(void)
{
 L_Stop();
 R_Stop();
}

//Function used for turning robot by specified degrees
void Angle_Rotate(unsigned int Degrees)
{
 float ReqdShaftCount = 0;
 unsigned int ReqdShaftCountInt = 0;

 ReqdShaftCount = (float) Degrees/ 4.090; // division by resolution to get shaft count
 ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
 Left_Shaft_Count = 0; 
 Right_Shaft_Count = 0; 
 while (1)
 {
  if((Right_Shaft_Count >= ReqdShaftCountInt) | (Left_Shaft_Count >= ReqdShaftCountInt))
  break;
 }
 Stop(); //Stop robot
}

//Function used for moving robot forward by specified distance
void Linear_Distance_mm(unsigned int DistanceInMM)
{
 float ReqdShaftCount = 0;
 unsigned int ReqdShaftCountInt = 0;

 ReqdShaftCount = DistanceInMM / 5.338; // division by resolution to get shaft count
 ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
  
 Right_Shaft_Count = 0;
 while(1)
 {
  if(Right_Shaft_Count > ReqdShaftCountInt)
  {
  	break;
  }
 } 
 Stop(); //Stop robot
}

void Forward_mm(unsigned int DistanceInMM)
{
 Forward();
 Linear_Distance_mm(DistanceInMM);
}

void Back_mm(unsigned int DistanceInMM)
{
 Back();
 Linear_Distance_mm(DistanceInMM);
}

void Left_Degrees(unsigned int Degrees) 
{
// 88 pulses for 360 degrees rotation 4.090 degrees per count
 Left(); //Turn left
 Angle_Rotate(Degrees);
}



void Right_Degrees(unsigned int Degrees)
{
// 88 pulses for 360 degrees rotation 4.090 degrees per count
 Right(); //Turn right
 Angle_Rotate(Degrees);
}


void Soft_Left_Degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 Soft_Left(); //Turn soft left
 Degrees=Degrees*2;
 Angle_Rotate(Degrees);
}

void Soft_Right_Degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 Soft_Right();  //Turn soft right
 Degrees=Degrees*2;
 Angle_Rotate(Degrees);
}

void Soft_Left_2_Degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 Soft_Left2(); //Turn reverse soft left
 Degrees=Degrees*2;
 Angle_Rotate(Degrees);
}

void Soft_Right_2_Degrees(unsigned int Degrees)
{
 // 176 pulses for 360 degrees rotation 2.045 degrees per count
 Soft_Right2();  //Turn reverse soft right
 Degrees=Degrees*2;
 Angle_Rotate(Degrees);
}

