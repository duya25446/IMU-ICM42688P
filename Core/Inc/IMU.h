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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "ICM42688P_Config.h"
#include "stm32g4xx_hal.h"  // 需要 UART_HandleTypeDef 类型定义

    /* Exported types ------------------------------------------------------------*/

    /**
     * @brief IMU数据结构体
     * 
     * 包含加速度计、陀螺仪、四元数、磁力计和时间戳数据
     */
    typedef struct
    {
        /** 加速度计数据，单位为g（重力加速度） */
        float accel_x; // X轴加速度
        float accel_y; // Y轴加速度
        float accel_z; // Z轴加速度

        /** 陀螺仪数据，单位为度每秒（dps） */
        float gyro_x; // X轴角速度
        float gyro_y; // Y轴角速度
        float gyro_z; // Z轴角速度

        /** 四元数，用于表示设备的旋转状态 */
        float q0; // q0（四元数实部）
        float q1; // q1
        float q2; // q2
        float q3; // q3
        
        /** 温度数据，单位为摄氏度 */
        float temperature;

        /** IMU绝对时间戳，单位为微秒（μs），从初始化开始累计 */
        uint32_t timestamp;

        /** 磁力计数据，单位为微特斯拉（μT） */
        float mag_x; // X轴磁场
        float mag_y; // Y轴磁场
        float mag_z; // Z轴磁场

        /** 磁力计温度数据，单位为摄氏度 */
        float mag_temperature;

        /** 磁力计绝对时间戳，单位为微秒（μs），从初始化开始累计 */
        uint32_t mag_timestamp;

        /** 磁力计数据就绪标志（内部使用） */
        volatile uint8_t mag_data_ready;

    } IMU_Data;

    /* Exported constants --------------------------------------------------------*/

    /* Exported macro ------------------------------------------------------------*/

    /* Exported functions prototypes ---------------------------------------------*/
    
    /**
     * @brief 配置IMU模块使用的UART句柄
     * @param huart UART句柄指针
     * @note 必须在IMU_Init()之前调用，否则使用默认的huart2
     *       用于输出初始化信息和调试信息
     */
    void IMU_Set_UART(UART_HandleTypeDef *huart);
    
    /**
     * @brief 初始化IMU传感器（包括ICM42688P和BMM350）
     * @return 0=成功, 非0=失败
     */
    uint8_t IMU_Init(void);
    
    /**
     * @brief 生成出厂配置（保留接口）
     * @param config ICM42688P配置结构体指针
     */
    void IMU_GenerateFactoryConfig(ICM42688P_Config *config);
    
    /**
     * @brief ICM42688P中断处理函数
     * @param data IMU数据结构体指针
     * @return 0=成功, 非0=失败
     */
    uint8_t IMU_InterruptHandle(IMU_Data *data);
    
    /**
     * @brief BMM350磁力计中断处理函数
     * @param data IMU数据结构体指针
     * @return 0=成功, 非0=失败
     */
    uint8_t IMU_MagInterruptHandle(IMU_Data *data);

#ifdef __cplusplus
}
#endif

#endif /* IMU_H */
