/**
 ******************************************************************************
 * @file    ICM-42688P.h
 * @brief   ICM-42688P IMU传感器驱动头文件
 * @note    该文件定义了ICM-42688P IMU传感器的驱动接口，包括寄存器操作、
 *          数据读取、配置设置等功能。
 ******************************************************************************
 */

#ifndef ICM42688P_H
#define ICM42688P_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"  // 需要HAL_GPIO_WritePin等HAL库函数
#include "math.h"  // 需要fabs等数学函数
#include "float.h" // 需要FLT_EPSILON等浮点常量
#include "IMU.h"   // 使用统一的IMU_Data结构体定义

/* Exported constants --------------------------------------------------------*/

/**
 * @brief 片选信号置高
 */
#define cs_high() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET)

/**
 * @brief 片选信号置低
 */
#define cs_low() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET)

/**
 * @brief 陀螺仪全量程，设置为2000dps
 */
#define GYRO_FULL_SCALE 2000.0

/**
 * @brief 陀螺仪灵敏度，16位分辨率
 */
#define GYRO_SENSITIVITY (GYRO_FULL_SCALE / 32768.0)

/**
 * @brief 加速度计全量程，设置为16g
 */
#define ACCEL_FULL_SCALE 16.0

/**
 * @brief 加速度计灵敏度，16位分辨率
 */
#define ACCEL_SENSITIVITY (ACCEL_FULL_SCALE / 32768.0)

/**
 * @brief 零偏校准参数 - IMU2
 */
#define axzeroffset 0.0f
#define ayzeroffset 0.0f
#define azzeroffset 0.0f
#define gxzeroffset 0.0f
#define gyzeroffset 0.0f
#define gzzeroffset 0.0f

/**
 * @brief SPI读取寄存器标志位
 */
#define ICM42688P_READ 0x80

/**
 * @brief WHOAMI寄存器地址
 */
#define ICM42688P_WHOAMI 0x75

/**
 * @brief 自检配置寄存器地址（Bank 0）
 */
#define ICM42688P_SELF_TEST_CONFIG  0x70

/**
 * @brief 陀螺仪自检数据寄存器地址（Bank 1）
 */
#define ICM42688P_XG_ST_DATA        0x5F
#define ICM42688P_YG_ST_DATA        0x60
#define ICM42688P_ZG_ST_DATA        0x61

/**
 * @brief 加速度计自检数据寄存器地址（Bank 2）
 */
#define ICM42688P_XA_ST_DATA        0x3B
#define ICM42688P_YA_ST_DATA        0x3C
#define ICM42688P_ZA_ST_DATA        0x3D

/* Exported types ------------------------------------------------------------*/

/**
 * @brief ICM42688P自检结果结构体
 */
typedef struct {
    uint8_t gyro_x_pass;           // X轴陀螺仪自检是否通过
    uint8_t gyro_y_pass;           // Y轴陀螺仪自检是否通过
    uint8_t gyro_z_pass;           // Z轴陀螺仪自检是否通过
    uint8_t accel_x_pass;          // X轴加速度计自检是否通过
    uint8_t accel_y_pass;          // Y轴加速度计自检是否通过
    uint8_t accel_z_pass;          // Z轴加速度计自检是否通过
    int16_t gyro_st_response[3];   // 陀螺仪自检响应（X/Y/Z）
    int16_t accel_st_response[3];  // 加速度计自检响应（X/Y/Z）
    uint8_t gyro_st_otp[3];        // 陀螺仪出厂自检数据（X/Y/Z）
    uint8_t accel_st_otp[3];       // 加速度计出厂自检数据（X/Y/Z）
    uint8_t overall_pass;          // 整体自检是否通过
} ICM42688P_SelfTest_Result;

/* IMU_Data结构体定义已移至IMU.h */

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 初始化ICM42688P传感器
 */
uint8_t ICM42688P_Init(void);

/**
 * @brief 停止ICM42688P传感器
 */
void ICM42688P_Stop(void);

/**
 * @brief 启动ICM42688P传感器
 */
void ICM42688P_Start(void);

/**
 * @brief 配置输出数据速率(ODR)
 */
void ICM42688P_ODR_Config(void);

/**
 * @brief 配置中断
 */
void ICM42688P_Interrupt_Config(void);

/**
 * @brief 配置时钟
 */
void ICM42688P_Clock_Config(void);

/**
 * @brief 软件复位
 */
void ICM42688P_Software_Reset(void);

/**
 * @brief 读取IMU数据（包含温度）
 * @param data 指向IMU_Data结构体的指针，用于存储读取的数据
 * 
 * 从0x1D开始一次性读取温度和IMU数据，提高效率
 */
void ICM42688P_ReadIMUData(IMU_Data *data);

/**
 * @brief 选择寄存器组
 * @param bank 寄存器组编号
 */
void ICM42688P_Bank_Select(uint8_t bank);

/**
 * @brief 读取寄存器
 * @param reg_address 寄存器地址
 * @param rxdata 接收数据缓冲区
 * @param length 数据长度
 */
void ICM42688P_ReadRegister(uint8_t reg_address, uint8_t *rxdata, uint8_t length);

/**
 * @brief 写入寄存器
 * @param reg_address 寄存器地址
 * @param txdata 发送数据缓冲区
 * @param length 数据长度
 * @return 0表示成功，1表示失败
 */
uint8_t ICM42688P_WriteRegister(uint8_t reg_address, uint8_t *txdata, uint8_t length);

/**
 * @brief 执行ICM42688P自检
 * @param result 自检结果结构体指针
 * @return 0表示成功，1表示失败
 * 
 * 自检流程：
 * 1. 读取未使能自检时的传感器输出（基准数据）
 * 2. 使能陀螺仪自检，读取自检输出
 * 3. 计算陀螺仪自检响应 = 自检数据 - 基准数据
 * 4. 使能加速度计自检，读取自检输出
 * 5. 计算加速度计自检响应 = 自检数据 - 基准数据
 * 6. 读取工厂出厂自检数据（Bank 1和Bank 2）
 * 7. 比较自检响应与出厂数据，验证是否在±30%范围内
 */
uint8_t ICM42688P_SelfTest(ICM42688P_SelfTest_Result *result);

#ifdef __cplusplus
}
#endif

#endif /* ICM42688P_H */