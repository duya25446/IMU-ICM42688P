/**
 * @file ICM-42688P.h
 * @brief ICM-42688P IMU传感器驱动头文件
 *
 * 该文件定义了ICM-42688P IMU传感器的驱动接口，包括寄存器操作、
 * 数据读取、配置设置等功能。
 */

#ifndef ICM42688P_H
#define ICM42688P_H

#include "main.h"  // 需要HAL_GPIO_WritePin等HAL库函数
#include "math.h"  // 需要fabs等数学函数
#include "float.h" // 需要FLT_EPSILON等浮点常量

/**
 * @brief 片选信号置高
 */
#define cs_high() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);

/**
 * @brief 片选信号置低
 */
#define cs_low() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

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

// 零偏校准参数 - IMU2
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
 * @brief IMU数据结构体
 *
 * 包含加速度计、陀螺仪、四元数和时间戳数据
 */
typedef struct
{
    /** 加速度计数据，单位为g（重力加速度） */
    double accel_x; // X轴加速度
    double accel_y; // Y轴加速度
    double accel_z; // Z轴加速度

    /** 陀螺仪数据，单位为度每秒（dps） */
    double gyro_x; // X轴角速度
    double gyro_y; // Y轴角速度
    double gyro_z; // Z轴角速度

    /** 四元数，用于表示设备的旋转状态 */
    double q0; // q0（四元数实部）
    double q1; // q1
    double q2; // q2
    double q3; // q3

    /** 时间戳，用于记录系统时间或某个标准时间的数据 */
    uint64_t timestamp;

} IMU_Data;

// 函数声明

/**
 * @brief 初始化ICM42688P传感器
 */
void ICM42688P_Init(void);

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
 * @brief 获取温度数据
 * @return 温度值（摄氏度）
 */
float ICM42688P_GetTemperature(void);

/**
 * @brief 配置时钟
 */
void ICM42688P_Clock_Config(void);

/**
 * @brief 软件复位
 */
void ICM42688P_Software_Reset(void);

/**
 * @brief 读取IMU数据
 * @param data 指向IMU_Data结构体的指针，用于存储读取的数据
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

#endif