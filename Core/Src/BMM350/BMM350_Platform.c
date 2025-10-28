/**
 ******************************************************************************
 * @file    BMM350_Platform.c
 * @brief   BMM350磁力计STM32平台适配层实现文件
 * @note    实现I2C读写和延时函数，适配STM32 HAL库
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "BMM350_Platform.h"
#include <string.h>

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;  // 使用I2C1接口

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief I2C读取函数实现
 * @param reg_addr 寄存器地址
 * @param reg_data 读取数据缓冲区指针
 * @param len 读取数据长度（调用者已包含dummy bytes）
 * @param intf_ptr 接口指针（未使用）
 * @return BMM350_INTF_RET_SUCCESS(0) 成功，其他值失败
 * @note BMM350 I2C读取协议：前2字节为dummy data
 *       重要：bmm350_get_regs()已经在len中包含了dummy bytes，
 *       所以这里直接读取len字节，不需要再加
 */
BMM350_INTF_RET_TYPE BMM350_I2C_Read(uint8_t reg_addr, uint8_t *reg_data, 
                                      uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    uint8_t device_addr = BMM350_I2C_ADDR << 1;  // 转换为8位地址
    
    (void)intf_ptr;  // 未使用的参数
    
    if (len > (BMM350_READ_BUFFER_LENGTH + BMM350_DUMMY_BYTES)) {
        return BMM350_E_COM_FAIL;
    }
    
    // 步骤1：发送寄存器地址
    status = HAL_I2C_Master_Transmit(&hi2c1, device_addr, &reg_addr, 1, BMM350_I2C_TIMEOUT);
    if (status != HAL_OK) {
        return BMM350_E_COM_FAIL;
    }
    
    // 步骤2：读取数据（len已经包含了2字节dummy data）
    // 调用者(bmm350_get_regs)会自动处理dummy bytes的跳过
    status = HAL_I2C_Master_Receive(&hi2c1, device_addr, reg_data, len, BMM350_I2C_TIMEOUT);
    if (status != HAL_OK) {
        return BMM350_E_COM_FAIL;
    }
    
    return BMM350_INTF_RET_SUCCESS;
}

/**
 * @brief I2C写入函数实现
 * @param reg_addr 寄存器地址
 * @param reg_data 写入数据缓冲区指针
 * @param len 写入数据长度
 * @param intf_ptr 接口指针（未使用）
 * @return BMM350_INTF_RET_SUCCESS(0) 成功，其他值失败
 */
BMM350_INTF_RET_TYPE BMM350_I2C_Write(uint8_t reg_addr, const uint8_t *reg_data,
                                       uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    uint8_t device_addr = BMM350_I2C_ADDR << 1;  // 转换为8位地址
    
    (void)intf_ptr;  // 未使用的参数
    
    // BMM350 I2C写入：使用HAL_I2C_Mem_Write更简洁
    status = HAL_I2C_Mem_Write(&hi2c1, device_addr, reg_addr, I2C_MEMADD_SIZE_8BIT,
                                (uint8_t*)reg_data, len, BMM350_I2C_TIMEOUT);
    
    if (status != HAL_OK) {
        return BMM350_E_COM_FAIL;
    }
    
    return BMM350_INTF_RET_SUCCESS;
}

/**
 * @brief 微秒级延时函数实现
 * @param period 延时时间（微秒）
 * @param intf_ptr 接口指针（未使用）
 * @note 对于大于1000us的延时使用HAL_Delay，小于1000us使用循环延时
 */
void BMM350_Delay_US(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;  // 未使用的参数
    
    if (period >= 1000) {
        // 大于等于1ms，使用HAL_Delay
        HAL_Delay(period / 1000);
        
        // 处理余数（微秒部分）
        uint32_t remainder_us = period % 1000;
        if (remainder_us > 0) {
            // 简单循环延时（170MHz系统时钟，约每循环6ns）
            // 1us约需要170个循环
            volatile uint32_t delay_count = remainder_us * 170;
            while (delay_count--) {
                __NOP();
            }
        }
    } else {
        // 小于1ms，使用循环延时
        // 简单循环延时（170MHz系统时钟，约每循环6ns）
        // 1us约需要170个循环
        volatile uint32_t delay_count = period * 170;
        while (delay_count--) {
            __NOP();
        }
    }
}

