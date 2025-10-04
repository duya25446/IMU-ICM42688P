/**
 ******************************************************************************
 * @file    ICM42688P_Config.c
 * @brief   ICM-42688-P配置管理库实现
 ******************************************************************************
 */

#include "ICM42688P_Config.h"
#include "ICM-42688P.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

// 延时函数声明
extern void delay_ms(uint32_t ms);

/* ============================================================================
 * 内部配置存储（用于增量更新）
 * ============================================================================ */
static ICM42688P_Config g_internal_config;  // 内部存储的当前配置
static uint8_t g_config_initialized = 0;    // 是否已初始化（首次ApplyConfig后为1）
static ICM42688P_ConfigDiff g_diff;         // 静态diff结构体，复用节省栈空间

/* ============================================================================
 * 寄存器映射表（用于增量对比）
 * ============================================================================ */

/**
 * @brief 寄存器映射条目
 */
typedef struct {
    uint8_t bank;              // 寄存器所在Bank
    uint8_t reg_addr;          // 寄存器地址
    uint8_t offset_in_struct;  // 在对应Bank结构体中的偏移量（字节）
} ICM42688P_RegMap;

// Bank 0 寄存器映射表
static const ICM42688P_RegMap bank0_reg_map[] = {
    {0, 0x11, offsetof(ICM42688P_Bank0_Config, DEVICE_CONFIG)},
    {0, 0x13, offsetof(ICM42688P_Bank0_Config, DRIVE_CONFIG)},
    {0, 0x14, offsetof(ICM42688P_Bank0_Config, INT_CONFIG)},
    {0, 0x16, offsetof(ICM42688P_Bank0_Config, FIFO_CONFIG)},
    {0, 0x4C, offsetof(ICM42688P_Bank0_Config, INTF_CONFIG0)},
    {0, 0x4D, offsetof(ICM42688P_Bank0_Config, INTF_CONFIG1)},
    {0, 0x4E, offsetof(ICM42688P_Bank0_Config, PWR_MGMT0)},
    {0, 0x4F, offsetof(ICM42688P_Bank0_Config, GYRO_CONFIG0)},
    {0, 0x50, offsetof(ICM42688P_Bank0_Config, ACCEL_CONFIG0)},
    {0, 0x51, offsetof(ICM42688P_Bank0_Config, GYRO_CONFIG1)},
    {0, 0x52, offsetof(ICM42688P_Bank0_Config, GYRO_ACCEL_CONFIG0)},
    {0, 0x53, offsetof(ICM42688P_Bank0_Config, ACCEL_CONFIG1)},
    {0, 0x54, offsetof(ICM42688P_Bank0_Config, TMST_CONFIG)},
    {0, 0x56, offsetof(ICM42688P_Bank0_Config, APEX_CONFIG0)},
    {0, 0x57, offsetof(ICM42688P_Bank0_Config, SMD_CONFIG)},
    {0, 0x5F, offsetof(ICM42688P_Bank0_Config, FIFO_CONFIG1)},
    {0, 0x60, offsetof(ICM42688P_Bank0_Config, FIFO_CONFIG2)},
    {0, 0x61, offsetof(ICM42688P_Bank0_Config, FIFO_CONFIG3)},
    {0, 0x62, offsetof(ICM42688P_Bank0_Config, FSYNC_CONFIG)},
    {0, 0x63, offsetof(ICM42688P_Bank0_Config, INT_CONFIG0)},
    {0, 0x64, offsetof(ICM42688P_Bank0_Config, INT_CONFIG1)},
    {0, 0x65, offsetof(ICM42688P_Bank0_Config, INT_SOURCE0)},
    {0, 0x66, offsetof(ICM42688P_Bank0_Config, INT_SOURCE1)},
    {0, 0x68, offsetof(ICM42688P_Bank0_Config, INT_SOURCE3)},
    {0, 0x69, offsetof(ICM42688P_Bank0_Config, INT_SOURCE4)},
    {0, 0x70, offsetof(ICM42688P_Bank0_Config, SELF_TEST_CONFIG)},
};
#define BANK0_REG_COUNT (sizeof(bank0_reg_map) / sizeof(ICM42688P_RegMap))

// Bank 1 寄存器映射表
static const ICM42688P_RegMap bank1_reg_map[] = {
    {1, 0x03, offsetof(ICM42688P_Bank1_Config, SENSOR_CONFIG0)},
    {1, 0x0B, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC2)},
    {1, 0x0C, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC3)},
    {1, 0x0D, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC4)},
    {1, 0x0E, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC5)},
    {1, 0x0F, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC6)},
    {1, 0x10, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC7)},
    {1, 0x11, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC8)},
    {1, 0x12, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC9)},
    {1, 0x13, offsetof(ICM42688P_Bank1_Config, GYRO_CONFIG_STATIC10)},
    {1, 0x7A, offsetof(ICM42688P_Bank1_Config, INTF_CONFIG4)},
    {1, 0x7B, offsetof(ICM42688P_Bank1_Config, INTF_CONFIG5)},
    {1, 0x7C, offsetof(ICM42688P_Bank1_Config, INTF_CONFIG6)},
};
#define BANK1_REG_COUNT (sizeof(bank1_reg_map) / sizeof(ICM42688P_RegMap))

// Bank 2 寄存器映射表
static const ICM42688P_RegMap bank2_reg_map[] = {
    {2, 0x03, offsetof(ICM42688P_Bank2_Config, ACCEL_CONFIG_STATIC2)},
    {2, 0x04, offsetof(ICM42688P_Bank2_Config, ACCEL_CONFIG_STATIC3)},
    {2, 0x05, offsetof(ICM42688P_Bank2_Config, ACCEL_CONFIG_STATIC4)},
};
#define BANK2_REG_COUNT (sizeof(bank2_reg_map) / sizeof(ICM42688P_RegMap))

// Bank 4 寄存器映射表
static const ICM42688P_RegMap bank4_reg_map[] = {
    {4, 0x40, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG1)},
    {4, 0x41, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG2)},
    {4, 0x42, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG3)},
    {4, 0x43, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG4)},
    {4, 0x44, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG5)},
    {4, 0x45, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG6)},
    {4, 0x46, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG7)},
    {4, 0x47, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG8)},
    {4, 0x48, offsetof(ICM42688P_Bank4_Config, APEX_CONFIG9)},
    {4, 0x4A, offsetof(ICM42688P_Bank4_Config, ACCEL_WOM_X_THR)},
    {4, 0x4B, offsetof(ICM42688P_Bank4_Config, ACCEL_WOM_Y_THR)},
    {4, 0x4C, offsetof(ICM42688P_Bank4_Config, ACCEL_WOM_Z_THR)},
    {4, 0x4D, offsetof(ICM42688P_Bank4_Config, INT_SOURCE6)},
    {4, 0x4E, offsetof(ICM42688P_Bank4_Config, INT_SOURCE7)},
    {4, 0x4F, offsetof(ICM42688P_Bank4_Config, INT_SOURCE8)},
    {4, 0x50, offsetof(ICM42688P_Bank4_Config, INT_SOURCE9)},
    {4, 0x51, offsetof(ICM42688P_Bank4_Config, INT_SOURCE10)},
    {4, 0x77, offsetof(ICM42688P_Bank4_Config, OFFSET_USER0)},
    {4, 0x78, offsetof(ICM42688P_Bank4_Config, OFFSET_USER1)},
    {4, 0x79, offsetof(ICM42688P_Bank4_Config, OFFSET_USER2)},
    {4, 0x7A, offsetof(ICM42688P_Bank4_Config, OFFSET_USER3)},
    {4, 0x7B, offsetof(ICM42688P_Bank4_Config, OFFSET_USER4)},
    {4, 0x7C, offsetof(ICM42688P_Bank4_Config, OFFSET_USER5)},
    {4, 0x7D, offsetof(ICM42688P_Bank4_Config, OFFSET_USER6)},
    {4, 0x7E, offsetof(ICM42688P_Bank4_Config, OFFSET_USER7)},
    {4, 0x7F, offsetof(ICM42688P_Bank4_Config, OFFSET_USER8)},
};
#define BANK4_REG_COUNT (sizeof(bank4_reg_map) / sizeof(ICM42688P_RegMap))

/* ============================================================================
 * 静态辅助函数
 * ============================================================================ */

/**
 * @brief 确保内部配置已初始化（Level 3辅助函数）
 * @note 如果未初始化，自动读取芯片配置并初始化
 */
static inline void ICM42688P_EnsureConfigInitialized(void)
{
    if (!g_config_initialized) {
        ICM42688P_ReadAllConfigRegisters(&g_internal_config);
        g_config_initialized = 1;
    }
}

/**
 * @brief 计算CRC16-CCITT校验和 (非反射版本)
 * @param data 数据指针
 * @param length 数据长度
 * @return CRC16校验和
 * @note 使用多项式0x1021, 初始值0xFFFF, 无反射, 最终异或0x0000
 */
static uint16_t crc16_ccitt(const uint8_t *data, uint16_t length)
{
    // 参数检查
    if (data == NULL || length == 0) {
        return 0xFFFF;
    }

    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }

    return crc;
}

/* ============================================================================
 * 初始化与默认配置函数
 * ============================================================================ */

void ICM42688P_InitConfig(ICM42688P_Config *config)
{
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(ICM42688P_Config));
    config->magic = ICM42688P_CONFIG_MAGIC;
    config->version = ICM42688P_CONFIG_VERSION;
    ICM42688P_LoadDefaultConfig(config);
    ICM42688P_ReadGyroFactoryCalibration(config);
}

/**
 * @brief 读取芯片中所有配置寄存器的初始值
 * @param config 配置结构体指针，用于存储读取的寄存器值
 * @return 0=成功（注意：底层ReadRegister无错误返回，此函数总是返回0）
 * @note 此函数用于验证默认配置值是否正确，通过读取芯片实际寄存器值来确认
 */
uint8_t ICM42688P_ReadAllConfigRegisters(ICM42688P_Config *config)
{
    if (config == NULL) {
        return 1; // NULL指针错误
    }

    // 初始化配置结构体
    memset(config, 0, sizeof(ICM42688P_Config));
    config->magic = ICM42688P_CONFIG_MAGIC;
    config->version = ICM42688P_CONFIG_VERSION;

    // 使用映射表读取所有Bank的寄存器（优化后的实现，避免代码重复）
    // Bank 0 寄存器读取
    ICM42688P_Bank_Select(0);
    uint8_t *bank0_ptr = (uint8_t *)&config->bank0;
    for (uint8_t i = 0; i < BANK0_REG_COUNT; i++) {
        ICM42688P_ReadRegister(bank0_reg_map[i].reg_addr,
                               &bank0_ptr[bank0_reg_map[i].offset_in_struct], 1);
    }

    // Bank 1 寄存器读取
    ICM42688P_Bank_Select(1);
    uint8_t *bank1_ptr = (uint8_t *)&config->bank1;
    for (uint8_t i = 0; i < BANK1_REG_COUNT; i++) {
        ICM42688P_ReadRegister(bank1_reg_map[i].reg_addr,
                               &bank1_ptr[bank1_reg_map[i].offset_in_struct], 1);
    }

    // Bank 2 寄存器读取
    ICM42688P_Bank_Select(2);
    uint8_t *bank2_ptr = (uint8_t *)&config->bank2;
    for (uint8_t i = 0; i < BANK2_REG_COUNT; i++) {
        ICM42688P_ReadRegister(bank2_reg_map[i].reg_addr,
                               &bank2_ptr[bank2_reg_map[i].offset_in_struct], 1);
    }

    // Bank 4 寄存器读取
    ICM42688P_Bank_Select(4);
    uint8_t *bank4_ptr = (uint8_t *)&config->bank4;
    for (uint8_t i = 0; i < BANK4_REG_COUNT; i++) {
        ICM42688P_ReadRegister(bank4_reg_map[i].reg_addr,
                               &bank4_ptr[bank4_reg_map[i].offset_in_struct], 1);
    }

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    // 计算校验和
    config->checksum = ICM42688P_CalculateChecksum(config);

    // 注意：底层ReadRegister无错误返回机制，此函数总是返回0表示成功
    return 0;
}

/**
 * @brief 格式化打印所有寄存器值到字符串缓冲区
 * @param config 配置结构体指针
 * @param buffer 输出缓冲区，最小长度建议为2048字节
 * @param buffer_size 缓冲区大小
 * @return 实际写入的字符数，0=缓冲区太小
 * @note 此函数将寄存器值格式化为人类可读的字符串，方便调试和验证
 */
uint16_t ICM42688P_FormatRegisters(const ICM42688P_Config *config, char *buffer,
                                   uint16_t buffer_size)
{
    if (config == NULL || buffer == NULL) {
        return 0; // NULL指针错误
    }

    if (buffer_size < 2500) {
        return 0; // Buffer too small
    }

    char *ptr = buffer;
    int written = 0;

    // Header information
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "ICM42688P Configuration Register Values\n"
                       "========================================\n"
                       "Magic: 0x%08" PRIX32 ", Version: 0x%04X, Checksum: 0x%04X\n\n",
                       config->magic, config->version, config->checksum);
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0; // 格式化错误或缓冲区不足
    }
    ptr += written;

    // Bank 0 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 0 Registers:\n"
                       "----------------\n");
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    written =
        snprintf(ptr, buffer_size - (ptr - buffer),
                 "DEVICE_CONFIG    (0x11): 0x%02X   | DRIVE_CONFIG     (0x13): 0x%02X\n"
                 "INT_CONFIG       (0x14): 0x%02X   | FIFO_CONFIG      (0x16): 0x%02X\n"
                 "INTF_CONFIG0     (0x4C): 0x%02X   | INTF_CONFIG1     (0x4D): 0x%02X\n"
                 "PWR_MGMT0        (0x4E): 0x%02X   | GYRO_CONFIG0     (0x4F): 0x%02X\n"
                 "ACCEL_CONFIG0    (0x50): 0x%02X   | GYRO_CONFIG1     (0x51): 0x%02X\n"
                 "GYRO_ACCEL_CONFIG0 (0x52): 0x%02X | ACCEL_CONFIG1    (0x53): 0x%02X\n"
                 "TMST_CONFIG      (0x54): 0x%02X   | SMD_CONFIG       (0x57): 0x%02X\n"
                 "FIFO_CONFIG1     (0x5F): 0x%02X   | FIFO_CONFIG2     (0x60): 0x%02X\n"
                 "FIFO_CONFIG3     (0x61): 0x%02X   | FSYNC_CONFIG     (0x62): 0x%02X\n"
                 "INT_CONFIG0      (0x63): 0x%02X   | INT_CONFIG1      (0x64): 0x%02X\n"
                 "INT_SOURCE0      (0x65): 0x%02X   | INT_SOURCE1      (0x66): 0x%02X\n"
                 "INT_SOURCE3      (0x68): 0x%02X   | INT_SOURCE4      (0x69): 0x%02X\n"
                 "SELF_TEST_CONFIG (0x70): 0x%02X   | APEX_CONFIG0     (0x56): 0x%02X\n\n",
                 config->bank0.DEVICE_CONFIG, config->bank0.DRIVE_CONFIG, config->bank0.INT_CONFIG,
                 config->bank0.FIFO_CONFIG, config->bank0.INTF_CONFIG0, config->bank0.INTF_CONFIG1,
                 config->bank0.PWR_MGMT0, config->bank0.GYRO_CONFIG0, config->bank0.ACCEL_CONFIG0,
                 config->bank0.GYRO_CONFIG1, config->bank0.GYRO_ACCEL_CONFIG0,
                 config->bank0.ACCEL_CONFIG1, config->bank0.TMST_CONFIG, config->bank0.SMD_CONFIG,
                 config->bank0.FIFO_CONFIG1, config->bank0.FIFO_CONFIG2, config->bank0.FIFO_CONFIG3,
                 config->bank0.FSYNC_CONFIG, config->bank0.INT_CONFIG0, config->bank0.INT_CONFIG1,
                 config->bank0.INT_SOURCE0, config->bank0.INT_SOURCE1, config->bank0.INT_SOURCE3,
                 config->bank0.INT_SOURCE4, config->bank0.SELF_TEST_CONFIG, config->bank0.APEX_CONFIG0);
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    // Bank 1 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 1 Registers:\n"
                       "----------------\n");
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    written = snprintf(
        ptr, buffer_size - (ptr - buffer),
        "SENSOR_CONFIG0        (0x03): 0x%02X | GYRO_CONFIG_STATIC2  (0x0B): 0x%02X\n"
        "GYRO_CONFIG_STATIC3   (0x0C): 0x%02X | GYRO_CONFIG_STATIC4  (0x0D): 0x%02X\n"
        "GYRO_CONFIG_STATIC5   (0x0E): 0x%02X | GYRO_CONFIG_STATIC6  (0x0F): 0x%02X\n"
        "GYRO_CONFIG_STATIC7   (0x10): 0x%02X | GYRO_CONFIG_STATIC8  (0x11): 0x%02X\n"
        "GYRO_CONFIG_STATIC9   (0x12): 0x%02X | GYRO_CONFIG_STATIC10 (0x13): 0x%02X\n"
        "INTF_CONFIG4          (0x7A): 0x%02X | INTF_CONFIG5         (0x7B): 0x%02X\n"
        "INTF_CONFIG6          (0x7C): 0x%02X\n\n",
        config->bank1.SENSOR_CONFIG0, config->bank1.GYRO_CONFIG_STATIC2,
        config->bank1.GYRO_CONFIG_STATIC3, config->bank1.GYRO_CONFIG_STATIC4,
        config->bank1.GYRO_CONFIG_STATIC5, config->bank1.GYRO_CONFIG_STATIC6,
        config->bank1.GYRO_CONFIG_STATIC7, config->bank1.GYRO_CONFIG_STATIC8,
        config->bank1.GYRO_CONFIG_STATIC9, config->bank1.GYRO_CONFIG_STATIC10,
        config->bank1.INTF_CONFIG4, config->bank1.INTF_CONFIG5, config->bank1.INTF_CONFIG6);
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    // Bank 2 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 2 Registers:\n"
                       "----------------\n");
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "ACCEL_CONFIG_STATIC2 (0x03): 0x%02X\n"
                       "ACCEL_CONFIG_STATIC3 (0x04): 0x%02X\n"
                       "ACCEL_CONFIG_STATIC4 (0x05): 0x%02X\n\n",
                       config->bank2.ACCEL_CONFIG_STATIC2, config->bank2.ACCEL_CONFIG_STATIC3,
                       config->bank2.ACCEL_CONFIG_STATIC4);
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    // Bank 4 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 4 Registers:\n"
                       "----------------\n");
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    written = snprintf(
        ptr, buffer_size - (ptr - buffer),
        "APEX_CONFIG1    (0x40): 0x%02X | APEX_CONFIG2    (0x41): 0x%02X\n"
        "APEX_CONFIG3    (0x42): 0x%02X | APEX_CONFIG4    (0x43): 0x%02X\n"
        "APEX_CONFIG5    (0x44): 0x%02X | APEX_CONFIG6    (0x45): 0x%02X\n"
        "APEX_CONFIG7    (0x46): 0x%02X | APEX_CONFIG8    (0x47): 0x%02X\n"
        "APEX_CONFIG9    (0x48): 0x%02X\n"
        "ACCEL_WOM_X_THR (0x4A): 0x%02X | ACCEL_WOM_Y_THR (0x4B): 0x%02X\n"
        "ACCEL_WOM_Z_THR (0x4C): 0x%02X | OFFSET_USER0    (0x77): 0x%02X\n"
        "OFFSET_USER1    (0x78): 0x%02X | OFFSET_USER2    (0x79): 0x%02X\n"
        "OFFSET_USER3    (0x7A): 0x%02X | OFFSET_USER4    (0x7B): 0x%02X\n"
        "OFFSET_USER5    (0x7C): 0x%02X | OFFSET_USER6    (0x7D): 0x%02X\n"
        "OFFSET_USER7    (0x7E): 0x%02X | OFFSET_USER8    (0x7F): 0x%02X\n\n",
        config->bank4.APEX_CONFIG1, config->bank4.APEX_CONFIG2, config->bank4.APEX_CONFIG3,
        config->bank4.APEX_CONFIG4, config->bank4.APEX_CONFIG5, config->bank4.APEX_CONFIG6,
        config->bank4.APEX_CONFIG7, config->bank4.APEX_CONFIG8, config->bank4.APEX_CONFIG9,
        config->bank4.ACCEL_WOM_X_THR, config->bank4.ACCEL_WOM_Y_THR, config->bank4.ACCEL_WOM_Z_THR,
        config->bank4.OFFSET_USER0, config->bank4.OFFSET_USER1, config->bank4.OFFSET_USER2,
        config->bank4.OFFSET_USER3, config->bank4.OFFSET_USER4, config->bank4.OFFSET_USER5,
        config->bank4.OFFSET_USER6, config->bank4.OFFSET_USER7, config->bank4.OFFSET_USER8);
    if (written < 0 || (size_t)written >= (buffer_size - (ptr - buffer))) {
        return 0;
    }
    ptr += written;

    return (uint16_t)(ptr - buffer);
}

void ICM42688P_LoadDefaultConfig(ICM42688P_Config *config)
{
    if (config == NULL) {
        return;
    }

    // Bank 0 默认值 (根据数据手册)
    config->bank0.DEVICE_CONFIG = 0x00;
    config->bank0.DRIVE_CONFIG = 0x05;
    config->bank0.INT_CONFIG = 0x00;
    config->bank0.FIFO_CONFIG = 0x00;
    config->bank0.INTF_CONFIG0 = 0x30;
    config->bank0.INTF_CONFIG1 = 0x91;
    config->bank0.PWR_MGMT0 = 0x00;     // Sleep模式
    config->bank0.GYRO_CONFIG0 = 0x06;  // ±2000dps, 1kHz
    config->bank0.ACCEL_CONFIG0 = 0x06; // ±16g, 1kHz
    config->bank0.GYRO_CONFIG1 = 0x16;
    config->bank0.GYRO_ACCEL_CONFIG0 = 0x11;
    config->bank0.ACCEL_CONFIG1 = 0x0D;
    config->bank0.TMST_CONFIG = 0x23;
    config->bank0.APEX_CONFIG0 = 0x82; // DMP省电激活, 其他功能禁用
    config->bank0.SMD_CONFIG = 0x00;
    config->bank0.FIFO_CONFIG1 = 0x00;
    config->bank0.FIFO_CONFIG2 = 0x00;
    config->bank0.FIFO_CONFIG3 = 0x00;
    config->bank0.FSYNC_CONFIG = 0x10;
    config->bank0.INT_CONFIG0 = 0x00;
    config->bank0.INT_CONFIG1 = 0x10;
    config->bank0.INT_SOURCE0 = 0x10;
    config->bank0.INT_SOURCE1 = 0x00;
    config->bank0.INT_SOURCE3 = 0x00;
    config->bank0.INT_SOURCE4 = 0x00;
    config->bank0.SELF_TEST_CONFIG = 0x00;

    // Bank 1 默认值
    config->bank1.SENSOR_CONFIG0 = 0x80;
    config->bank1.GYRO_CONFIG_STATIC2 = 0xA0;
    config->bank1.GYRO_CONFIG_STATIC3 = 0x0D;
    config->bank1.GYRO_CONFIG_STATIC4 = 0xAA;
    config->bank1.GYRO_CONFIG_STATIC5 = 0x80;
    config->bank1.GYRO_CONFIG_STATIC6 = 0x00; // 工厂校准值，通过读取获取实际值
    config->bank1.GYRO_CONFIG_STATIC7 = 0x00; // 工厂校准值，通过读取获取实际值
    config->bank1.GYRO_CONFIG_STATIC8 = 0x00; // 工厂校准值，通过读取获取实际值
    config->bank1.GYRO_CONFIG_STATIC9 = 0x00; // 工厂校准值，通过读取获取实际值
    config->bank1.GYRO_CONFIG_STATIC10 = 0x11;
    config->bank1.INTF_CONFIG4 = 0x83;
    config->bank1.INTF_CONFIG5 = 0x00;
    config->bank1.INTF_CONFIG6 = 0x5F;

    // Bank 2 默认值
    config->bank2.ACCEL_CONFIG_STATIC2 = 0x30;
    config->bank2.ACCEL_CONFIG_STATIC3 = 0x40;
    config->bank2.ACCEL_CONFIG_STATIC4 = 0x62;

    // Bank 4 默认值
    config->bank4.APEX_CONFIG1 = 0xA2;
    config->bank4.APEX_CONFIG2 = 0x85;
    config->bank4.APEX_CONFIG3 = 0x51;
    config->bank4.APEX_CONFIG4 = 0xA4;
    config->bank4.APEX_CONFIG5 = 0x8C;
    config->bank4.APEX_CONFIG6 = 0x5C;
    config->bank4.APEX_CONFIG7 = 0x45;
    config->bank4.APEX_CONFIG8 = 0x5B;
    config->bank4.APEX_CONFIG9 = 0x00;
    config->bank4.ACCEL_WOM_X_THR = 0x00;
    config->bank4.ACCEL_WOM_Y_THR = 0x00;
    config->bank4.ACCEL_WOM_Z_THR = 0x00;
    config->bank4.INT_SOURCE6 = 0x00;
    config->bank4.INT_SOURCE7 = 0x00;
    config->bank4.INT_SOURCE8 = 0x00;
    config->bank4.INT_SOURCE9 = 0x00;
    config->bank4.INT_SOURCE10 = 0x00;
    config->bank4.OFFSET_USER0 = 0x00;
    config->bank4.OFFSET_USER1 = 0x00;
    config->bank4.OFFSET_USER2 = 0x00;
    config->bank4.OFFSET_USER3 = 0x00;
    config->bank4.OFFSET_USER4 = 0x00;
    config->bank4.OFFSET_USER5 = 0x00;
    config->bank4.OFFSET_USER6 = 0x00;
    config->bank4.OFFSET_USER7 = 0x00;
    config->bank4.OFFSET_USER8 = 0x00;

    // 更新校验和
    config->checksum = ICM42688P_CalculateChecksum(config);
}

/* ============================================================================
 * 模块化配置函数
 * ============================================================================ */

void ICM42688P_ConfigPower(ICM42688P_Config *config, uint8_t gyro_mode, uint8_t accel_mode,
                           uint8_t temp_disable, uint8_t idle_mode)
{
    if (config == NULL) {
        return;
    }

    config->bank0.PWR_MGMT0 = ((temp_disable & 0x01) << 5) | ((idle_mode & 0x01) << 4) |
                              ((gyro_mode & 0x03) << 2) | (accel_mode & 0x03);
}

void ICM42688P_ConfigGyro(ICM42688P_Config *config, uint8_t fs_sel, uint8_t odr,
                          uint8_t ui_filt_ord, uint8_t ui_filt_bw)
{
    if (config == NULL) {
        return;
    }

    // GYRO_CONFIG0: FS_SEL[7:5] | ODR[3:0]
    config->bank0.GYRO_CONFIG0 = ((fs_sel & 0x07) << 5) | (odr & 0x0F);

    // GYRO_CONFIG1: UI_FILT_ORD[3:2]
    config->bank0.GYRO_CONFIG1 = (config->bank0.GYRO_CONFIG1 & 0xF3) | ((ui_filt_ord & 0x03) << 2);

    // GYRO_ACCEL_CONFIG0: GYRO_UI_FILT_BW[3:0]
    config->bank0.GYRO_ACCEL_CONFIG0 =
        (config->bank0.GYRO_ACCEL_CONFIG0 & 0xF0) | (ui_filt_bw & 0x0F);
}

void ICM42688P_ConfigAccel(ICM42688P_Config *config, uint8_t fs_sel, uint8_t odr,
                           uint8_t ui_filt_ord, uint8_t ui_filt_bw)
{
    if (config == NULL) {
        return;
    }

    // ACCEL_CONFIG0: FS_SEL[7:5] | ODR[3:0]
    config->bank0.ACCEL_CONFIG0 = ((fs_sel & 0x07) << 5) | (odr & 0x0F);

    // ACCEL_CONFIG1: UI_FILT_ORD[4:3]
    config->bank0.ACCEL_CONFIG1 =
        (config->bank0.ACCEL_CONFIG1 & 0xE7) | ((ui_filt_ord & 0x03) << 3);

    // GYRO_ACCEL_CONFIG0: ACCEL_UI_FILT_BW[7:4]
    config->bank0.GYRO_ACCEL_CONFIG0 =
        (config->bank0.GYRO_ACCEL_CONFIG0 & 0x0F) | ((ui_filt_bw & 0x0F) << 4);
}

void ICM42688P_ConfigFIFO(ICM42688P_Config *config, uint8_t mode, uint8_t gyro_en, uint8_t accel_en,
                          uint8_t temp_en, uint16_t watermark)
{
    if (config == NULL) {
        return;
    }

    // 数据手册明确禁止FIFO_WM设为0
    if (watermark == 0) {
        watermark = 1;
    }

    // 限制最大值为2048（物理FIFO大小，防止超出硬件限制）
    if (watermark > 2048) {
        watermark = 2048;
    }

    // FIFO_CONFIG: FIFO_MODE[7:6]
    config->bank0.FIFO_CONFIG = (mode & 0x03) << 6;

    // FIFO_CONFIG1
    config->bank0.FIFO_CONFIG1 =
        ((temp_en & 0x01) << 2) | ((gyro_en & 0x01) << 1) | (accel_en & 0x01);

    // FIFO水印 (12位)
    config->bank0.FIFO_CONFIG2 = watermark & 0xFF;
    config->bank0.FIFO_CONFIG3 = (watermark >> 8) & 0x0F;
}

void ICM42688P_ConfigInterrupt(ICM42688P_Config *config, uint8_t int1_polarity, uint8_t int1_drive,
                               uint8_t int1_mode, uint8_t int2_polarity, uint8_t int2_drive,
                               uint8_t int2_mode)
{
    if (config == NULL) {
        return;
    }

    config->bank0.INT_CONFIG = ((int2_mode & 0x01) << 5) | ((int2_drive & 0x01) << 4) |
                               ((int2_polarity & 0x01) << 3) | ((int1_mode & 0x01) << 2) |
                               ((int1_drive & 0x01) << 1) | (int1_polarity & 0x01);
}

void ICM42688P_ConfigInterruptSource(ICM42688P_Config *config, uint8_t int1_sources,
                                     uint8_t int2_sources)
{
    if (config == NULL) {
        return;
    }

    config->bank0.INT_SOURCE0 = int1_sources;
    config->bank0.INT_SOURCE3 = int2_sources;
}

void ICM42688P_ConfigGyroAAF(ICM42688P_Config *config, uint8_t disable, uint8_t delt,
                             uint16_t deltsqr, uint8_t bitshift)
{
    if (config == NULL) {
        return;
    }

    // GYRO_CONFIG_STATIC2: AAF_DIS[1] (0=使能AAF, 1=禁用AAF)
    config->bank1.GYRO_CONFIG_STATIC2 =
        (config->bank1.GYRO_CONFIG_STATIC2 & 0xFD) | ((disable & 0x01) << 1);

    // GYRO_CONFIG_STATIC3: AAF_DELT[5:0]
    config->bank1.GYRO_CONFIG_STATIC3 = delt & 0x3F;

    // GYRO_CONFIG_STATIC4: AAF_DELTSQR[7:0]
    config->bank1.GYRO_CONFIG_STATIC4 = deltsqr & 0xFF;

    // GYRO_CONFIG_STATIC5: AAF_BITSHIFT[7:4] | AAF_DELTSQR[11:8]
    config->bank1.GYRO_CONFIG_STATIC5 = ((bitshift & 0x0F) << 4) | ((deltsqr >> 8) & 0x0F);
}

void ICM42688P_ConfigAccelAAF(ICM42688P_Config *config, uint8_t disable, uint8_t delt,
                              uint16_t deltsqr, uint8_t bitshift)
{
    if (config == NULL) {
        return;
    }

    // ACCEL_CONFIG_STATIC2: AAF_DELT[6:1] | AAF_DIS[0] (0=使能AAF, 1=禁用AAF)
    config->bank2.ACCEL_CONFIG_STATIC2 = ((delt & 0x3F) << 1) | (disable & 0x01);

    // ACCEL_CONFIG_STATIC3: AAF_DELTSQR[7:0]
    config->bank2.ACCEL_CONFIG_STATIC3 = deltsqr & 0xFF;

    // ACCEL_CONFIG_STATIC4: AAF_BITSHIFT[7:4] | AAF_DELTSQR[11:8]
    config->bank2.ACCEL_CONFIG_STATIC4 = ((bitshift & 0x0F) << 4) | ((deltsqr >> 8) & 0x0F);
}

void ICM42688P_ConfigGyroOffset(ICM42688P_Config *config, int16_t offset_x, int16_t offset_y,
                                int16_t offset_z)
{
    if (config == NULL) {
        return;
    }

    // X轴偏移 (12位, ±64dps, 1/32dps分辨率)
    uint16_t x_offset = (uint16_t)offset_x & 0x0FFF;
    config->bank4.OFFSET_USER0 = x_offset & 0xFF;
    config->bank4.OFFSET_USER1 = (config->bank4.OFFSET_USER1 & 0xF0) | ((x_offset >> 8) & 0x0F);

    // Y轴偏移
    uint16_t y_offset = (uint16_t)offset_y & 0x0FFF;
    config->bank4.OFFSET_USER2 = y_offset & 0xFF;
    config->bank4.OFFSET_USER1 = (config->bank4.OFFSET_USER1 & 0x0F) | ((y_offset >> 4) & 0xF0);

    // Z轴偏移
    uint16_t z_offset = (uint16_t)offset_z & 0x0FFF;
    config->bank4.OFFSET_USER3 = z_offset & 0xFF;
    config->bank4.OFFSET_USER4 = (config->bank4.OFFSET_USER4 & 0xF0) | ((z_offset >> 8) & 0x0F);
}

void ICM42688P_ConfigAccelOffset(ICM42688P_Config *config, int16_t offset_x, int16_t offset_y,
                                 int16_t offset_z)
{
    if (config == NULL) {
        return;
    }

    // X轴偏移 (12位, ±1g, 0.5mg分辨率)
    uint16_t x_offset = (uint16_t)offset_x & 0x0FFF;
    config->bank4.OFFSET_USER5 = x_offset & 0xFF;
    config->bank4.OFFSET_USER4 = (config->bank4.OFFSET_USER4 & 0x0F) | ((x_offset >> 4) & 0xF0);

    // Y轴偏移
    uint16_t y_offset = (uint16_t)offset_y & 0x0FFF;
    config->bank4.OFFSET_USER6 = y_offset & 0xFF;
    config->bank4.OFFSET_USER7 = (config->bank4.OFFSET_USER7 & 0xF0) | ((y_offset >> 8) & 0x0F);

    // Z轴偏移
    uint16_t z_offset = (uint16_t)offset_z & 0x0FFF;
    config->bank4.OFFSET_USER8 = z_offset & 0xFF;
    config->bank4.OFFSET_USER7 = (config->bank4.OFFSET_USER7 & 0x0F) | ((z_offset >> 4) & 0xF0);
}

void ICM42688P_ConfigWOM(ICM42688P_Config *config, uint8_t mode, uint8_t int_mode,
                         uint8_t threshold_x, uint8_t threshold_y, uint8_t threshold_z)
{
    if (config == NULL) {
        return;
    }

    // SMD_CONFIG: WOM_INT_MODE[3] | WOM_MODE[2]
    config->bank0.SMD_CONFIG =
        (config->bank0.SMD_CONFIG & 0xF3) | ((int_mode & 0x01) << 3) | ((mode & 0x01) << 2);

    // WOM阈值
    config->bank4.ACCEL_WOM_X_THR = threshold_x;
    config->bank4.ACCEL_WOM_Y_THR = threshold_y;
    config->bank4.ACCEL_WOM_Z_THR = threshold_z;
}

void ICM42688P_ConfigClock(ICM42688P_Config *config, uint8_t clk_sel, uint8_t rtc_mode,
                           uint8_t pin9_func)
{
    if (config == NULL) {
        return;
    }

    // INTF_CONFIG1: RTC_MODE[2] | CLKSEL[1:0]
    config->bank0.INTF_CONFIG1 =
        (config->bank0.INTF_CONFIG1 & 0xF8) | ((rtc_mode & 0x01) << 2) | (clk_sel & 0x03);

    // INTF_CONFIG5: PIN9_FUNCTION[2:1]
    config->bank1.INTF_CONFIG5 = (config->bank1.INTF_CONFIG5 & 0xF9) | ((pin9_func & 0x03) << 1);
}

void ICM42688P_ConfigTemperature(ICM42688P_Config *config, uint8_t filt_bw)
{
    if (config == NULL) {
        return;
    }

    // GYRO_CONFIG1: TEMP_FILT_BW[7:5]
    config->bank0.GYRO_CONFIG1 = (config->bank0.GYRO_CONFIG1 & 0x1F) | ((filt_bw & 0x07) << 5);
}

void ICM42688P_ConfigFSYNC(ICM42688P_Config *config, uint8_t ui_sel, uint8_t polarity,
                           uint8_t flag_clear_sel)
{
    if (config == NULL) {
        return;
    }

    config->bank0.FSYNC_CONFIG =
        ((ui_sel & 0x07) << 4) | ((flag_clear_sel & 0x01) << 1) | (polarity & 0x01);
}

void ICM42688P_ConfigTimestamp(ICM42688P_Config *config, uint8_t enable, uint8_t resolution,
                               uint8_t delta_en, uint8_t fsync_en, uint8_t tmst_to_regs_en)
{
    if (config == NULL) {
        return;
    }

    config->bank0.TMST_CONFIG = ((tmst_to_regs_en & 0x01) << 4) | ((resolution & 0x01) << 3) |
                                ((delta_en & 0x01) << 2) | ((fsync_en & 0x01) << 1) |
                                (enable & 0x01);
}

void ICM42688P_ConfigAPEX(ICM42688P_Config *config, uint8_t dmp_power_save, uint8_t tap_enable,
                          uint8_t pedometer_enable, uint8_t tilt_enable, uint8_t r2w_enable,
                          uint8_t dmp_odr)
{
    if (config == NULL) {
        return;
    }

    config->bank0.APEX_CONFIG0 = ((dmp_power_save & 0x01) << 7) | ((tap_enable & 0x01) << 6) |
                                 ((pedometer_enable & 0x01) << 5) | ((tilt_enable & 0x01) << 4) |
                                 ((r2w_enable & 0x01) << 3) | (dmp_odr & 0x03);
}

void ICM42688P_ConfigPedometer(ICM42688P_Config *config, uint8_t amp_th, uint8_t step_cnt_th,
                               uint8_t step_det_th, uint8_t sb_timer_th, uint8_t hi_enrgy_th)
{
    if (config == NULL) {
        return;
    }

    config->bank4.APEX_CONFIG2 = ((amp_th & 0x0F) << 4) | (step_cnt_th & 0x0F);
    config->bank4.APEX_CONFIG3 =
        ((step_det_th & 0x07) << 5) | ((sb_timer_th & 0x07) << 2) | (hi_enrgy_th & 0x03);
}

void ICM42688P_ConfigTilt(ICM42688P_Config *config, uint8_t wait_time)
{
    if (config == NULL) {
        return;
    }

    config->bank4.APEX_CONFIG4 = (config->bank4.APEX_CONFIG4 & 0x3F) | ((wait_time & 0x03) << 6);
}

void ICM42688P_ConfigTap(ICM42688P_Config *config, uint8_t min_jerk_thr, uint8_t max_peak_tol,
                         uint8_t tmax, uint8_t tavg, uint8_t tmin)
{
    if (config == NULL) {
        return;
    }

    config->bank4.APEX_CONFIG7 = ((min_jerk_thr & 0x3F) << 2) | (max_peak_tol & 0x03);
    config->bank4.APEX_CONFIG8 =
        ((tmax & 0x03) << 5) | ((tavg & 0x03) << 3) | (tmin & 0x07);
}

void ICM42688P_ConfigR2W(ICM42688P_Config *config, uint8_t sleep_time_out,
                         uint8_t sleep_gesture_delay)
{
    if (config == NULL) {
        return;
    }

    config->bank4.APEX_CONFIG4 = (config->bank4.APEX_CONFIG4 & 0xC7) | ((sleep_time_out & 0x07) << 3);
    config->bank4.APEX_CONFIG6 = sleep_gesture_delay & 0x07;
}

void ICM42688P_ConfigDMPPowerSave(ICM42688P_Config *config, uint8_t low_energy_amp_th,
                                  uint8_t power_save_time)
{
    if (config == NULL) {
        return;
    }

    config->bank4.APEX_CONFIG1 = ((low_energy_amp_th & 0x0F) << 4) | (power_save_time & 0x0F);
}

void ICM42688P_ConfigMountingMatrix(ICM42688P_Config *config, uint8_t matrix)
{
    if (config == NULL) {
        return;
    }

    config->bank4.APEX_CONFIG5 = matrix & 0x07;
}

void ICM42688P_ConfigAPEXInterruptSource(ICM42688P_Config *config, uint8_t int1_sources,
                                         uint8_t int2_sources, uint8_t ibi_sources)
{
    if (config == NULL) {
        return;
    }

    config->bank4.INT_SOURCE6 = int1_sources & 0x3F;
    config->bank4.INT_SOURCE7 = int2_sources & 0x3F;
    config->bank4.INT_SOURCE10 = ibi_sources & 0x3F;
}

/* ============================================================================
 * 辅助工具函数（用于寄存器延时判断）
 * ============================================================================ */

/**
 * @brief 获取寄存器写入后需要的延时时间
 * @param bank 寄存器所在Bank
 * @param reg_addr 寄存器地址
 * @return 延时时间(ms)，0表示无需延时
 * @note 根据数据手册要求，部分寄存器写入后需要延时
 *       为稳健起见，延时时间按手册要求的10倍设置
 */
static uint8_t ICM42688P_GetRegisterDelay(uint8_t bank, uint8_t reg_addr)
{
    // Bank 0 寄存器延时需求
    if (bank == 0) {
        switch (reg_addr) {
        case 0x4E: // PWR_MGMT0 - 电源模式切换后需要延时
            // 数据手册：从OFF切换到其他模式后200μs内不要写寄存器
            // 陀螺仪需保持开启至少45ms
            // 为稳健起见，延时50ms（45ms + 余量）
            return 50;

        case 0x11: // DEVICE_CONFIG - 软复位后需要延时
            // 数据手册：软复位后等待1ms
            // 按10倍 = 10ms
            return 10;

        default:
            // 其他寄存器无需延时
            return 0;
        }
    }

    // Bank 1, 2, 4 的寄存器通常无需延时
    return 0;
}

/* ============================================================================
 * 应用与读取配置函数
 * ============================================================================ */

uint8_t ICM42688P_ApplyConfig(const ICM42688P_Config *config)
{
    uint8_t error = 0;

    // 验证配置
    if (!ICM42688P_ValidateConfig(config)) {
        return 1;
    }

    // 使用映射表写入所有Bank的寄存器（优化后的实现，避免代码重复）
    // Bank 0配置
    ICM42688P_Bank_Select(0);
    const uint8_t *bank0_ptr = (const uint8_t *)&config->bank0;
    for (uint8_t i = 0; i < BANK0_REG_COUNT; i++) {
        error |= ICM42688P_WriteRegister(bank0_reg_map[i].reg_addr,
                                         (uint8_t *)&bank0_ptr[bank0_reg_map[i].offset_in_struct], 1);
        // 根据寄存器类型智能延时
        uint8_t delay = ICM42688P_GetRegisterDelay(0, bank0_reg_map[i].reg_addr);
        if (delay > 0) {
            delay_ms(delay);
        }
    }

    // Bank 1配置
    ICM42688P_Bank_Select(1);
    const uint8_t *bank1_ptr = (const uint8_t *)&config->bank1;
    for (uint8_t i = 0; i < BANK1_REG_COUNT; i++) {
        error |= ICM42688P_WriteRegister(bank1_reg_map[i].reg_addr,
                                         (uint8_t *)&bank1_ptr[bank1_reg_map[i].offset_in_struct], 1);
        // Bank 1寄存器通常不需要延时，但保留接口以便未来扩展
        uint8_t delay = ICM42688P_GetRegisterDelay(1, bank1_reg_map[i].reg_addr);
        if (delay > 0) {
            delay_ms(delay);
        }
    }

    // Bank 2配置
    ICM42688P_Bank_Select(2);
    const uint8_t *bank2_ptr = (const uint8_t *)&config->bank2;
    for (uint8_t i = 0; i < BANK2_REG_COUNT; i++) {
        error |= ICM42688P_WriteRegister(bank2_reg_map[i].reg_addr,
                                         (uint8_t *)&bank2_ptr[bank2_reg_map[i].offset_in_struct], 1);
        uint8_t delay = ICM42688P_GetRegisterDelay(2, bank2_reg_map[i].reg_addr);
        if (delay > 0) {
            delay_ms(delay);
        }
    }

    // Bank 4配置
    ICM42688P_Bank_Select(4);
    const uint8_t *bank4_ptr = (const uint8_t *)&config->bank4;
    for (uint8_t i = 0; i < BANK4_REG_COUNT; i++) {
        error |= ICM42688P_WriteRegister(bank4_reg_map[i].reg_addr,
                                         (uint8_t *)&bank4_ptr[bank4_reg_map[i].offset_in_struct], 1);
        uint8_t delay = ICM42688P_GetRegisterDelay(4, bank4_reg_map[i].reg_addr);
        if (delay > 0) {
            delay_ms(delay);
        }
    }

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    // Level 1: 全局应用成功后，更新内部配置存储
    if (!error) {
        memcpy(&g_internal_config, config, sizeof(ICM42688P_Config));
        g_config_initialized = 1;
    }

    return error;
}



/* ============================================================================
 * 序列化函数
 * ============================================================================ */

uint16_t ICM42688P_ExportConfig(const ICM42688P_Config *config, uint8_t *buffer,
                                uint16_t buffer_size)
{
    if (config == NULL || buffer == NULL) {
        return 0; // NULL指针错误
    }

    uint16_t required_size = sizeof(ICM42688P_Config);

    if (buffer_size < required_size) {
        return 0; // 缓冲区太小
    }

    // 复制配置到缓冲区
    memcpy(buffer, config, required_size);

    return required_size;
}

uint8_t ICM42688P_ImportConfig(ICM42688P_Config *config, const uint8_t *buffer,
                               uint16_t buffer_size)
{
    if (config == NULL || buffer == NULL) {
        return 1; // NULL指针错误
    }

    if (buffer_size < sizeof(ICM42688P_Config)) {
        return 2; // 数据不完整
    }

    // 从缓冲区复制到配置结构
    memcpy(config, buffer, sizeof(ICM42688P_Config));

    // 验证配置
    if (!ICM42688P_ValidateConfig(config)) {
        return 3; // 配置无效
    }

    return 0;
}

/* ============================================================================
 * 工具函数
 * ============================================================================ */

uint16_t ICM42688P_CalculateChecksum(const ICM42688P_Config *config)
{
    // 计算除magic、version和checksum字段外的所有数据的CRC16
    const uint8_t *data = (const uint8_t *)config;
    uint16_t offset = offsetof(ICM42688P_Config, bank0);
    uint16_t length = sizeof(ICM42688P_Config) - offset;

    return crc16_ccitt(data + offset, length);
}

uint8_t ICM42688P_ValidateConfig(const ICM42688P_Config *config)
{
    // 验证魔数
    if (config->magic != ICM42688P_CONFIG_MAGIC) {
        return 0;
    }

    // 验证版本（支持向后兼容）
    if (config->version > ICM42688P_CONFIG_VERSION) {
        return 0;
    }

    // // 验证校验和
    // uint16_t calculated = ICM42688P_CalculateChecksum(config);
    // if (config->checksum != calculated) {
    //     return 0;
    // }

    return 1;
}

uint16_t ICM42688P_GetConfigSize(void)
{
    return sizeof(ICM42688P_Config);
}

/* ============================================================================
 * 工厂校准值读取函数
 * ============================================================================ */

/**
 * @brief 读取陀螺仪噪声滤波器(NF)工厂校准值
 * @param config 配置结构体指针
 * @return 0=成功, 非0=读取错误
 * @note 这些是芯片出厂时的工厂校准值，每个芯片可能不同
 */
uint8_t ICM42688P_ReadGyroFactoryCalibration(ICM42688P_Config *config)
{
    if (config == NULL) {
        return 1; // NULL指针错误
    }

    // 切换到Bank 1
    ICM42688P_Bank_Select(1);

    // 读取陀螺仪噪声滤波器工厂校准值
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC6,
                           &config->bank1.GYRO_CONFIG_STATIC6, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC7,
                           &config->bank1.GYRO_CONFIG_STATIC7, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC8,
                           &config->bank1.GYRO_CONFIG_STATIC8, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC9,
                           &config->bank1.GYRO_CONFIG_STATIC9, 1);

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    return 0;
}

/* ============================================================================
 * 辅助工具函数（用于增量更新）
 * ============================================================================ */

/**
 * @brief 判断寄存器是否为运行时可修改
 * @param bank 寄存器所在Bank
 * @param reg_addr 寄存器地址
 * @param old_value 旧值
 * @param new_value 新值
 * @return 1=可运行时修改, 0=需要关闭传感器
 * @note 只有以下寄存器的特定位可在运行时修改：
 *       - GYRO_CONFIG0: 全部位（GYRO_FS_SEL, GYRO_ODR）
 *       - ACCEL_CONFIG0: 全部位（ACCEL_FS_SEL, ACCEL_ODR）
 *       - PWR_MGMT0: 仅bits[3:0]（GYRO_MODE, ACCEL_MODE）
 */
static uint8_t ICM42688P_IsRuntimeModifiable(uint8_t bank, uint8_t reg_addr, uint8_t old_value,
                                              uint8_t new_value)
{
    if (bank != 0) {
        return 0; // 只有Bank 0有运行时可修改的寄存器
    }

    switch (reg_addr) {
    case 0x4F: // GYRO_CONFIG0 - 全部位可运行时修改
    case 0x50: // ACCEL_CONFIG0 - 全部位可运行时修改
        return 1;

    case 0x4E: // PWR_MGMT0 - 仅bits[3:0]可运行时修改
    {
        // 检查是否只有bits[3:0]发生变化（bits[7:4]必须不变）
        uint8_t old_other_bits = old_value & ~ICM42688P_PWR_MGMT0_RUNTIME_MASK;
        uint8_t new_other_bits = new_value & ~ICM42688P_PWR_MGMT0_RUNTIME_MASK;

        // 如果bits[7:4]也变化了，则不是运行时可修改
        if (old_other_bits != new_other_bits) {
            return 0;
        }

        // 如果只有bits[3:0]变化，则可运行时修改
        return 1;
    }

    default:
        return 0;
    }
}

/**
 * @brief 检查芯片配置与内部存储配置是否一致
 * @param chip_config 从芯片读取的配置
 * @param internal_config 内部存储的配置
 * @return 1=一致, 0=不一致
 * @note FIX 4: 优化一致性检查，提取为独立函数
 */
static uint8_t ICM42688P_CheckConfigConsistency(const ICM42688P_Config *chip_config,
                                                  const ICM42688P_Config *internal_config)
{
    const uint8_t *chip_bank0 = (const uint8_t *)&chip_config->bank0;
    const uint8_t *internal_bank0 = (const uint8_t *)&internal_config->bank0;
    const uint8_t *chip_bank1 = (const uint8_t *)&chip_config->bank1;
    const uint8_t *internal_bank1 = (const uint8_t *)&internal_config->bank1;
    const uint8_t *chip_bank2 = (const uint8_t *)&chip_config->bank2;
    const uint8_t *internal_bank2 = (const uint8_t *)&internal_config->bank2;
    const uint8_t *chip_bank4 = (const uint8_t *)&chip_config->bank4;
    const uint8_t *internal_bank4 = (const uint8_t *)&internal_config->bank4;

    // 检查Bank 0
    for (uint8_t i = 0; i < BANK0_REG_COUNT; i++) {
        if (chip_bank0[bank0_reg_map[i].offset_in_struct] !=
            internal_bank0[bank0_reg_map[i].offset_in_struct]) {
            return 0; // 不一致
        }
    }

    // 检查Bank 1
    for (uint8_t i = 0; i < BANK1_REG_COUNT; i++) {
        if (chip_bank1[bank1_reg_map[i].offset_in_struct] !=
            internal_bank1[bank1_reg_map[i].offset_in_struct]) {
            return 0;
        }
    }

    // 检查Bank 2
    for (uint8_t i = 0; i < BANK2_REG_COUNT; i++) {
        if (chip_bank2[bank2_reg_map[i].offset_in_struct] !=
            internal_bank2[bank2_reg_map[i].offset_in_struct]) {
            return 0;
        }
    }

    // 检查Bank 4
    for (uint8_t i = 0; i < BANK4_REG_COUNT; i++) {
        if (chip_bank4[bank4_reg_map[i].offset_in_struct] !=
            internal_bank4[bank4_reg_map[i].offset_in_struct]) {
            return 0;
        }
    }

    return 1; // 一致
}

/* ============================================================================
 * Level 2: 智能增量更新函数
 * ============================================================================ */

/**
 * @brief 对比两个配置，生成增量更新列表
 * @param old_cfg 旧配置
 * @param new_cfg 新配置
 * @param diff 输出的差异列表
 * @return 变化的寄存器数量
 * @note 此函数会遍历所有寄存器，找出差异并判断是否需要关闭传感器
 */
uint8_t ICM42688P_CompareConfig(const ICM42688P_Config *old_cfg,
                                const ICM42688P_Config *new_cfg, ICM42688P_ConfigDiff *diff)
{
    // 初始化diff结构
    diff->write_count = 0;
    diff->needs_power_off = 0;

    const uint8_t *old_bank0 = (const uint8_t *)&old_cfg->bank0;
    const uint8_t *new_bank0 = (const uint8_t *)&new_cfg->bank0;
    const uint8_t *old_bank1 = (const uint8_t *)&old_cfg->bank1;
    const uint8_t *new_bank1 = (const uint8_t *)&new_cfg->bank1;
    const uint8_t *old_bank2 = (const uint8_t *)&old_cfg->bank2;
    const uint8_t *new_bank2 = (const uint8_t *)&new_cfg->bank2;
    const uint8_t *old_bank4 = (const uint8_t *)&old_cfg->bank4;
    const uint8_t *new_bank4 = (const uint8_t *)&new_cfg->bank4;

    // 对比Bank 0寄存器
    for (uint8_t i = 0; i < BANK0_REG_COUNT; i++) {
        // FIX 1: 边界检查，防止数组越界
        if (diff->write_count >= 68) {
            break;
        }

        uint8_t old_val = old_bank0[bank0_reg_map[i].offset_in_struct];
        uint8_t new_val = new_bank0[bank0_reg_map[i].offset_in_struct];

        if (old_val != new_val) {
            // FIX 2: PWR_MGMT0特殊处理 - 不添加到writes列表，在Level 2函数中单独处理
            if (bank0_reg_map[i].reg_addr == 0x4E) {
                // 判断是否需要关闭传感器
                if (!ICM42688P_IsRuntimeModifiable(0, 0x4E, old_val, new_val)) {
                    diff->needs_power_off = 1;
                }
                continue; // 跳过PWR_MGMT0，不添加到writes列表
            }

            // 记录其他寄存器的差异
            diff->writes[diff->write_count].bank = 0;
            diff->writes[diff->write_count].reg_addr = bank0_reg_map[i].reg_addr;
            diff->writes[diff->write_count].value = new_val;
            diff->write_count++;

            // 判断是否需要关闭传感器
            if (!ICM42688P_IsRuntimeModifiable(0, bank0_reg_map[i].reg_addr, old_val, new_val)) {
                diff->needs_power_off = 1;
            }
        }
    }

    // 对比Bank 1寄存器
    for (uint8_t i = 0; i < BANK1_REG_COUNT; i++) {
        // FIX 1: 边界检查
        if (diff->write_count >= 68) {
            break;
        }

        uint8_t old_val = old_bank1[bank1_reg_map[i].offset_in_struct];
        uint8_t new_val = new_bank1[bank1_reg_map[i].offset_in_struct];

        if (old_val != new_val) {
            diff->writes[diff->write_count].bank = 1;
            diff->writes[diff->write_count].reg_addr = bank1_reg_map[i].reg_addr;
            diff->writes[diff->write_count].value = new_val;
            diff->write_count++;
            diff->needs_power_off = 1; // Bank 1寄存器都需要关闭传感器
        }
    }

    // 对比Bank 2寄存器
    for (uint8_t i = 0; i < BANK2_REG_COUNT; i++) {
        // FIX 1: 边界检查
        if (diff->write_count >= 68) {
            break;
        }

        uint8_t old_val = old_bank2[bank2_reg_map[i].offset_in_struct];
        uint8_t new_val = new_bank2[bank2_reg_map[i].offset_in_struct];

        if (old_val != new_val) {
            diff->writes[diff->write_count].bank = 2;
            diff->writes[diff->write_count].reg_addr = bank2_reg_map[i].reg_addr;
            diff->writes[diff->write_count].value = new_val;
            diff->write_count++;
            diff->needs_power_off = 1; // Bank 2寄存器都需要关闭传感器
        }
    }

    // 对比Bank 4寄存器
    for (uint8_t i = 0; i < BANK4_REG_COUNT; i++) {
        // FIX 1: 边界检查
        if (diff->write_count >= 68) {
            break;
        }

        uint8_t old_val = old_bank4[bank4_reg_map[i].offset_in_struct];
        uint8_t new_val = new_bank4[bank4_reg_map[i].offset_in_struct];

        if (old_val != new_val) {
            diff->writes[diff->write_count].bank = 4;
            diff->writes[diff->write_count].reg_addr = bank4_reg_map[i].reg_addr;
            diff->writes[diff->write_count].value = new_val;
            diff->write_count++;
            diff->needs_power_off = 1; // Bank 4寄存器都需要关闭传感器
        }
    }

    return diff->write_count;
}

/**
 * @brief 智能增量更新配置（Level 2）
 * @param new_config 新配置结构体指针
 * @return 状态码: bit0=0成功/1失败, bit1=0配置一致/1有不一致警告
 * @note 自动检测变化的寄存器并增量更新，根据需要自动关闭/开启传感器
 *       如果检测到芯片配置与内部存储不一致会发出警告但继续执行
 */
uint8_t ICM42688P_ApplyConfigIncremental(const ICM42688P_Config *new_config)
{
    uint8_t result = 0; // bit0=失败标志, bit1=不一致警告
    uint8_t error = 0;
    ICM42688P_Config chip_config;

    // 1. 如果内部配置未初始化，使用全局ApplyConfig
    if (!g_config_initialized) {
        error = ICM42688P_ApplyConfig(new_config);
        return error ? 0x01 : 0x00;
    }

    // 2. 验证新配置有效性
    if (!ICM42688P_ValidateConfig(new_config)) {
        return 0x01; // 配置无效，返回失败
    }

    // 3. 读取芯片当前配置（调用统一的读取函数，避免代码重复）
    ICM42688P_ReadAllConfigRegisters(&chip_config);

    // 4. FIX 4: 使用优化的一致性检查函数
    if (!ICM42688P_CheckConfigConsistency(&chip_config, &g_internal_config)) {
        result |= 0x02; // 设置不一致警告标志
    }

    // 5. 对比内部配置与新配置，生成增量更新列表
    ICM42688P_CompareConfig(&g_internal_config, new_config, &g_diff);

    // 检查是否有PWR_MGMT0变化（需要单独处理）
    uint8_t pwr_mgmt0_changed =
        (g_internal_config.bank0.PWR_MGMT0 != new_config->bank0.PWR_MGMT0);

    // 如果没有变化（包括PWR_MGMT0），直接返回
    if (g_diff.write_count == 0 && !pwr_mgmt0_changed) {
        return result;
    }

    // 6. 如果需要关闭传感器，先关闭
    // FIX 3: 删除未使用的saved_pwr_mgmt0变量
    if (g_diff.needs_power_off) {
        // 关闭传感器（GYRO_MODE=00, ACCEL_MODE=00）
        ICM42688P_Bank_Select(0);
        uint8_t power_off = 0x00;
        error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &power_off, 1);
        if (error) {
            result |= 0x01;
        }

        // 延时2ms（200μs × 10倍）
        delay_ms(2);
    }

    // 7. 按顺序写入变化的寄存器（不包括PWR_MGMT0）
    uint8_t current_bank = 0;
    ICM42688P_Bank_Select(0);

    for (uint8_t i = 0; i < g_diff.write_count; i++) {
        // 切换Bank（如果需要）
        if (g_diff.writes[i].bank != current_bank) {
            current_bank = g_diff.writes[i].bank;
            ICM42688P_Bank_Select(current_bank);
        }

        // 写入寄存器
        error = ICM42688P_WriteRegister(g_diff.writes[i].reg_addr, &g_diff.writes[i].value, 1);
        if (error) {
            result |= 0x01; // 记录写入错误，但继续执行
        }

        // 根据寄存器类型决定是否延时
        uint8_t delay = ICM42688P_GetRegisterDelay(g_diff.writes[i].bank, g_diff.writes[i].reg_addr);
        if (delay > 0) {
            delay_ms(delay);
        }
    }

    // 8. FIX 3: 总是检查并写入PWR_MGMT0（如果有变化）
    if (pwr_mgmt0_changed) {
        ICM42688P_Bank_Select(0);
        uint8_t new_pwr_mgmt0 = new_config->bank0.PWR_MGMT0;
        error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &new_pwr_mgmt0, 1);
        if (error) {
            result |= 0x01;
        }

        // 只有在关闭了传感器的情况下才需要延时50ms
        // 运行时修改（仅bits[3:0]变化）不需要延时
        if (g_diff.needs_power_off) {
            delay_ms(50);
        }
    }

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    // 9. 更新内部存储配置
    memcpy(&g_internal_config, new_config, sizeof(ICM42688P_Config));

    return result;
}

/* ============================================================================
 * Level 3: 运行时快速修改函数（最高实时性）
 * ============================================================================ */

/**
 * @brief 运行时修改陀螺仪ODR（Level 3 - 最快）
 * @param odr 输出数据率: 0110=1kHz, 0111=200Hz, 1000=100Hz等
 * @return 0=成功, 非0=失败（可能芯片损坏）
 * @note 可在传感器运行时调用，无延时，最快速度
 *       失败可能表明芯片忙或损坏，请检查硬件
 */
uint8_t ICM42688P_SetGyroODR(uint8_t odr)
{
    uint8_t error;

    // 确保内部配置已初始化
    ICM42688P_EnsureConfigInitialized();

    // 读取当前GYRO_CONFIG0值
    uint8_t config0;
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0, &config0, 1);

    // 修改ODR位（bits 3:0），保持FS_SEL不变（bits 7:5）
    config0 = (config0 & 0xF0) | (odr & 0x0F);

    // 写入寄存器（内建写后读验证）
    error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0, &config0, 1);

    // 更新内部配置
    if (!error) {
        g_internal_config.bank0.GYRO_CONFIG0 = config0;
    }

    return error;
}

/**
 * @brief 运行时修改陀螺仪满量程范围（Level 3 - 最快）
 * @param fs_sel 满量程选择: 000=±2000dps, 001=±1000dps, 010=±500dps等
 * @return 0=成功, 非0=失败（可能芯片损坏）
 * @note 可在传感器运行时调用，无延时，最快速度
 *       失败可能表明芯片忙或损坏，请检查硬件
 */
uint8_t ICM42688P_SetGyroFSR(uint8_t fs_sel)
{
    uint8_t error;

    // 确保内部配置已初始化
    ICM42688P_EnsureConfigInitialized();

    // 读取当前GYRO_CONFIG0值
    uint8_t config0;
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0, &config0, 1);

    // 修改FS_SEL位（bits 7:5），保持ODR不变（bits 3:0）
    config0 = (config0 & 0x0F) | ((fs_sel & 0x07) << 5);

    // 写入寄存器（内建写后读验证）
    error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0, &config0, 1);

    // 更新内部配置
    if (!error) {
        g_internal_config.bank0.GYRO_CONFIG0 = config0;
    }

    return error;
}

/**
 * @brief 运行时修改陀螺仪电源模式（Level 3 - 最快）
 * @param mode 陀螺仪模式: 00=OFF, 01=Standby, 11=LN
 * @return 0=成功, 非0=失败（可能芯片损坏）
 * @note 可在传感器运行时调用，无延时，最快速度
 *       失败可能表明芯片忙或损坏，请检查硬件
 */
uint8_t ICM42688P_SetGyroMode(uint8_t mode)
{
    uint8_t error;

    // 确保内部配置已初始化
    ICM42688P_EnsureConfigInitialized();

    // 读取当前PWR_MGMT0值
    uint8_t pwr_mgmt0;
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &pwr_mgmt0, 1);

    // 修改GYRO_MODE位（bits 3:2），保持其他位不变
    pwr_mgmt0 = (pwr_mgmt0 & 0xF3) | ((mode & 0x03) << 2);

    // 写入寄存器（内建写后读验证）
    error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &pwr_mgmt0, 1);

    // 更新内部配置
    if (!error) {
        g_internal_config.bank0.PWR_MGMT0 = pwr_mgmt0;
    }

    return error;
}

/**
 * @brief 运行时修改加速度计ODR（Level 3 - 最快）
 * @param odr 输出数据率: 0110=1kHz, 0111=200Hz, 1000=100Hz等
 * @return 0=成功, 非0=失败（可能芯片损坏）
 * @note 可在传感器运行时调用，无延时，最快速度
 *       失败可能表明芯片忙或损坏，请检查硬件
 */
uint8_t ICM42688P_SetAccelODR(uint8_t odr)
{
    uint8_t error;

    // 确保内部配置已初始化
    ICM42688P_EnsureConfigInitialized();

    // 读取当前ACCEL_CONFIG0值
    uint8_t config0;
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0, &config0, 1);

    // 修改ODR位（bits 3:0），保持FS_SEL不变（bits 7:5）
    config0 = (config0 & 0xF0) | (odr & 0x0F);

    // 写入寄存器（内建写后读验证）
    error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0, &config0, 1);

    // 更新内部配置
    if (!error) {
        g_internal_config.bank0.ACCEL_CONFIG0 = config0;
    }

    return error;
}

/**
 * @brief 运行时修改加速度计满量程范围（Level 3 - 最快）
 * @param fs_sel 满量程选择: 000=±16g, 001=±8g, 010=±4g, 011=±2g
 * @return 0=成功, 非0=失败（可能芯片损坏）
 * @note 可在传感器运行时调用，无延时，最快速度
 *       失败可能表明芯片忙或损坏，请检查硬件
 */
uint8_t ICM42688P_SetAccelFSR(uint8_t fs_sel)
{
    uint8_t error;

    // 确保内部配置已初始化
    ICM42688P_EnsureConfigInitialized();

    // 读取当前ACCEL_CONFIG0值
    uint8_t config0;
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0, &config0, 1);

    // 修改FS_SEL位（bits 7:5），保持ODR不变（bits 3:0）
    config0 = (config0 & 0x0F) | ((fs_sel & 0x07) << 5);

    // 写入寄存器（内建写后读验证）
    error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0, &config0, 1);

    // 更新内部配置
    if (!error) {
        g_internal_config.bank0.ACCEL_CONFIG0 = config0;
    }

    return error;
}

/**
 * @brief 运行时修改加速度计电源模式（Level 3 - 最快）
 * @param mode 加速度计模式: 00=OFF, 10=LP, 11=LN
 * @return 0=成功, 非0=失败（可能芯片损坏）
 * @note 可在传感器运行时调用，无延时，最快速度
 *       失败可能表明芯片忙或损坏，请检查硬件
 */
uint8_t ICM42688P_SetAccelMode(uint8_t mode)
{
    uint8_t error;

    // 确保内部配置已初始化
    ICM42688P_EnsureConfigInitialized();

    // 读取当前PWR_MGMT0值
    uint8_t pwr_mgmt0;
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &pwr_mgmt0, 1);

    // 修改ACCEL_MODE位（bits 1:0），保持其他位不变
    pwr_mgmt0 = (pwr_mgmt0 & 0xFC) | (mode & 0x03);

    // 写入寄存器（内建写后读验证）
    error = ICM42688P_WriteRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &pwr_mgmt0, 1);

    // 更新内部配置
    if (!error) {
        g_internal_config.bank0.PWR_MGMT0 = pwr_mgmt0;
    }

    return error;
}
