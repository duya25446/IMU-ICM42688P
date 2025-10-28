/**
 ******************************************************************************
 * @file    BMM350_Debug.h
 * @brief   BMM350调试工具头文件
 * @note    提供I2C扫描、寄存器读取等调试功能
 ******************************************************************************
 */

#ifndef BMM350_DEBUG_H
#define BMM350_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bmm350_defs.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 扫描I2C1总线上的所有设备
 * @param huart UART句柄，用于输出结果
 * @note 扫描地址范围：0x01 ~ 0x7F
 */
void BMM350_Debug_I2C_Scan(UART_HandleTypeDef *huart);

/**
 * @brief 尝试读取BMM350的CHIP_ID
 * @param huart UART句柄，用于输出结果
 * @return 读取到的CHIP_ID（0xFF表示读取失败）
 */
uint8_t BMM350_Debug_Read_Chip_ID(UART_HandleTypeDef *huart);

/**
 * @brief 测试基本I2C通信
 * @param huart UART句柄，用于输出结果
 * @note 测试多个I2C地址，输出详细的通信结果
 */
void BMM350_Debug_Test_I2C_Communication(UART_HandleTypeDef *huart);

/**
 * @brief 输出详细的硬件检查清单
 * @param huart UART句柄，用于输出结果
 */
void BMM350_Debug_Hardware_Checklist(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* BMM350_DEBUG_H */

