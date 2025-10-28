/**
 ******************************************************************************
 * @file    BMM350_Driver.h
 * @brief   BMM350磁力计驱动封装层头文件
 * @note    提供高级驱动接口，包括初始化、校准、数据读取等功能
 ******************************************************************************
 */

#ifndef BMM350_DRIVER_H
#define BMM350_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "bmm350.h"
#include "BMM350_Platform.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 设置调试输出UART句柄
 * @param huart UART句柄指针
 * @note 在调用BMM350_Init()之前调用此函数来配置调试输出
 *       如果不调用此函数，所有调试信息将不会输出
 */
void BMM350_Set_Debug_UART(UART_HandleTypeDef *huart);

/**
 * @brief 初始化BMM350磁力计
 * @return BMM350_OK(0) 成功，其他值失败
 * @note 配置为400Hz ODR + 超低噪声模式（averaging=8）
 *       初始化流程：
 *       1. 配置设备结构体
 *       2. 执行芯片初始化（读取CHIP ID和OTP校准参数）
 *       3. 设置ODR=400Hz，AVG=8（超低噪声）
 *       4. 使能X/Y/Z三轴
 *       5. 配置数据就绪中断
 *       6. 切换到Normal Mode
 */
int8_t BMM350_Init(void);

/**
 * @brief 执行出厂校准
 * @return BMM350_OK(0) 成功，其他值失败
 * @note 校准流程：
 *       1. 执行磁场复位（消除强磁场影响）
 *       2. 执行X/Y轴自检（验证传感器功能）
 *       3. 通过UART2输出校准参数和自检结果
 */
int8_t BMM350_Factory_Calibration(void);

/**
 * @brief 获取原始磁场数据（未补偿）
 * @param raw_data 原始数据结构体指针
 * @return BMM350_OK(0) 成功，其他值失败
 * @note 返回24位有符号整型数据（LSB单位）
 */
int8_t BMM350_Get_Raw_Data(struct bmm350_raw_mag_data *raw_data);

/**
 * @brief 获取补偿后的磁场数据
 * @param mag_data 补偿后数据结构体指针
 * @return BMM350_OK(0) 成功，其他值失败
 * @note 返回浮点型数据，单位为微特斯拉（μT）
 *       包含X/Y/Z三轴磁场和温度数据
 */
int8_t BMM350_Get_Compensated_Data(struct bmm350_mag_temp_data *mag_data);

/**
 * @brief 检查数据是否就绪
 * @return 1 数据就绪，0 数据未就绪
 * @note 通过轮询INT_STATUS寄存器的DRDY位实现
 */
uint8_t BMM350_Is_Data_Ready(void);

/**
 * @brief 获取BMM350设备结构体指针
 * @return 设备结构体指针
 * @note 用于直接调用Bosch原生API
 */
struct bmm350_dev* BMM350_Get_Device(void);

#ifdef __cplusplus
}
#endif

#endif /* BMM350_DRIVER_H */

