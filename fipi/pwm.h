#ifndef	PWM_H
#define PWM_H

/***********Prototypes*****************/

void Init_PWM(void);
void UpdateLeftPWM(unsigned int vel);
void UpdateRightPWM(unsigned int vel);
void UpdateVelocity(unsigned int Left,unsigned int Right);

/**************************************/

#endif
