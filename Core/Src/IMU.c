/**
 ******************************************************************************
 * @file    IMU.c
 * @brief   IMU传感器抽象层实现
 ******************************************************************************
 */

#include "IMU.h"
#include "main.h"
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
// 用于软件时间戳计算的上次TIM3计数值
static uint16_t last_tim3_count = 0;

// 外部定时器句柄声明
extern TIM_HandleTypeDef htim3;

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
                         0x06,  // odr: 1kHz (从8kHz降低到1kHz以适应UART带宽)
                         0b10,  // ui_filt_ord: 3阶滤波器（保持默认）
                         1      // ui_filt_bw: 带宽设置（保持默认）
    );

    // 4. 配置加速度计 (对应 ICM42688P_ODR_Config)
    ICM42688P_ConfigAccel(config,
                          0b000, // fs_sel: ±16g
                          0x06,  // odr: 1kHz (从8kHz降低到1kHz以适应UART带宽)
                          0b10,  // ui_filt_ord: 3阶滤波器（保持默认）
                          1      // ui_filt_bw: 带宽设置（保持默认）
    );

    // 5. 配置时钟 (对应 ICM42688P_Clock_Config)
    // 使用TIM2生成的PWM作为外部时钟源(约32768Hz)，实现时钟同步
    ICM42688P_ConfigClock(config,
                          0b01, // clk_sel: 自动选择PLL（陀螺仪开启时用PLL）
                          1,    // rtc_mode: 1 = 需要外部RTC时钟
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

    // 6.1 配置中断时序 - 高速ODR的推荐配置
    // 直接设置 INT_CONFIG1 寄存器 (0x64) = 0x60
    // bit6=1: 8μs脉冲宽度（适用于ODR≥4kHz）
    // bit5=1: 禁用去断言延迟（适用于ODR≥4kHz）
    // bit4=0: async_reset=0（必须设为0以确保INT1/INT2正常工作）
    config->bank0.INT_CONFIG1 = 0x60;

    // 6.2 配置中断清除方式
    // 直接设置 INT_CONFIG0 寄存器 (0x63) = 0x30
    // bit5:4=11: 数据就绪中断需要读状态位+读传感器数据才清除（最安全）
    // bit3:2=00: FIFO阈值中断清除方式默认（未使用FIFO）
    // bit1:0=00: FIFO满中断清除方式默认（未使用FIFO）
    config->bank0.INT_CONFIG0 = 0x30;

    // 6.3 配置时间戳功能（注：UI读取模式下使用软件时间戳，硬件时间戳配置可选）
    // 对应寄存器 TMST_CONFIG (0x54)
    ICM42688P_ConfigTimestamp(config,
                              0, // enable: 0=禁用硬件时间戳（使用软件时间戳）
                              0, // resolution: 0=1μs分辨率
                              0, // delta_en: 0=绝对时间戳模式
                              0, // fsync_en: 0=禁用FSYNC时间戳
                              0  // tmst_to_regs_en: 0=禁用寄存器读取
    );

    // 7. 配置中断源 (对应 ICM42688P_Interrupt_Config)
    ICM42688P_ConfigInterruptSource(config,
                                    0x08, // int1_sources: 数据就绪中断 (UI_DRDY_INT1_EN)
                                    0x00  // int2_sources: 未使用
    );
}

uint8_t IMU_InterruptHandle(IMU_Data *data)
{
    // 立即读取TIM3计数器，获取最准确的时间戳
    // TIM3配置：Prescaler=169, Period=65535, 时钟=170MHz/170=1MHz
    // 计数精度：1μs，溢出周期：65.536ms
    uint16_t current_tim3_count = __HAL_TIM_GET_COUNTER(&htim3);
    
    // 计算时间差（μs），处理16位计数器溢出
    // 8kHz ODR理论间隔=125μs，不会在正常情况下溢出
    uint16_t time_delta;
    if (current_tim3_count >= last_tim3_count) {
        // 正常情况：当前计数值大于上次计数值
        time_delta = current_tim3_count - last_tim3_count;
    } else {
        // 溢出情况：计数器从65535回绕到0
        time_delta = (65536 - last_tim3_count) + current_tim3_count;
    }
    data->timestamp = time_delta;
    last_tim3_count = current_tim3_count;
    
    // 读取并验证中断状态
    uint8_t interrupt_status;
    ICM42688P_ReadRegister(0x2D, &interrupt_status, 1);
    if (!(interrupt_status == 0b00011000 || interrupt_status == 0b00001000))
    {
        return 1;
    }
    
    // 读取IMU数据（会自动清除中断）
    ICM42688P_ReadIMUData(data);
    return 0;
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
    
    // 初始化软件时间戳基准
    last_tim3_count = __HAL_TIM_GET_COUNTER(&htim3);
    
    return error;
}

void IMU_GenerateFactoryConfig(ICM42688P_Config *config) {}
