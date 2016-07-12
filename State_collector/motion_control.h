#ifndef	MOTION_CONTROL_H
#define MOTION_CONTROL_H

/*****Function Prototypes*********************/

void Forward(void);
void Back(void);
void Left(void);
void Right(void);
void Stop(void);
void Soft_Left(void);
void Soft_Right(void);
void Soft_Left2(void);
void Soft_Right2(void);
void L_Forward(void);
void L_Back(void);
void R_Forward(void);
void R_Back(void);
void L_Stop(void);
void R_Stop(void);
void Init_Motion_Pin(void);
void Init_Buzzer_Pin(void);
void Init_Peripherals(void);
void Init_Ports(void);
void Angle_Rotate(unsigned int Degrees);
void Linear_Distance_mm(unsigned int DistanceInMM);
void Forward_mm(unsigned int DistanceInMM);
void Back_mm(unsigned int DistanceInMM);
void Left_Degrees(unsigned int Degrees);
void Soft_Left_Degrees(unsigned int Degrees);
void Soft_Left_2_Degrees(unsigned int Degrees);
void Right_Degrees(unsigned int Degrees);
void Soft_Right_Degrees(unsigned int Degrees);
void Soft_Right_2_Degrees(unsigned int Degrees);

/**********************************************/

#endif
