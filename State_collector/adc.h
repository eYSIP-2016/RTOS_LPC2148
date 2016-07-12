#ifndef	ADC_H
#define ADC_H

/*******************Prototype***************************/

void Init_ADC_Pin(void);
void Init_ADC0(void);
void Init_ADC1(void);
unsigned int AD0_Conversion(unsigned char channel);
unsigned int AD1_Conversion(unsigned char channel);
unsigned int Sharp_GP2D12_Estimation(unsigned int Val);
unsigned int Batt_Voltage_Conversion(unsigned int Val);

/*******************************************************/

#endif
