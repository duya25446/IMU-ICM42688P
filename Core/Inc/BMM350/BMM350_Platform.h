/**
 ******************************************************************************
 * @file    BMM350_Platform.h
 * @brief   BMM350磁力计STM32平台适配层头文件
 * @note    提供I2C读写和延时函数接口，适配STM32 HAL库
 ******************************************************************************
 */

#ifndef BMM350_PLATFORM_H
#define BMM350_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "bmm350.h"
#include "main.h"

/* Exported constants --------------------------------------------------------*/

/**
 * @brief BMM350 I2C设备地址（7位地址，ADSEL引脚接地）
 */
#define BMM350_I2C_ADDR  0x14

/**
 * @brief I2C超时时间（毫秒）
 */
#define BMM350_I2C_TIMEOUT  100

/* Exported functions --------------------------------------------------------*/

/**
 * @brief I2C读取函数
 * @param reg_addr 寄存器地址
 * @param reg_data 读取数据缓冲区指针
 * @param len 读取数据长度
 * @param intf_ptr 接口指针（未使用，保留用于兼容性）
 * @return BMM350_INTF_RET_SUCCESS(0) 成功，其他值失败
 * @note BMM350 I2C读取会返回2字节dummy data，在实现中已处理
 */
BMM350_INTF_RET_TYPE BMM350_I2C_Read(uint8_t reg_addr, uint8_t *reg_data, 
                                      uint32_t len, void *intf_ptr);

/**
 * @brief I2C写入函数
 * @param reg_addr 寄存器地址
 * @param reg_data 写入数据缓冲区指针
 * @param len 写入数据长度
 * @param intf_ptr 接口指针（未使用，保留用于兼容性）
 * @return BMM350_INTF_RET_SUCCESS(0) 成功，其他值失败
 */
BMM350_INTF_RET_TYPE BMM350_I2C_Write(uint8_t reg_addr, const uint8_t *reg_data,
                                       uint32_t len, void *intf_ptr);

/**
 * @brief 微秒级延时函数
 * @param period 延时时间（微秒）
 * @param intf_ptr 接口指针（未使用，保留用于兼容性）
 * @note 使用HAL_Delay实现毫秒级延时，对于微秒级延时使用简单循环
 */
void BMM350_Delay_US(uint32_t period, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif /* BMM350_PLATFORM_H */

