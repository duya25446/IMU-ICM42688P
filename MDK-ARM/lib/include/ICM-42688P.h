#ifndef ICM42688P_H
#define ICM42688P_H

#include "main.h"

#define cs_high() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);
#define cs_low() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);



#define ICM42688P_READ 0x80


uint8_t ICM42688P_ReadRegister(uint8_t reg);



#endif