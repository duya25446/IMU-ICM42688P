/**
 * @file IMU.h
 * @brief IMU传感器抽象层头文件
 *
 * 该文件定义了IMU传感器的抽象接口，提供统一的传感器数据访问和配置接口。
 * 支持多种IMU传感器，包括ICM-42688P等。
 */

#ifndef IMU_H
#define IMU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "ICM42688P_Config.h"
#include "ICM-42688P.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
uint8_t IMU_Init(void);
void IMU_GenerateFactoryConfig(ICM42688P_Config *config);
#ifdef __cplusplus
}
#endif

#endif /* IMU_H */
