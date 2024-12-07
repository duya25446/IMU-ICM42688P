#ifndef ICM42688P_H
#define ICM42688P_H

#include "main.h"

#define cs_high() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);
#define cs_low() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);



#define ICM42688P_READ 0x80

void ICM42688P_Init();
void ICM42688P_Stop();
void ICM42688P_Start();
void ICM42688P_ODRcfg();
float ICM42688P_GetTemp();
void ICM42688P_SoftwareReset();
void ICM42688P_ReadRegister(uint8_t reg_address,uint8_t* rxdata,uint8_t length);
uint8_t ICM42688P_WriteRegister(uint8_t reg_address,uint8_t* txdata,uint8_t length);



#endif