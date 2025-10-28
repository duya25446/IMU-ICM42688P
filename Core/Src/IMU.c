/**
 ******************************************************************************
 * @file    IMU.c
 * @brief   IMU传感器抽象层实现
 * @note    集成ICM42688P（加速度计+陀螺仪）和BMM350（磁力计）
 ******************************************************************************
 */

#include "IMU.h"
#include "ICM-42688P.h"
#include "BMM350/BMM350_Driver.h"
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
// ICM42688P时间戳相关变量
// 用于软件时间戳计算的上次TIM3计数值
static uint16_t last_tim3_count = 0;

// 绝对时间戳累积变量（单位：微秒），从IMU初始化开始累计
static uint32_t absolute_timestamp_us = 0;

// BMM350磁力计时间戳相关变量
// 磁力计上次TIM3计数值
static uint16_t last_mag_tim3_count = 0;

// 磁力计绝对时间戳累积变量（单位：微秒），从初始化开始累计
static uint32_t mag_absolute_timestamp_us = 0;

// 外部定时器句柄声明
extern TIM_HandleTypeDef htim3;

// 外部UART句柄声明（用于初始化输出）
extern UART_HandleTypeDef huart2;

// IMU模块使用的UART句柄，默认使用huart2
static UART_HandleTypeDef *imu_uart = &huart2;

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
    // 1kHz ODR理论间隔=1000μs，8kHz ODR理论间隔=125μs
    uint16_t time_delta;
    if (current_tim3_count >= last_tim3_count)
    {
        // 正常情况：当前计数值大于上次计数值
        time_delta = current_tim3_count - last_tim3_count;
    }
    else
    {
        // 溢出情况：计数器从65535回绕到0
        time_delta = (65536 - last_tim3_count) + current_tim3_count;
    }

    // 累积到绝对时间戳（μs）
    // uint32_t最大值：4,294,967,295μs ≈ 4295秒 ≈ 71.6分钟
    // 溢出后会自动回绕，用户需要根据应用场景自行处理
    absolute_timestamp_us += time_delta;
    data->timestamp = absolute_timestamp_us;
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
 * @brief 配置IMU模块使用的UART句柄
 * @param huart UART句柄指针
 * @note 必须在IMU_Init()之前调用，否则使用默认的huart2
 */
void IMU_Set_UART(UART_HandleTypeDef *huart)
{
    if (huart != NULL) {
        imu_uart = huart;
    }
}

/**
 * @brief 初始化IMU传感器（包括ICM42688P和BMM350）
 * @return 0=成功, 非0=失败
 */
uint8_t IMU_Init(void)
{
    uint8_t error = 0;
    int8_t bmm_rslt = 0;
    char buffer[200];

    // ========================================================================
    // 1. 初始化ICM42688P（加速度计+陀螺仪）
    // ========================================================================
    sprintf(buffer, "\r\n========================================\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "    ICM42688P IMU初始化\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "========================================\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    if (ICM42688P_Init() != 0)
    {
        sprintf(buffer, "[ICM42688P] ✗ 初始化失败\r\n");
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
        return 1;
    }

    sprintf(buffer, "[ICM42688P] ✓ 初始化成功\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    // 执行ICM42688P自检
    sprintf(buffer, "\r\n---------- ICM42688P自检 ----------\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    ICM42688P_SelfTest_Result st_result;
    if (ICM42688P_SelfTest(&st_result) == 0)
    {
        // 输出陀螺仪自检结果
        sprintf(buffer, "[陀螺仪] X轴: %s (响应=%d, OTP=%d)\r\n",
                st_result.gyro_x_pass ? "✓ 通过" : "✗ 失败",
                st_result.gyro_st_response[0],
                st_result.gyro_st_otp[0]);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        sprintf(buffer, "[陀螺仪] Y轴: %s (响应=%d, OTP=%d)\r\n",
                st_result.gyro_y_pass ? "✓ 通过" : "✗ 失败",
                st_result.gyro_st_response[1],
                st_result.gyro_st_otp[1]);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        sprintf(buffer, "[陀螺仪] Z轴: %s (响应=%d, OTP=%d)\r\n",
                st_result.gyro_z_pass ? "✓ 通过" : "✗ 失败",
                st_result.gyro_st_response[2],
                st_result.gyro_st_otp[2]);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        // 输出加速度计自检结果
        sprintf(buffer, "[加速度计] X轴: %s (响应=%d, OTP=%d)\r\n",
                st_result.accel_x_pass ? "✓ 通过" : "✗ 失败",
                st_result.accel_st_response[0],
                st_result.accel_st_otp[0]);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        sprintf(buffer, "[加速度计] Y轴: %s (响应=%d, OTP=%d)\r\n",
                st_result.accel_y_pass ? "✓ 通过" : "✗ 失败",
                st_result.accel_st_response[1],
                st_result.accel_st_otp[1]);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        sprintf(buffer, "[加速度计] Z轴: %s (响应=%d, OTP=%d)\r\n",
                st_result.accel_z_pass ? "✓ 通过" : "✗ 失败",
                st_result.accel_st_response[2],
                st_result.accel_st_otp[2]);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        // 输出整体自检结果
        sprintf(buffer, "\r\n[ICM42688P] 整体自检: %s\r\n",
                st_result.overall_pass ? "✓ 通过" : "✗ 失败");
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

        if (!st_result.overall_pass)
        {
            error += 1;
        }
    }
    else
    {
        sprintf(buffer, "[ICM42688P] ✗ 自检执行失败\r\n");
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
        error += 1;
    }

    sprintf(buffer, "========================================\r\n\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    // 加载默认配置
    ICM42688P_LoadDefaultConfig(&icm42688p_config);
    error += ICM42688P_ReadGyroFactoryCalibration(&icm42688p_config);
    ICM42688P_MigrateDefaultConfig(&icm42688p_config);
    error += ICM42688P_ApplyConfig(&icm42688p_config);

    // 初始化ICM42688P时间戳系统
    absolute_timestamp_us = 0;
    last_tim3_count = __HAL_TIM_GET_COUNTER(&htim3);

    // ========================================================================
    // 2. 初始化BMM350（磁力计）
    // ========================================================================
    sprintf(buffer, "\r\n========================================\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "    BMM350磁力计初始化\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "========================================\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
    BMM350_Set_Debug_UART(imu_uart);
    // 初始化BMM350
    bmm_rslt = BMM350_Init();

    if (bmm_rslt == BMM350_OK)
    {
        sprintf(buffer, "[BMM350] ✓ 初始化成功\r\n");
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    else
    {
        sprintf(buffer, "[BMM350] ✗ 初始化失败，错误码: %d\r\n", bmm_rslt);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
        error += 1;
    }

    // 等待传感器稳定
    HAL_Delay(100);

    // 执行出厂校准
    bmm_rslt = BMM350_Factory_Calibration();

    if (bmm_rslt == BMM350_OK)
    {
        sprintf(buffer, "[BMM350] ✓ 出厂校准成功\r\n");
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    else
    {
        sprintf(buffer, "[BMM350] ✗ 出厂校准失败，错误码: %d\r\n", bmm_rslt);
        HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    sprintf(buffer, "\r\n========================================\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "    开始采集IMU+磁力计数据\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "    ICM42688P: 1kHz | BMM350: 100Hz\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    sprintf(buffer, "========================================\r\n");
    HAL_UART_Transmit(imu_uart, (uint8_t *)buffer, strlen(buffer), 1000);

    // 初始化BMM350时间戳系统
    mag_absolute_timestamp_us = 0;
    last_mag_tim3_count = __HAL_TIM_GET_COUNTER(&htim3);

    return error;
}

/**
 * @brief BMM350磁力计中断处理函数
 * @param data IMU数据结构体指针
 * @return 0=成功, 非0=失败
 */
uint8_t IMU_MagInterruptHandle(IMU_Data *data)
{
    // 立即读取TIM3计数器，获取最准确的时间戳
    // TIM3配置：Prescaler=169, Period=65535, 时钟=170MHz/170=1MHz
    // 计数精度：1μs，溢出周期：65.536ms
    uint16_t current_tim3_count = __HAL_TIM_GET_COUNTER(&htim3);

    // 计算时间差（μs），处理16位计数器溢出
    // 400Hz ODR理论间隔=2500μs
    uint16_t time_delta;
    if (current_tim3_count >= last_mag_tim3_count)
    {
        // 正常情况：当前计数值大于上次计数值
        time_delta = current_tim3_count - last_mag_tim3_count;
    }
    else
    {
        // 溢出情况：计数器从65535回绕到0
        time_delta = (65536 - last_mag_tim3_count) + current_tim3_count;
    }

    // 累积到磁力计绝对时间戳（μs）
    mag_absolute_timestamp_us += time_delta;
    last_mag_tim3_count = current_tim3_count;

    // 读取补偿后的磁场数据
    struct bmm350_mag_temp_data mag_data;
    int8_t rslt = BMM350_Get_Compensated_Data(&mag_data);

    if (rslt == BMM350_OK)
    {
        // 填充磁力计数据到IMU_Data结构体
        data->mag_x = mag_data.x;
        data->mag_y = mag_data.y;
        data->mag_z = mag_data.z;
        data->mag_temperature = mag_data.temperature;
        data->mag_timestamp = mag_absolute_timestamp_us;

        // 设置数据就绪标志
        data->mag_data_ready = 1;

        return 0;
    }

    return 1;
}

void IMU_GenerateFactoryConfig(ICM42688P_Config *config) {}
