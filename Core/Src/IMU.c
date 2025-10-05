/**
 ******************************************************************************
 * @file    IMU.c
 * @brief   IMU传感器抽象层实现
 ******************************************************************************
 */

#include "IMU.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>

/* ============================================================================
 * Private Macro Definitions
 * ============================================================================ */

/* ============================================================================
 * Private Type Definitions
 * ============================================================================ */
ICM42688P_Config icm42688p_config;

/* ============================================================================
 * Private Variables
 * ============================================================================ */

/* ============================================================================
 * Private Function Prototypes
 * ============================================================================ */
void ICM42688P_MigrateDefaultConfig(ICM42688P_Config *config)
{
    // 1. 初始化配置结构体
    ICM42688P_InitConfig(config);

    // 2. 配置电源管理 (对应 ICM42688P_Start)
    ICM42688P_ConfigPower(config,
                          0b11, // gyro_mode: 11 = Low Noise Mode
                          0b11, // accel_mode: 11 = Low Noise Mode
                          0,    // temp_disable: 0 = 使能温度传感器
                          0     // idle_mode: 0 = OFF时关闭RC
    );

    // 3. 配置陀螺仪 (对应 ICM42688P_ODR_Config)
    ICM42688P_ConfigGyro(config,
                         0b000, // fs_sel: ±2000dps
                         0x01,  // odr: 32kHz
                         0b10,  // ui_filt_ord: 3阶滤波器（保持默认）
                         1      // ui_filt_bw: 带宽设置（保持默认）
    );

    // 4. 配置加速度计 (对应 ICM42688P_ODR_Config)
    ICM42688P_ConfigAccel(config,
                          0b000, // fs_sel: ±16g
                          0x01,  // odr: 32kHz
                          0b10,  // ui_filt_ord: 3阶滤波器（保持默认）
                          1      // ui_filt_bw: 带宽设置（保持默认）
    );

    // 5. 配置时钟 (对应 ICM42688P_Clock_Config)
    ICM42688P_ConfigClock(config,
                          0b01, // clk_sel: 自动选择PLL
                          1,    // rtc_mode: 需要外部RTC时钟
                          0b10  // pin9_func: CLKIN (外部时钟输入)
    );

    // 6. 配置中断引脚 (对应 ICM42688P_Interrupt_Config)
    ICM42688P_ConfigInterrupt(config,
                              0, // int1_polarity: 低电平有效
                              1, // int1_drive: 推挽输出
                              0, // int1_mode: 脉冲模式
                              0, // int2_polarity: (未使用)
                              0, // int2_drive: (未使用)
                              0  // int2_mode: (未使用)
    );

    // 7. 配置中断源 (对应 ICM42688P_Interrupt_Config)
    ICM42688P_ConfigInterruptSource(config,
                                    0x08, // int1_sources: 数据就绪中断 (UI_DRDY_INT1_EN)
                                    0x00  // int2_sources: 未使用
    );

    // 8. 应用配置到芯片
    //  uint8_t result = ICM42688P_ApplyConfigIncremental(config);
    //  if (result != 0)
    //  {
    //      // 配置失败处理
    //      return 1;
    //  }
    //  return 0;
}
/* ============================================================================
 * Public Function Implementations
 * ============================================================================ */

/**
 * @brief 初始化IMU传感器
 * @return 0=成功, 非0=失败
 */
uint8_t IMU_Init(void)
{
    uint8_t error = 0;
    if (ICM42688P_Init() != 0)
    {
        return 1;
    } // 检查IMU是否正常工作
    // TODO:未来可以添加判断EEPROM是否存在过去配置，如果存在就用非出厂模式启动，如果不存在就使用出厂测试模式流程
    // 加载默认配置
    ICM42688P_LoadDefaultConfig(&icm42688p_config);
    error += ICM42688P_ReadGyroFactoryCalibration(&icm42688p_config);
    ICM42688P_MigrateDefaultConfig(&icm42688p_config);
    error += ICM42688P_ApplyConfig(&icm42688p_config);
    return error;
}

void IMU_GenerateFactoryConfig(ICM42688P_Config *config) {}
