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

// 延时函数声明
extern void delay_ms(uint32_t ms);

/* ============================================================================
 * 静态辅助函数
 * ============================================================================ */

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
    memset(config, 0, sizeof(ICM42688P_Config));
    config->magic = ICM42688P_CONFIG_MAGIC;
    config->version = ICM42688P_CONFIG_VERSION;
    ICM42688P_LoadDefaultConfig(config);
}

/**
 * @brief 读取芯片中所有配置寄存器的初始值
 * @param config 配置结构体指针，用于存储读取的寄存器值
 * @return 0=成功, 非0=读取错误
 * @note 此函数用于验证默认配置值是否正确，通过读取芯片实际寄存器值来确认
 */
uint8_t ICM42688P_ReadAllConfigRegisters(ICM42688P_Config *config)
{
    uint8_t error = 0;

    // 初始化配置结构体
    memset(config, 0, sizeof(ICM42688P_Config));
    config->magic = ICM42688P_CONFIG_MAGIC;
    config->version = ICM42688P_CONFIG_VERSION;

    // Bank 0 寄存器读取
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_DEVICE_CONFIG, &config->bank0.DEVICE_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_DRIVE_CONFIG, &config->bank0.DRIVE_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_CONFIG, &config->bank0.INT_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG, &config->bank0.FIFO_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INTF_CONFIG0, &config->bank0.INTF_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INTF_CONFIG1, &config->bank0.INTF_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &config->bank0.PWR_MGMT0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0, &config->bank0.GYRO_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0, &config->bank0.ACCEL_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_CONFIG1, &config->bank0.GYRO_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_ACCEL_CONFIG0,
                           &config->bank0.GYRO_ACCEL_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG1, &config->bank0.ACCEL_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_TMST_CONFIG, &config->bank0.TMST_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_SMD_CONFIG, &config->bank0.SMD_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG1, &config->bank0.FIFO_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG2, &config->bank0.FIFO_CONFIG2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG3, &config->bank0.FIFO_CONFIG3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FSYNC_CONFIG, &config->bank0.FSYNC_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_CONFIG0, &config->bank0.INT_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_CONFIG1, &config->bank0.INT_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE0, &config->bank0.INT_SOURCE0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE1, &config->bank0.INT_SOURCE1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE3, &config->bank0.INT_SOURCE3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE4, &config->bank0.INT_SOURCE4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_SELF_TEST_CONFIG, &config->bank0.SELF_TEST_CONFIG,
                           1);

    // Bank 1 寄存器读取
    ICM42688P_Bank_Select(1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_SENSOR_CONFIG0, &config->bank1.SENSOR_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC2,
                           &config->bank1.GYRO_CONFIG_STATIC2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC3,
                           &config->bank1.GYRO_CONFIG_STATIC3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC4,
                           &config->bank1.GYRO_CONFIG_STATIC4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC5,
                           &config->bank1.GYRO_CONFIG_STATIC5, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC6,
                           &config->bank1.GYRO_CONFIG_STATIC6, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC7,
                           &config->bank1.GYRO_CONFIG_STATIC7, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC8,
                           &config->bank1.GYRO_CONFIG_STATIC8, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC9,
                           &config->bank1.GYRO_CONFIG_STATIC9, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC10,
                           &config->bank1.GYRO_CONFIG_STATIC10, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_INTF_CONFIG4, &config->bank1.INTF_CONFIG4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_INTF_CONFIG5, &config->bank1.INTF_CONFIG5, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_INTF_CONFIG6, &config->bank1.INTF_CONFIG6, 1);

    // Bank 2 寄存器读取
    ICM42688P_Bank_Select(2);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC2,
                           &config->bank2.ACCEL_CONFIG_STATIC2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC3,
                           &config->bank2.ACCEL_CONFIG_STATIC3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC4,
                           &config->bank2.ACCEL_CONFIG_STATIC4, 1);

    // Bank 4 寄存器读取
    ICM42688P_Bank_Select(4);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_ACCEL_WOM_X_THR, &config->bank4.ACCEL_WOM_X_THR, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_ACCEL_WOM_Y_THR, &config->bank4.ACCEL_WOM_Y_THR, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_ACCEL_WOM_Z_THR, &config->bank4.ACCEL_WOM_Z_THR, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER0, &config->bank4.OFFSET_USER0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER1, &config->bank4.OFFSET_USER1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER2, &config->bank4.OFFSET_USER2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER3, &config->bank4.OFFSET_USER3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER4, &config->bank4.OFFSET_USER4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER5, &config->bank4.OFFSET_USER5, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER6, &config->bank4.OFFSET_USER6, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER7, &config->bank4.OFFSET_USER7, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER8, &config->bank4.OFFSET_USER8, 1);

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    // 计算校验和
    config->checksum = ICM42688P_CalculateChecksum(config);

    // 注意：由于ICM42688P_ReadRegister返回void，这里无法检测读取错误
    // 在实际应用中，您可能需要修改底层驱动来返回错误状态
    return error;
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
    if (buffer_size < 2500) {
        return 0; // Buffer too small
    }

    char *ptr = buffer;
    int written = 0;

    // Header information
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "ICM42688P Configuration Register Values\n"
                       "========================================\n"
                       "Magic: 0x%08X, Version: 0x%04X, Checksum: 0x%04X\n\n",
                       config->magic, config->version, config->checksum);
    ptr += written;

    // Bank 0 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 0 Registers:\n"
                       "----------------\n");
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
                 "SELF_TEST_CONFIG (0x70): 0x%02X\n\n",
                 config->bank0.DEVICE_CONFIG, config->bank0.DRIVE_CONFIG, config->bank0.INT_CONFIG,
                 config->bank0.FIFO_CONFIG, config->bank0.INTF_CONFIG0, config->bank0.INTF_CONFIG1,
                 config->bank0.PWR_MGMT0, config->bank0.GYRO_CONFIG0, config->bank0.ACCEL_CONFIG0,
                 config->bank0.GYRO_CONFIG1, config->bank0.GYRO_ACCEL_CONFIG0,
                 config->bank0.ACCEL_CONFIG1, config->bank0.TMST_CONFIG, config->bank0.SMD_CONFIG,
                 config->bank0.FIFO_CONFIG1, config->bank0.FIFO_CONFIG2, config->bank0.FIFO_CONFIG3,
                 config->bank0.FSYNC_CONFIG, config->bank0.INT_CONFIG0, config->bank0.INT_CONFIG1,
                 config->bank0.INT_SOURCE0, config->bank0.INT_SOURCE1, config->bank0.INT_SOURCE3,
                 config->bank0.INT_SOURCE4, config->bank0.SELF_TEST_CONFIG);
    ptr += written;

    // Bank 1 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 1 Registers:\n"
                       "----------------\n");
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
    ptr += written;

    // Bank 2 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 2 Registers:\n"
                       "----------------\n");
    ptr += written;

    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "ACCEL_CONFIG_STATIC2 (0x03): 0x%02X\n"
                       "ACCEL_CONFIG_STATIC3 (0x04): 0x%02X\n"
                       "ACCEL_CONFIG_STATIC4 (0x05): 0x%02X\n\n",
                       config->bank2.ACCEL_CONFIG_STATIC2, config->bank2.ACCEL_CONFIG_STATIC3,
                       config->bank2.ACCEL_CONFIG_STATIC4);
    ptr += written;

    // Bank 4 registers
    written = snprintf(ptr, buffer_size - (ptr - buffer),
                       "Bank 4 Registers:\n"
                       "----------------\n");
    ptr += written;

    written = snprintf(
        ptr, buffer_size - (ptr - buffer),
        "ACCEL_WOM_X_THR (0x4A): 0x%02X | ACCEL_WOM_Y_THR (0x4B): 0x%02X\n"
        "ACCEL_WOM_Z_THR (0x4C): 0x%02X | OFFSET_USER0    (0x77): 0x%02X\n"
        "OFFSET_USER1    (0x78): 0x%02X | OFFSET_USER2    (0x79): 0x%02X\n"
        "OFFSET_USER3    (0x7A): 0x%02X | OFFSET_USER4    (0x7B): 0x%02X\n"
        "OFFSET_USER5    (0x7C): 0x%02X | OFFSET_USER6    (0x7D): 0x%02X\n"
        "OFFSET_USER7    (0x7E): 0x%02X | OFFSET_USER8    (0x7F): 0x%02X\n\n",
        config->bank4.ACCEL_WOM_X_THR, config->bank4.ACCEL_WOM_Y_THR, config->bank4.ACCEL_WOM_Z_THR,
        config->bank4.OFFSET_USER0, config->bank4.OFFSET_USER1, config->bank4.OFFSET_USER2,
        config->bank4.OFFSET_USER3, config->bank4.OFFSET_USER4, config->bank4.OFFSET_USER5,
        config->bank4.OFFSET_USER6, config->bank4.OFFSET_USER7, config->bank4.OFFSET_USER8);
    ptr += written;

    return (uint16_t)(ptr - buffer);
}

void ICM42688P_LoadDefaultConfig(ICM42688P_Config *config)
{
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
    config->bank4.ACCEL_WOM_X_THR = 0x00;
    config->bank4.ACCEL_WOM_Y_THR = 0x00;
    config->bank4.ACCEL_WOM_Z_THR = 0x00;
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
    config->bank0.PWR_MGMT0 = ((temp_disable & 0x01) << 5) | ((idle_mode & 0x01) << 4) |
                              ((gyro_mode & 0x03) << 2) | (accel_mode & 0x03);
}

void ICM42688P_ConfigGyro(ICM42688P_Config *config, uint8_t fs_sel, uint8_t odr,
                          uint8_t ui_filt_ord, uint8_t ui_filt_bw)
{
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
    config->bank0.INT_CONFIG = ((int2_mode & 0x01) << 5) | ((int2_drive & 0x01) << 4) |
                               ((int2_polarity & 0x01) << 3) | ((int1_mode & 0x01) << 2) |
                               ((int1_drive & 0x01) << 1) | (int1_polarity & 0x01);
}

void ICM42688P_ConfigInterruptSource(ICM42688P_Config *config, uint8_t int1_sources,
                                     uint8_t int2_sources)
{
    config->bank0.INT_SOURCE0 = int1_sources;
    config->bank0.INT_SOURCE3 = int2_sources;
}

void ICM42688P_ConfigGyroAAF(ICM42688P_Config *config, uint8_t enable, uint8_t delt,
                             uint16_t deltsqr, uint8_t bitshift)
{
    // GYRO_CONFIG_STATIC2: AAF_DIS[1]
    config->bank1.GYRO_CONFIG_STATIC2 =
        (config->bank1.GYRO_CONFIG_STATIC2 & 0xFD) | ((enable & 0x01) << 1);

    // GYRO_CONFIG_STATIC3: AAF_DELT[5:0]
    config->bank1.GYRO_CONFIG_STATIC3 = delt & 0x3F;

    // GYRO_CONFIG_STATIC4: AAF_DELTSQR[7:0]
    config->bank1.GYRO_CONFIG_STATIC4 = deltsqr & 0xFF;

    // GYRO_CONFIG_STATIC5: AAF_BITSHIFT[7:4] | AAF_DELTSQR[11:8]
    config->bank1.GYRO_CONFIG_STATIC5 = ((bitshift & 0x0F) << 4) | ((deltsqr >> 8) & 0x0F);
}

void ICM42688P_ConfigAccelAAF(ICM42688P_Config *config, uint8_t enable, uint8_t delt,
                              uint16_t deltsqr, uint8_t bitshift)
{
    // ACCEL_CONFIG_STATIC2: AAF_DELT[6:1] | AAF_DIS[0]
    config->bank2.ACCEL_CONFIG_STATIC2 = ((delt & 0x3F) << 1) | (enable & 0x01);

    // ACCEL_CONFIG_STATIC3: AAF_DELTSQR[7:0]
    config->bank2.ACCEL_CONFIG_STATIC3 = deltsqr & 0xFF;

    // ACCEL_CONFIG_STATIC4: AAF_BITSHIFT[7:4] | AAF_DELTSQR[11:8]
    config->bank2.ACCEL_CONFIG_STATIC4 = ((bitshift & 0x0F) << 4) | ((deltsqr >> 8) & 0x0F);
}

void ICM42688P_ConfigGyroOffset(ICM42688P_Config *config, int16_t offset_x, int16_t offset_y,
                                int16_t offset_z)
{
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
    // INTF_CONFIG1: RTC_MODE[2] | CLKSEL[1:0]
    config->bank0.INTF_CONFIG1 =
        (config->bank0.INTF_CONFIG1 & 0xF8) | ((rtc_mode & 0x01) << 2) | (clk_sel & 0x03);

    // INTF_CONFIG5: PIN9_FUNCTION[2:1]
    config->bank1.INTF_CONFIG5 = (config->bank1.INTF_CONFIG5 & 0xF9) | ((pin9_func & 0x03) << 1);
}

void ICM42688P_ConfigTemperature(ICM42688P_Config *config, uint8_t filt_bw)
{
    // GYRO_CONFIG1: TEMP_FILT_BW[7:5]
    config->bank0.GYRO_CONFIG1 = (config->bank0.GYRO_CONFIG1 & 0x1F) | ((filt_bw & 0x07) << 5);
}

void ICM42688P_ConfigFSYNC(ICM42688P_Config *config, uint8_t ui_sel, uint8_t polarity,
                           uint8_t flag_clear_sel)
{
    config->bank0.FSYNC_CONFIG =
        ((ui_sel & 0x07) << 4) | ((flag_clear_sel & 0x01) << 1) | (polarity & 0x01);
}

void ICM42688P_ConfigTimestamp(ICM42688P_Config *config, uint8_t enable, uint8_t resolution,
                               uint8_t delta_en, uint8_t fsync_en)
{
    config->bank0.TMST_CONFIG = ((resolution & 0x01) << 3) | ((delta_en & 0x01) << 2) |
                                ((fsync_en & 0x01) << 1) | (enable & 0x01);
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

    // Bank 0配置
    ICM42688P_Bank_Select(0);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_DEVICE_CONFIG,
                                     (uint8_t *)&config->bank0.DEVICE_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_DRIVE_CONFIG,
                                     (uint8_t *)&config->bank0.DRIVE_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_CONFIG,
                                     (uint8_t *)&config->bank0.INT_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_FIFO_CONFIG,
                                     (uint8_t *)&config->bank0.FIFO_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INTF_CONFIG0,
                                     (uint8_t *)&config->bank0.INTF_CONFIG0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INTF_CONFIG1,
                                     (uint8_t *)&config->bank0.INTF_CONFIG1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_PWR_MGMT0,
                                     (uint8_t *)&config->bank0.PWR_MGMT0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0,
                                     (uint8_t *)&config->bank0.GYRO_CONFIG0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0,
                                     (uint8_t *)&config->bank0.ACCEL_CONFIG0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_GYRO_CONFIG1,
                                     (uint8_t *)&config->bank0.GYRO_CONFIG1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_GYRO_ACCEL_CONFIG0,
                                     (uint8_t *)&config->bank0.GYRO_ACCEL_CONFIG0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG1,
                                     (uint8_t *)&config->bank0.ACCEL_CONFIG1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_TMST_CONFIG,
                                     (uint8_t *)&config->bank0.TMST_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_SMD_CONFIG,
                                     (uint8_t *)&config->bank0.SMD_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_FIFO_CONFIG1,
                                     (uint8_t *)&config->bank0.FIFO_CONFIG1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_FIFO_CONFIG2,
                                     (uint8_t *)&config->bank0.FIFO_CONFIG2, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_FIFO_CONFIG3,
                                     (uint8_t *)&config->bank0.FIFO_CONFIG3, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_FSYNC_CONFIG,
                                     (uint8_t *)&config->bank0.FSYNC_CONFIG, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_CONFIG0,
                                     (uint8_t *)&config->bank0.INT_CONFIG0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_CONFIG1,
                                     (uint8_t *)&config->bank0.INT_CONFIG1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_SOURCE0,
                                     (uint8_t *)&config->bank0.INT_SOURCE0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_SOURCE1,
                                     (uint8_t *)&config->bank0.INT_SOURCE1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_SOURCE3,
                                     (uint8_t *)&config->bank0.INT_SOURCE3, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_INT_SOURCE4,
                                     (uint8_t *)&config->bank0.INT_SOURCE4, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK0_SELF_TEST_CONFIG,
                                     (uint8_t *)&config->bank0.SELF_TEST_CONFIG, 1);

    // Bank 1配置
    ICM42688P_Bank_Select(1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_SENSOR_CONFIG0,
                                     (uint8_t *)&config->bank1.SENSOR_CONFIG0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC2,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC2, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC3,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC3, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC4,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC4, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC5,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC5, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC6,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC6, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC7,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC7, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC8,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC8, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC9,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC9, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC10,
                                     (uint8_t *)&config->bank1.GYRO_CONFIG_STATIC10, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_INTF_CONFIG4,
                                     (uint8_t *)&config->bank1.INTF_CONFIG4, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_INTF_CONFIG5,
                                     (uint8_t *)&config->bank1.INTF_CONFIG5, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK1_INTF_CONFIG6,
                                     (uint8_t *)&config->bank1.INTF_CONFIG6, 1);

    // Bank 2配置
    ICM42688P_Bank_Select(2);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC2,
                                     (uint8_t *)&config->bank2.ACCEL_CONFIG_STATIC2, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC3,
                                     (uint8_t *)&config->bank2.ACCEL_CONFIG_STATIC3, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC4,
                                     (uint8_t *)&config->bank2.ACCEL_CONFIG_STATIC4, 1);

    // Bank 4配置
    ICM42688P_Bank_Select(4);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_ACCEL_WOM_X_THR,
                                     (uint8_t *)&config->bank4.ACCEL_WOM_X_THR, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_ACCEL_WOM_Y_THR,
                                     (uint8_t *)&config->bank4.ACCEL_WOM_Y_THR, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_ACCEL_WOM_Z_THR,
                                     (uint8_t *)&config->bank4.ACCEL_WOM_Z_THR, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER0,
                                     (uint8_t *)&config->bank4.OFFSET_USER0, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER1,
                                     (uint8_t *)&config->bank4.OFFSET_USER1, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER2,
                                     (uint8_t *)&config->bank4.OFFSET_USER2, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER3,
                                     (uint8_t *)&config->bank4.OFFSET_USER3, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER4,
                                     (uint8_t *)&config->bank4.OFFSET_USER4, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER5,
                                     (uint8_t *)&config->bank4.OFFSET_USER5, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER6,
                                     (uint8_t *)&config->bank4.OFFSET_USER6, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER7,
                                     (uint8_t *)&config->bank4.OFFSET_USER7, 1);
    error |= ICM42688P_WriteRegister(ICM42688P_REG_BANK4_OFFSET_USER8,
                                     (uint8_t *)&config->bank4.OFFSET_USER8, 1);

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    return error;
}

uint8_t ICM42688P_ReadConfig(ICM42688P_Config *config)
{
    // Bank 0读取
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_DEVICE_CONFIG, &config->bank0.DEVICE_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_DRIVE_CONFIG, &config->bank0.DRIVE_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_CONFIG, &config->bank0.INT_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG, &config->bank0.FIFO_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INTF_CONFIG0, &config->bank0.INTF_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INTF_CONFIG1, &config->bank0.INTF_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_PWR_MGMT0, &config->bank0.PWR_MGMT0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_CONFIG0, &config->bank0.GYRO_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG0, &config->bank0.ACCEL_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_CONFIG1, &config->bank0.GYRO_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_GYRO_ACCEL_CONFIG0,
                           &config->bank0.GYRO_ACCEL_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_ACCEL_CONFIG1, &config->bank0.ACCEL_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_TMST_CONFIG, &config->bank0.TMST_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_SMD_CONFIG, &config->bank0.SMD_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG1, &config->bank0.FIFO_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG2, &config->bank0.FIFO_CONFIG2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FIFO_CONFIG3, &config->bank0.FIFO_CONFIG3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_FSYNC_CONFIG, &config->bank0.FSYNC_CONFIG, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_CONFIG0, &config->bank0.INT_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_CONFIG1, &config->bank0.INT_CONFIG1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE0, &config->bank0.INT_SOURCE0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE1, &config->bank0.INT_SOURCE1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE3, &config->bank0.INT_SOURCE3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_INT_SOURCE4, &config->bank0.INT_SOURCE4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK0_SELF_TEST_CONFIG, &config->bank0.SELF_TEST_CONFIG,
                           1);

    // Bank 1读取
    ICM42688P_Bank_Select(1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_SENSOR_CONFIG0, &config->bank1.SENSOR_CONFIG0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC2,
                           &config->bank1.GYRO_CONFIG_STATIC2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC3,
                           &config->bank1.GYRO_CONFIG_STATIC3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC4,
                           &config->bank1.GYRO_CONFIG_STATIC4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC5,
                           &config->bank1.GYRO_CONFIG_STATIC5, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC6,
                           &config->bank1.GYRO_CONFIG_STATIC6, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC7,
                           &config->bank1.GYRO_CONFIG_STATIC7, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC8,
                           &config->bank1.GYRO_CONFIG_STATIC8, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC9,
                           &config->bank1.GYRO_CONFIG_STATIC9, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC10,
                           &config->bank1.GYRO_CONFIG_STATIC10, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_INTF_CONFIG4, &config->bank1.INTF_CONFIG4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_INTF_CONFIG5, &config->bank1.INTF_CONFIG5, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK1_INTF_CONFIG6, &config->bank1.INTF_CONFIG6, 1);

    // Bank 2读取
    ICM42688P_Bank_Select(2);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC2,
                           &config->bank2.ACCEL_CONFIG_STATIC2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC3,
                           &config->bank2.ACCEL_CONFIG_STATIC3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC4,
                           &config->bank2.ACCEL_CONFIG_STATIC4, 1);

    // Bank 4读取
    ICM42688P_Bank_Select(4);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_ACCEL_WOM_X_THR, &config->bank4.ACCEL_WOM_X_THR, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_ACCEL_WOM_Y_THR, &config->bank4.ACCEL_WOM_Y_THR, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_ACCEL_WOM_Z_THR, &config->bank4.ACCEL_WOM_Z_THR, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER0, &config->bank4.OFFSET_USER0, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER1, &config->bank4.OFFSET_USER1, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER2, &config->bank4.OFFSET_USER2, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER3, &config->bank4.OFFSET_USER3, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER4, &config->bank4.OFFSET_USER4, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER5, &config->bank4.OFFSET_USER5, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER6, &config->bank4.OFFSET_USER6, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER7, &config->bank4.OFFSET_USER7, 1);
    ICM42688P_ReadRegister(ICM42688P_REG_BANK4_OFFSET_USER8, &config->bank4.OFFSET_USER8, 1);

    // 切回Bank 0
    ICM42688P_Bank_Select(0);

    // 更新魔数、版本和校验和
    config->magic = ICM42688P_CONFIG_MAGIC;
    config->version = ICM42688P_CONFIG_VERSION;
    config->checksum = ICM42688P_CalculateChecksum(config);

    return 0;
}

/* ============================================================================
 * 序列化函数
 * ============================================================================ */

uint16_t ICM42688P_ExportConfig(const ICM42688P_Config *config, uint8_t *buffer,
                                uint16_t buffer_size)
{
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
    if (buffer_size < sizeof(ICM42688P_Config)) {
        return 1; // 数据不完整
    }

    // 从缓冲区复制到配置结构
    memcpy(config, buffer, sizeof(ICM42688P_Config));

    // 验证配置
    if (!ICM42688P_ValidateConfig(config)) {
        return 2; // 配置无效
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

    // 验证校验和
    uint16_t calculated = ICM42688P_CalculateChecksum(config);
    if (config->checksum != calculated) {
        return 0;
    }

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
