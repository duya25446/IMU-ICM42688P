/**
 ******************************************************************************
 * @file    BMM350_Debug.c
 * @brief   BMM350调试工具实现文件
 * @note    提供I2C扫描、寄存器读取等调试功能
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "BMM350_Debug.h"
#include "BMM350_Platform.h"
#include <stdio.h>
#include <string.h>

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 扫描I2C1总线上的所有设备
 */
void BMM350_Debug_I2C_Scan(UART_HandleTypeDef *huart)
{
    char buffer[100];
    uint8_t found_devices = 0;
    
    sprintf(buffer, "\r\n========== I2C1总线扫描 ==========\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "扫描地址范围: 0x01 ~ 0x7F\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    for (uint8_t addr = 0x01; addr < 0x80; addr++) {
        // 尝试向该地址发送数据
        HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 10);
        
        if (result == HAL_OK) {
            sprintf(buffer, "✓ 发现设备: 0x%02X (7位地址)\r\n", addr);
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
            found_devices++;
        }
    }
    
    if (found_devices == 0) {
        sprintf(buffer, "\r\n✗ 未发现任何I2C设备！\r\n");
    } else {
        sprintf(buffer, "\r\n总共发现 %d 个I2C设备\r\n", found_devices);
    }
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "==================================\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
}

/**
 * @brief 尝试读取BMM350的CHIP_ID
 */
uint8_t BMM350_Debug_Read_Chip_ID(UART_HandleTypeDef *huart)
{
    char buffer[150];
    uint8_t chip_id = 0xFF;
    HAL_StatusTypeDef status;
    
    sprintf(buffer, "\r\n========== 读取CHIP_ID测试 ==========\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // 测试地址0x14（ADSEL接地）
    sprintf(buffer, "测试地址 0x14 (ADSEL=GND)...\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    uint8_t reg_addr = BMM350_REG_CHIP_ID;
    uint8_t temp_buffer[BMM350_DUMMY_BYTES + 1];
    
    // 发送寄存器地址
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x14 << 1, &reg_addr, 1, BMM350_I2C_TIMEOUT);
    if (status != HAL_OK) {
        sprintf(buffer, "  ✗ 发送寄存器地址失败，HAL状态: %d\r\n", status);
        HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    } else {
        sprintf(buffer, "  ✓ 发送寄存器地址成功\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
        
        // 读取数据（包含2字节dummy）
        status = HAL_I2C_Master_Receive(&hi2c1, 0x14 << 1, temp_buffer, 3, BMM350_I2C_TIMEOUT);
        if (status != HAL_OK) {
            sprintf(buffer, "  ✗ 读取数据失败，HAL状态: %d\r\n", status);
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
        } else {
            chip_id = temp_buffer[2];  // 跳过2字节dummy
            sprintf(buffer, "  ✓ 读取成功\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
            sprintf(buffer, "  Dummy bytes: 0x%02X 0x%02X\r\n", temp_buffer[0], temp_buffer[1]);
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
            sprintf(buffer, "  CHIP_ID: 0x%02X (期望值: 0x33)\r\n", chip_id);
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
            
            if (chip_id == 0x33) {
                sprintf(buffer, "  ✓ CHIP_ID正确！\r\n");
            } else {
                sprintf(buffer, "  ✗ CHIP_ID不匹配！\r\n");
            }
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
        }
    }
    
    // 测试地址0x15（ADSEL接高）
    sprintf(buffer, "\r\n测试地址 0x15 (ADSEL=VDD)...\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x15 << 1, &reg_addr, 1, BMM350_I2C_TIMEOUT);
    if (status == HAL_OK) {
        status = HAL_I2C_Master_Receive(&hi2c1, 0x15 << 1, temp_buffer, 3, BMM350_I2C_TIMEOUT);
        if (status == HAL_OK) {
            sprintf(buffer, "  ✓ 地址0x15响应！CHIP_ID: 0x%02X\r\n", temp_buffer[2]);
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
            sprintf(buffer, "  ⚠️ ADSEL可能未接地！\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
        } else {
            sprintf(buffer, "  ✓ 地址0x15无响应（正常，ADSEL应该接地）\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
        }
    } else {
        sprintf(buffer, "  ✓ 地址0x15无响应（正常，ADSEL应该接地）\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    }
    
    sprintf(buffer, "====================================\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    return chip_id;
}

/**
 * @brief 测试基本I2C通信
 */
void BMM350_Debug_Test_I2C_Communication(UART_HandleTypeDef *huart)
{
    char buffer[150];
    
    sprintf(buffer, "\r\n========== I2C通信测试 ==========\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // 测试1：检查I2C设备是否就绪
    sprintf(buffer, "测试1: 检查设备是否就绪...\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, 0x14 << 1, 3, 100);
    if (status == HAL_OK) {
        sprintf(buffer, "  ✓ 地址0x14设备就绪\r\n");
    } else {
        sprintf(buffer, "  ✗ 地址0x14设备未就绪，HAL状态: %d\r\n", status);
        sprintf(buffer + strlen(buffer), "    HAL_OK=0, HAL_ERROR=1, HAL_BUSY=2, HAL_TIMEOUT=3\r\n");
    }
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // 测试2：读取多个寄存器
    sprintf(buffer, "\r\n测试2: 读取前3个寄存器...\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    uint8_t reg_data[10];
    uint8_t reg_addr;
    
    // 读取0x00 (CHIP_ID)
    reg_addr = 0x00;
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x14 << 1, &reg_addr, 1, 100);
    if (status == HAL_OK) {
        status = HAL_I2C_Master_Receive(&hi2c1, 0x14 << 1, reg_data, 3, 100);
        if (status == HAL_OK) {
            sprintf(buffer, "  0x00 CHIP_ID: 0x%02X (期望0x33)\r\n", reg_data[2]);
        } else {
            sprintf(buffer, "  ✗ 读取0x00失败\r\n");
        }
    } else {
        sprintf(buffer, "  ✗ 发送寄存器地址0x00失败\r\n");
    }
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // 读取0x02 (ERR_REG)
    reg_addr = 0x02;
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x14 << 1, &reg_addr, 1, 100);
    if (status == HAL_OK) {
        status = HAL_I2C_Master_Receive(&hi2c1, 0x14 << 1, reg_data, 3, 100);
        if (status == HAL_OK) {
            sprintf(buffer, "  0x02 ERR_REG: 0x%02X\r\n", reg_data[2]);
            HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
        }
    }
    
    sprintf(buffer, "==================================\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
}

/**
 * @brief 输出详细的硬件检查清单
 */
void BMM350_Debug_Hardware_Checklist(UART_HandleTypeDef *huart)
{
    char buffer[200];
    
    sprintf(buffer, "\r\n========== BMM350硬件检查清单 ==========\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "请逐项检查以下硬件连接：\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "1. 电源供电\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] VDD = 1.72V ~ 1.98V (典型1.8V)\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] VDDIO = 1.72V ~ 3.6V (可用3.3V)\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] GND正确连接\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "2. I2C通信线\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] SCL (A3)连接到I2C1_SCL\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] SDA (A2)连接到I2C1_SDA\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] SCL/SDA有上拉电阻(2.2kΩ到VDD或VDDIO)\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "3. 地址配置\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] ADSEL (B2)接地 → I2C地址=0x14\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] ADSEL不能悬空！\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "4. 必需外部元件\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] CRST (C2)连接2.2μF电容到GND\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] VDD去耦电容100nF\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] VDDIO去耦电容100nF\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] BYPASS (C3)接GND\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "5. 焊接质量\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] 所有焊点良好（无虚焊、短路）\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] 芯片封装无损坏\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "6. STM32侧I2C配置\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] I2C1已初始化\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] I2C1时钟使能\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    sprintf(buffer, "   [ ] GPIO已配置为I2C功能\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
    
    sprintf(buffer, "==========================================\r\n\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
}

