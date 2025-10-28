/**
 ******************************************************************************
 * @file    BMM350_Driver.c
 * @brief   BMM350磁力计驱动封装层实现文件
 * @note    实现初始化、校准、数据读取等高级驱动功能
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "BMM350_Driver.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
static struct bmm350_dev bmm350_dev;      // BMM350设备结构体
static uint8_t i2c_addr = BMM350_I2C_ADDR;  // I2C设备地址
static UART_HandleTypeDef *p_debug_uart = NULL;  // 调试输出UART句柄指针

/* Private macros ------------------------------------------------------------*/
// 安全的UART输出宏（仅在UART句柄有效时输出）
#define DEBUG_PRINT(buffer) \
    do { \
        if (p_debug_uart != NULL) { \
            HAL_UART_Transmit(p_debug_uart, (uint8_t*)(buffer), strlen(buffer), 1000); \
        } \
    } while(0)

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 设置调试输出UART句柄
 * @param huart UART句柄指针
 * @note 在调用BMM350_Init()之前调用此函数来配置调试输出
 */
void BMM350_Set_Debug_UART(UART_HandleTypeDef *huart)
{
    p_debug_uart = huart;
}

/**
 * @brief 初始化BMM350磁力计
 */
int8_t BMM350_Init(void)
{
    int8_t rslt;
    char debug_buffer[200];
    
    // 1. 配置设备结构体
    bmm350_dev.intf_ptr = &i2c_addr;
    bmm350_dev.read = BMM350_I2C_Read;
    bmm350_dev.write = BMM350_I2C_Write;
    bmm350_dev.delay_us = BMM350_Delay_US;
    
    // 2. 初始化芯片（读取CHIP ID和OTP校准参数）
    rslt = bmm350_init(&bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 初始化失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    sprintf(debug_buffer, "[BMM350] Chip ID: 0x%02X\r\n", bmm350_dev.chip_id);
    DEBUG_PRINT(debug_buffer);
    
    // 3. 设置ODR=100Hz，AVG=4（Low Noise模式）
    // 根据API代码分析：100Hz支持的最大AVG值为4
    rslt = bmm350_set_odr_performance(BMM350_DATA_RATE_100HZ, BMM350_AVERAGING_4, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 设置ODR和性能失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    sprintf(debug_buffer, "[BMM350] 配置: ODR=100Hz, AVG=4 (Low Noise, 4次平均)\r\n");
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "[BMM350] 性能: 功耗335μA, 噪声X/Y~190nT Z~450nT\r\n");
    DEBUG_PRINT(debug_buffer);
    
    // 4. 使能X/Y/Z三轴
    rslt = bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 使能三轴失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    // 5. 配置数据就绪中断（映射到INT引脚PB9）
    rslt = bmm350_configure_interrupt(BMM350_PULSED,
                                      BMM350_ACTIVE_HIGH,
                                      BMM350_INTR_PUSH_PULL,
                                      BMM350_MAP_TO_PIN,
                                      &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 配置中断失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    // 使能数据就绪中断
    rslt = bmm350_enable_interrupt(BMM350_ENABLE_INTERRUPT, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 使能中断失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    // 6. 切换到Normal Mode
    rslt = bmm350_set_powermode(BMM350_NORMAL_MODE, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 切换到Normal模式失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    sprintf(debug_buffer, "[BMM350] 已切换到Normal Mode\r\n");
    DEBUG_PRINT(debug_buffer);
    
    return BMM350_OK;
}

/**
 * @brief 执行出厂校准
 */
int8_t BMM350_Factory_Calibration(void)
{
    int8_t rslt;
    char debug_buffer[256];
    struct bmm350_self_test self_test_result;
    
    sprintf(debug_buffer, "\r\n========== BMM350出厂校准开始 ==========\r\n");
    DEBUG_PRINT(debug_buffer);
    
    // 1. 切换到Suspend模式（磁场复位和自检必须在Suspend模式下进行）
    rslt = bmm350_set_powermode(BMM350_SUSPEND_MODE, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 切换到Suspend模式失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    HAL_Delay(10);  // 等待模式切换完成
    
    // 2. 执行磁场复位
    sprintf(debug_buffer, "[BMM350] 执行磁场复位...\r\n");
    DEBUG_PRINT(debug_buffer);
    
    rslt = bmm350_magnetic_reset_and_wait(&bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 磁场复位失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    sprintf(debug_buffer, "[BMM350] 磁场复位完成\r\n");
    DEBUG_PRINT(debug_buffer);
    
    // 3. 执行X/Y轴自检
    sprintf(debug_buffer, "[BMM350] 执行X/Y轴自检...\r\n");
    DEBUG_PRINT(debug_buffer);
    
    rslt = bmm350_perform_self_test(&self_test_result, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 自检失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    // 4. 输出自检结果
    sprintf(debug_buffer, "[BMM350] X轴自检结果: %.2f μT\r\n", self_test_result.out_ust_x);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "[BMM350] Y轴自检结果: %.2f μT\r\n", self_test_result.out_ust_y);
    DEBUG_PRINT(debug_buffer);
    
    // 验证自检结果（应该≥130μT）
    if (self_test_result.out_ust_x >= 130.0f && self_test_result.out_ust_y >= 130.0f) {
        sprintf(debug_buffer, "[BMM350] ✓ 自检通过（X/Y轴磁场变化≥130μT）\r\n");
    } else {
        sprintf(debug_buffer, "[BMM350] ✗ 自检未通过（X/Y轴磁场变化<130μT）\r\n");
    }
    DEBUG_PRINT(debug_buffer);
    
    // 5. 输出校准参数
    sprintf(debug_buffer, "\r\n---------- 校准参数 ----------\r\n");
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "磁场Offset: X=%.2f, Y=%.2f, Z=%.2f\r\n",
            bmm350_dev.mag_comp.dut_offset_coef.offset_x,
            bmm350_dev.mag_comp.dut_offset_coef.offset_y,
            bmm350_dev.mag_comp.dut_offset_coef.offset_z);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "磁场Sensitivity: X=%.2f, Y=%.2f, Z=%.2f\r\n",
            bmm350_dev.mag_comp.dut_sensit_coef.sens_x,
            bmm350_dev.mag_comp.dut_sensit_coef.sens_y,
            bmm350_dev.mag_comp.dut_sensit_coef.sens_z);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "温度Offset: %.2f\r\n", bmm350_dev.mag_comp.dut_offset_coef.t_offs);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "温度Sensitivity: %.2f\r\n", bmm350_dev.mag_comp.dut_sensit_coef.t_sens);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "TCO: X=%.2f, Y=%.2f, Z=%.2f\r\n",
            bmm350_dev.mag_comp.dut_tco.tco_x,
            bmm350_dev.mag_comp.dut_tco.tco_y,
            bmm350_dev.mag_comp.dut_tco.tco_z);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "TCS: X=%.4f, Y=%.4f, Z=%.4f\r\n",
            bmm350_dev.mag_comp.dut_tcs.tcs_x,
            bmm350_dev.mag_comp.dut_tcs.tcs_y,
            bmm350_dev.mag_comp.dut_tcs.tcs_z);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "T0: %.2f\r\n", bmm350_dev.mag_comp.dut_t0);
    DEBUG_PRINT(debug_buffer);
    
    sprintf(debug_buffer, "========== 出厂校准完成 ==========\r\n\r\n");
    DEBUG_PRINT(debug_buffer);
    
    // 6. 切换回Normal Mode
    rslt = bmm350_set_powermode(BMM350_NORMAL_MODE, &bmm350_dev);
    if (rslt != BMM350_OK) {
        sprintf(debug_buffer, "[BMM350] 切换回Normal模式失败，错误码: %d\r\n", rslt);
        DEBUG_PRINT(debug_buffer);
        return rslt;
    }
    
    return BMM350_OK;
}

/**
 * @brief 获取原始磁场数据（未补偿）
 */
int8_t BMM350_Get_Raw_Data(struct bmm350_raw_mag_data *raw_data)
{
    if (raw_data == NULL) {
        return BMM350_E_NULL_PTR;
    }
    
    return bmm350_read_uncomp_mag_temp_data(raw_data, &bmm350_dev);
}

/**
 * @brief 获取补偿后的磁场数据
 */
int8_t BMM350_Get_Compensated_Data(struct bmm350_mag_temp_data *mag_data)
{
    if (mag_data == NULL) {
        return BMM350_E_NULL_PTR;
    }
    
    return bmm350_get_compensated_mag_xyz_temp_data(mag_data, &bmm350_dev);
}

/**
 * @brief 检查数据是否就绪
 */
uint8_t BMM350_Is_Data_Ready(void)
{
    uint8_t int_status = 0;
    int8_t rslt;
    
    // 读取中断状态寄存器
    rslt = bmm350_get_regs(BMM350_REG_INT_STATUS, &int_status, 1, &bmm350_dev);
    
    if (rslt != BMM350_OK) {
        return 0;
    }
    
    // 检查数据就绪位
    if (int_status & BMM350_DRDY_DATA_REG_MSK) {
        return 1;
    }
    
    return 0;
}

/**
 * @brief 获取BMM350设备结构体指针
 */
struct bmm350_dev* BMM350_Get_Device(void)
{
    return &bmm350_dev;
}

