/**
 ******************************************************************************
 * @file    ICM42688P_Config.h
 * @brief   ICM-42688-P配置管理库头文件
 * @note    提供完整的寄存器配置管理、序列化和EEPROM存储功能
 ******************************************************************************
 */

#ifndef ICM42688P_CONFIG_H
#define ICM42688P_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * 配置常量定义
 * ============================================================================ */
#define ICM42688P_CONFIG_MAGIC      0x49434D42  // "ICMB"
#define ICM42688P_CONFIG_VERSION    0x0100      // v1.0

/* ============================================================================
 * Bank 0 寄存器地址定义
 * ============================================================================ */
#define ICM42688P_REG_BANK0_DEVICE_CONFIG       0x11
#define ICM42688P_REG_BANK0_DRIVE_CONFIG        0x13
#define ICM42688P_REG_BANK0_INT_CONFIG          0x14
#define ICM42688P_REG_BANK0_FIFO_CONFIG         0x16
#define ICM42688P_REG_BANK0_INTF_CONFIG0        0x4C
#define ICM42688P_REG_BANK0_INTF_CONFIG1        0x4D
#define ICM42688P_REG_BANK0_PWR_MGMT0           0x4E
#define ICM42688P_REG_BANK0_GYRO_CONFIG0        0x4F
#define ICM42688P_REG_BANK0_ACCEL_CONFIG0       0x50
#define ICM42688P_REG_BANK0_GYRO_CONFIG1        0x51
#define ICM42688P_REG_BANK0_GYRO_ACCEL_CONFIG0  0x52
#define ICM42688P_REG_BANK0_ACCEL_CONFIG1       0x53
#define ICM42688P_REG_BANK0_TMST_CONFIG         0x54
#define ICM42688P_REG_BANK0_SMD_CONFIG          0x57
#define ICM42688P_REG_BANK0_FIFO_CONFIG1        0x5F
#define ICM42688P_REG_BANK0_FIFO_CONFIG2        0x60
#define ICM42688P_REG_BANK0_FIFO_CONFIG3        0x61
#define ICM42688P_REG_BANK0_FSYNC_CONFIG        0x62
#define ICM42688P_REG_BANK0_INT_CONFIG0         0x63
#define ICM42688P_REG_BANK0_INT_CONFIG1         0x64
#define ICM42688P_REG_BANK0_INT_SOURCE0         0x65
#define ICM42688P_REG_BANK0_INT_SOURCE1         0x66
#define ICM42688P_REG_BANK0_INT_SOURCE3         0x68
#define ICM42688P_REG_BANK0_INT_SOURCE4         0x69
#define ICM42688P_REG_BANK0_SELF_TEST_CONFIG    0x70

/* ============================================================================
 * Bank 1 寄存器地址定义
 * ============================================================================ */
#define ICM42688P_REG_BANK1_SENSOR_CONFIG0          0x03
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC2     0x0B
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC3     0x0C
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC4     0x0D
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC5     0x0E
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC6     0x0F
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC7     0x10
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC8     0x11
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC9     0x12
#define ICM42688P_REG_BANK1_GYRO_CONFIG_STATIC10    0x13
#define ICM42688P_REG_BANK1_INTF_CONFIG4            0x7A
#define ICM42688P_REG_BANK1_INTF_CONFIG5            0x7B
#define ICM42688P_REG_BANK1_INTF_CONFIG6            0x7C

/* ============================================================================
 * Bank 2 寄存器地址定义
 * ============================================================================ */
#define ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC2    0x03
#define ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC3    0x04
#define ICM42688P_REG_BANK2_ACCEL_CONFIG_STATIC4    0x05

/* ============================================================================
 * Bank 4 寄存器地址定义
 * ============================================================================ */
#define ICM42688P_REG_BANK4_ACCEL_WOM_X_THR         0x4A
#define ICM42688P_REG_BANK4_ACCEL_WOM_Y_THR         0x4B
#define ICM42688P_REG_BANK4_ACCEL_WOM_Z_THR         0x4C
#define ICM42688P_REG_BANK4_OFFSET_USER0            0x77
#define ICM42688P_REG_BANK4_OFFSET_USER1            0x78
#define ICM42688P_REG_BANK4_OFFSET_USER2            0x79
#define ICM42688P_REG_BANK4_OFFSET_USER3            0x7A
#define ICM42688P_REG_BANK4_OFFSET_USER4            0x7B
#define ICM42688P_REG_BANK4_OFFSET_USER5            0x7C
#define ICM42688P_REG_BANK4_OFFSET_USER6            0x7D
#define ICM42688P_REG_BANK4_OFFSET_USER7            0x7E
#define ICM42688P_REG_BANK4_OFFSET_USER8            0x7F

/* ============================================================================
 * 配置结构体定义
 * ============================================================================ */

/**
 * @brief Bank 0 配置寄存器结构体
 */
typedef struct {
    uint8_t DEVICE_CONFIG;        // 0x11 - 设备配置
    uint8_t DRIVE_CONFIG;         // 0x13 - 驱动强度配置
    uint8_t INT_CONFIG;           // 0x14 - 中断引脚配置
    uint8_t FIFO_CONFIG;          // 0x16 - FIFO模式配置
    uint8_t INTF_CONFIG0;         // 0x4C - 接口配置0
    uint8_t INTF_CONFIG1;         // 0x4D - 接口配置1
    uint8_t PWR_MGMT0;            // 0x4E - 电源管理
    uint8_t GYRO_CONFIG0;         // 0x4F - 陀螺仪配置0
    uint8_t ACCEL_CONFIG0;        // 0x50 - 加速度计配置0
    uint8_t GYRO_CONFIG1;         // 0x51 - 陀螺仪配置1
    uint8_t GYRO_ACCEL_CONFIG0;   // 0x52 - 陀螺仪/加速度计滤波器带宽
    uint8_t ACCEL_CONFIG1;        // 0x53 - 加速度计配置1
    uint8_t TMST_CONFIG;          // 0x54 - 时间戳配置
    uint8_t SMD_CONFIG;           // 0x57 - SMD/WOM配置
    uint8_t FIFO_CONFIG1;         // 0x5F - FIFO配置1
    uint8_t FIFO_CONFIG2;         // 0x60 - FIFO水印低字节
    uint8_t FIFO_CONFIG3;         // 0x61 - FIFO水印高字节
    uint8_t FSYNC_CONFIG;         // 0x62 - FSYNC配置
    uint8_t INT_CONFIG0;          // 0x63 - 中断清除配置
    uint8_t INT_CONFIG1;          // 0x64 - 中断时序配置
    uint8_t INT_SOURCE0;          // 0x65 - INT1中断源
    uint8_t INT_SOURCE1;          // 0x66 - INT1中断源(WOM)
    uint8_t INT_SOURCE3;          // 0x68 - INT2中断源
    uint8_t INT_SOURCE4;          // 0x69 - INT2中断源(WOM)
    uint8_t SELF_TEST_CONFIG;     // 0x70 - 自检配置
} ICM42688P_Bank0_Config;

/**
 * @brief Bank 1 配置寄存器结构体
 */
typedef struct {
    uint8_t SENSOR_CONFIG0;           // 0x03 - 传感器轴使能
    uint8_t GYRO_CONFIG_STATIC2;      // 0x0B - 陀螺仪AAF/NF使能
    uint8_t GYRO_CONFIG_STATIC3;      // 0x0C - 陀螺仪AAF DELT
    uint8_t GYRO_CONFIG_STATIC4;      // 0x0D - 陀螺仪AAF DELTSQR[7:0]
    uint8_t GYRO_CONFIG_STATIC5;      // 0x0E - 陀螺仪AAF BITSHIFT/DELTSQR[11:8]
    uint8_t GYRO_CONFIG_STATIC6;      // 0x0F - 陀螺仪X轴NF COSWZ[7:0]
    uint8_t GYRO_CONFIG_STATIC7;      // 0x10 - 陀螺仪Y轴NF COSWZ[7:0]
    uint8_t GYRO_CONFIG_STATIC8;      // 0x11 - 陀螺仪Z轴NF COSWZ[7:0]
    uint8_t GYRO_CONFIG_STATIC9;      // 0x12 - 陀螺仪NF COSWZ[8]
    uint8_t GYRO_CONFIG_STATIC10;     // 0x13 - 陀螺仪NF带宽选择
    uint8_t INTF_CONFIG4;             // 0x7A - 接口配置4 (SPI/I3C)
    uint8_t INTF_CONFIG5;             // 0x7B - 接口配置5 (引脚9功能)
    uint8_t INTF_CONFIG6;             // 0x7C - 接口配置6 (I3C)
} ICM42688P_Bank1_Config;

/**
 * @brief Bank 2 配置寄存器结构体
 */
typedef struct {
    uint8_t ACCEL_CONFIG_STATIC2;     // 0x03 - 加速度计AAF DELT/使能
    uint8_t ACCEL_CONFIG_STATIC3;     // 0x04 - 加速度计AAF DELTSQR[7:0]
    uint8_t ACCEL_CONFIG_STATIC4;     // 0x05 - 加速度计AAF BITSHIFT/DELTSQR[11:8]
} ICM42688P_Bank2_Config;

/**
 * @brief Bank 4 配置寄存器结构体 (不包含APEX功能)
 */
typedef struct {
    uint8_t ACCEL_WOM_X_THR;          // 0x4A - X轴WOM阈值
    uint8_t ACCEL_WOM_Y_THR;          // 0x4B - Y轴WOM阈值
    uint8_t ACCEL_WOM_Z_THR;          // 0x4C - Z轴WOM阈值
    uint8_t OFFSET_USER0;             // 0x77 - 陀螺仪X偏移[7:0]
    uint8_t OFFSET_USER1;             // 0x78 - 陀螺仪Y偏移[11:8] | X偏移[11:8]
    uint8_t OFFSET_USER2;             // 0x79 - 陀螺仪Y偏移[7:0]
    uint8_t OFFSET_USER3;             // 0x7A - 陀螺仪Z偏移[7:0]
    uint8_t OFFSET_USER4;             // 0x7B - 加速度计X偏移[11:8] | 陀螺仪Z偏移[11:8]
    uint8_t OFFSET_USER5;             // 0x7C - 加速度计X偏移[7:0]
    uint8_t OFFSET_USER6;             // 0x7D - 加速度计Y偏移[7:0]
    uint8_t OFFSET_USER7;             // 0x7E - 加速度计Z偏移[11:8] | Y偏移[11:8]
    uint8_t OFFSET_USER8;             // 0x7F - 加速度计Z偏移[7:0]
} ICM42688P_Bank4_Config;

/**
 * @brief ICM42688P完整配置结构体
 */
typedef struct {
    uint32_t magic;                    // 魔数标识: 0x49434D42 ("ICMB")
    uint16_t version;                  // 配置版本号
    uint16_t checksum;                 // CRC16校验和
    ICM42688P_Bank0_Config bank0;      // Bank 0配置
    ICM42688P_Bank1_Config bank1;      // Bank 1配置
    ICM42688P_Bank2_Config bank2;      // Bank 2配置
    ICM42688P_Bank4_Config bank4;      // Bank 4配置
} ICM42688P_Config;

/* ============================================================================
 * 函数声明
 * ============================================================================ */

/* 初始化与默认配置 */
/**
 * @brief 初始化配置结构体并加载默认值
 * @param config 配置结构体指针
 */
void ICM42688P_InitConfig(ICM42688P_Config* config);

/**
 * @brief 加载默认配置值
 * @param config 配置结构体指针
 */
void ICM42688P_LoadDefaultConfig(ICM42688P_Config* config);

/* 模块化配置函数 */
/**
 * @brief 配置电源管理
 * @param config 配置结构体指针
 * @param gyro_mode 陀螺仪模式: 00=OFF, 01=Standby, 11=LN
 * @param accel_mode 加速度计模式: 00=OFF, 10=LP, 11=LN
 * @param temp_disable 温度传感器禁用: 0=使能, 1=禁用
 * @param idle_mode IDLE模式: 0=OFF时关闭RC, 1=保持RC开启
 */
void ICM42688P_ConfigPower(ICM42688P_Config* config, 
                           uint8_t gyro_mode, uint8_t accel_mode,
                           uint8_t temp_disable, uint8_t idle_mode);

/**
 * @brief 配置陀螺仪
 * @param config 配置结构体指针
 * @param fs_sel 满量程选择: 000=±2000dps, 001=±1000dps, ...
 * @param odr 输出数据率: 0110=1kHz, 0111=200Hz, ...
 * @param ui_filt_ord UI滤波器阶数: 00=1阶, 01=2阶, 10=3阶
 * @param ui_filt_bw UI滤波器带宽
 */
void ICM42688P_ConfigGyro(ICM42688P_Config* config,
                          uint8_t fs_sel, uint8_t odr,
                          uint8_t ui_filt_ord, uint8_t ui_filt_bw);

/**
 * @brief 配置加速度计
 * @param config 配置结构体指针
 * @param fs_sel 满量程选择: 000=±16g, 001=±8g, 010=±4g, 011=±2g
 * @param odr 输出数据率: 0110=1kHz, 0111=200Hz, ...
 * @param ui_filt_ord UI滤波器阶数: 00=1阶, 01=2阶, 10=3阶
 * @param ui_filt_bw UI滤波器带宽
 */
void ICM42688P_ConfigAccel(ICM42688P_Config* config,
                           uint8_t fs_sel, uint8_t odr,
                           uint8_t ui_filt_ord, uint8_t ui_filt_bw);

/**
 * @brief 配置FIFO
 * @param config 配置结构体指针
 * @param mode FIFO模式: 00=Bypass, 01=Stream, 10/11=Stop-on-full
 * @param gyro_en 陀螺仪数据写入FIFO: 0=禁用, 1=使能
 * @param accel_en 加速度计数据写入FIFO: 0=禁用, 1=使能
 * @param temp_en 温度数据写入FIFO: 0=禁用, 1=使能
 * @param watermark FIFO水印值 (0-2048)
 */
void ICM42688P_ConfigFIFO(ICM42688P_Config* config,
                          uint8_t mode, uint8_t gyro_en, uint8_t accel_en,
                          uint8_t temp_en, uint16_t watermark);

/**
 * @brief 配置中断引脚
 * @param config 配置结构体指针
 * @param int1_polarity INT1极性: 0=低电平有效, 1=高电平有效
 * @param int1_drive INT1驱动: 0=开漏, 1=推挽
 * @param int1_mode INT1模式: 0=脉冲, 1=锁存
 * @param int2_polarity INT2极性
 * @param int2_drive INT2驱动
 * @param int2_mode INT2模式
 */
void ICM42688P_ConfigInterrupt(ICM42688P_Config* config,
                               uint8_t int1_polarity, uint8_t int1_drive,
                               uint8_t int1_mode, uint8_t int2_polarity,
                               uint8_t int2_drive, uint8_t int2_mode);

/**
 * @brief 配置中断源
 * @param config 配置结构体指针
 * @param int1_sources INT1中断源位掩码
 * @param int2_sources INT2中断源位掩码
 */
void ICM42688P_ConfigInterruptSource(ICM42688P_Config* config,
                                     uint8_t int1_sources, uint8_t int2_sources);

/**
 * @brief 配置陀螺仪抗混叠滤波器(AAF)
 * @param config 配置结构体指针
 * @param enable 使能: 0=使能, 1=禁用
 * @param delt DELT参数
 * @param deltsqr DELTSQR参数 (12位)
 * @param bitshift BITSHIFT参数
 */
void ICM42688P_ConfigGyroAAF(ICM42688P_Config* config, uint8_t enable,
                             uint8_t delt, uint16_t deltsqr, uint8_t bitshift);

/**
 * @brief 配置加速度计抗混叠滤波器(AAF)
 * @param config 配置结构体指针
 * @param enable 使能: 0=使能, 1=禁用
 * @param delt DELT参数
 * @param deltsqr DELTSQR参数 (12位)
 * @param bitshift BITSHIFT参数
 */
void ICM42688P_ConfigAccelAAF(ICM42688P_Config* config, uint8_t enable,
                              uint8_t delt, uint16_t deltsqr, uint8_t bitshift);

/**
 * @brief 配置陀螺仪用户偏移量
 * @param config 配置结构体指针
 * @param offset_x X轴偏移 (±64dps, 1/32 dps分辨率)
 * @param offset_y Y轴偏移
 * @param offset_z Z轴偏移
 */
void ICM42688P_ConfigGyroOffset(ICM42688P_Config* config,
                                int16_t offset_x, int16_t offset_y, int16_t offset_z);

/**
 * @brief 配置加速度计用户偏移量
 * @param config 配置结构体指针
 * @param offset_x X轴偏移 (±1g, 0.5mg分辨率)
 * @param offset_y Y轴偏移
 * @param offset_z Z轴偏移
 */
void ICM42688P_ConfigAccelOffset(ICM42688P_Config* config,
                                 int16_t offset_x, int16_t offset_y, int16_t offset_z);

/**
 * @brief 配置运动唤醒(WOM)
 * @param config 配置结构体指针
 * @param mode WOM模式: 0=与初始样本比较, 1=与前一样本比较
 * @param int_mode 中断模式: 0=OR逻辑, 1=AND逻辑
 * @param threshold_x X轴阈值 (0-255, ~3.9mg分辨率)
 * @param threshold_y Y轴阈值
 * @param threshold_z Z轴阈值
 */
void ICM42688P_ConfigWOM(ICM42688P_Config* config, uint8_t mode, uint8_t int_mode,
                         uint8_t threshold_x, uint8_t threshold_y, uint8_t threshold_z);

/**
 * @brief 配置时钟
 * @param config 配置结构体指针
 * @param clk_sel 时钟源: 00=RC, 01=PLL(自动), 11=禁用
 * @param rtc_mode RTC模式: 0=不需要外部时钟, 1=需要外部RTC
 * @param pin9_func 引脚9功能: 00=INT2, 01=FSYNC, 10=CLKIN
 */
void ICM42688P_ConfigClock(ICM42688P_Config* config,
                           uint8_t clk_sel, uint8_t rtc_mode, uint8_t pin9_func);

/**
 * @brief 配置温度传感器滤波器
 * @param config 配置结构体指针
 * @param filt_bw 滤波器带宽: 000=4000Hz, 001=170Hz, ...
 */
void ICM42688P_ConfigTemperature(ICM42688P_Config* config, uint8_t filt_bw);

/**
 * @brief 配置FSYNC
 * @param config 配置结构体指针
 * @param ui_sel FSYNC标志位置: 000=不标记, 001=TEMP_OUT, 010-111=GYRO/ACCEL
 * @param polarity 极性: 0=上升沿, 1=下降沿
 * @param flag_clear_sel 标志清除: 0=寄存器更新时, 1=读取LSB时
 */
void ICM42688P_ConfigFSYNC(ICM42688P_Config* config,
                           uint8_t ui_sel, uint8_t polarity, uint8_t flag_clear_sel);

/**
 * @brief 配置时间戳
 * @param config 配置结构体指针
 * @param enable 时间戳使能: 0=禁用, 1=使能
 * @param resolution 分辨率: 0=1μs, 1=16μs或RTC周期
 * @param delta_en 增量模式: 0=绝对时间, 1=时间差
 * @param fsync_en FSYNC时间戳: 0=禁用, 1=使能
 */
void ICM42688P_ConfigTimestamp(ICM42688P_Config* config,
                               uint8_t enable, uint8_t resolution, 
                               uint8_t delta_en, uint8_t fsync_en);

/* 应用与读取配置 */
/**
 * @brief 将配置应用到芯片
 * @param config 配置结构体指针
 * @return 0=成功, 1=配置无效, 其他=写入错误
 */
uint8_t ICM42688P_ApplyConfig(const ICM42688P_Config* config);

/**
 * @brief 从芯片读取当前配置
 * @param config 配置结构体指针
 * @return 0=成功, 非0=读取错误
 */
uint8_t ICM42688P_ReadConfig(ICM42688P_Config* config);

/* 序列化函数 */
/**
 * @brief 导出配置到缓冲区(用于EEPROM写入)
 * @param config 配置结构体指针
 * @param buffer 目标缓冲区
 * @param buffer_size 缓冲区大小
 * @return 实际写入字节数, 0=缓冲区太小
 */
uint16_t ICM42688P_ExportConfig(const ICM42688P_Config* config, 
                                uint8_t* buffer, uint16_t buffer_size);

/**
 * @brief 从缓冲区导入配置(用于EEPROM读取)
 * @param config 配置结构体指针
 * @param buffer 源缓冲区
 * @param buffer_size 缓冲区大小
 * @return 0=成功, 1=数据不完整, 2=配置无效
 */
uint8_t ICM42688P_ImportConfig(ICM42688P_Config* config, 
                               const uint8_t* buffer, uint16_t buffer_size);

/* 工具函数 */
/**
 * @brief 计算配置的CRC16校验和
 * @param config 配置结构体指针
 * @return CRC16校验和
 */
uint16_t ICM42688P_CalculateChecksum(const ICM42688P_Config* config);

/**
 * @brief 验证配置有效性
 * @param config 配置结构体指针
 * @return 1=有效, 0=无效
 */
uint8_t ICM42688P_ValidateConfig(const ICM42688P_Config* config);

/**
 * @brief 获取配置结构体大小
 * @return 配置结构体大小(字节)
 */
uint16_t ICM42688P_GetConfigSize(void);

#ifdef __cplusplus
}
#endif

#endif /* ICM42688P_CONFIG_H */

