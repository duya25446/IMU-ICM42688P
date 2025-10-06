/**
 * @file ICM-426688P.c
 * @brief ICM-42688P IMU传感器驱动实现
 *
 * 该文件实现了ICM-42688P IMU传感器的驱动功能，包括SPI通信、
 * 寄存器操作、数据解析和传感器配置等功能。
 */

#include "ICM-42688P.h"
#include <stdint.h>

// 外部SPI句柄声明
extern SPI_HandleTypeDef hspi1;

/**
 * @brief 通过SPI发送数据（使用HAL库）
 * @param bytes 要发送的数据缓冲区
 * @param length 数据长度
 */
void spi_send_bytes(uint8_t *bytes, uint8_t length)
{
    HAL_SPI_Transmit(&hspi1, bytes, length, HAL_MAX_DELAY);
}

/**
 * @brief 延时函数
 * @param ms 延时时间（毫秒）
 */
void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief 通过SPI接收数据（使用HAL库）
 * @param bytes 接收数据缓冲区
 * @param length 数据长度
 */
void spi_read_bytes(uint8_t *bytes, uint8_t length)
{
    HAL_SPI_Receive(&hspi1, bytes, length, HAL_MAX_DELAY);
}

/**
 * @brief 突发读取寄存器（底层硬件实现 - HAL库版本）
 * @param reg_address 寄存器地址
 * @param rxdata 接收数据缓冲区
 * @param length 数据长度
 *
 * @note 使用HAL库进行SPI通信，提供稳定可靠的寄存器读取功能
 *
 * @warning 此函数为内部实现，不处理Bank选择等上层逻辑
 */
void ICM42688P_BurstRead_Internal(uint8_t reg_address, uint8_t *rxdata, uint8_t length)
{
    uint8_t tx_addr = reg_address | ICM42688P_READ;
    
    cs_low();
    
    // 发送寄存器地址（带读标志）
    HAL_SPI_Transmit(&hspi1, &tx_addr, 1, HAL_MAX_DELAY);
    
    // 读取数据
    if (length > 0)
    {
        HAL_SPI_Receive(&hspi1, rxdata, length, HAL_MAX_DELAY);
    }
    
    cs_high();
}

/**
 * @brief 初始化ICM42688P传感器
 *
 * 执行传感器初始化流程：
 * 1. 片选置高
 * 2. 软件复位
 * 3. 时钟配置
 * 4. 输出数据速率配置
 * 5. 启动传感器
 */
uint8_t ICM42688P_Init(void)
{
    uint8_t whoami;
    cs_high();
    ICM42688P_Software_Reset();
    delay_ms(10);
    ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(ICM42688P_WHOAMI, &whoami, 1);
    if (whoami != 0x47)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    // 这里未来添加判断EEPROM是否存在过去配置，如果存在就用非出厂模式启动，如果不存在就使用出厂测试模式流程
    // ICM42688P_LoadDefaultConfig(&config);
    // ICM42688P_ReadGyroFactoryCalibration(&config);
    //   ICM42688P_Clock_Config();
    //   // ICM42688P_Interrupt_Config();
    //   ICM42688P_ODR_Config();
    //   ICM42688P_Start();
}

/**
 * @brief 选择寄存器组
 * @param bank 寄存器组编号
 */
void ICM42688P_Bank_Select(uint8_t bank)
{
    uint8_t config = bank;
    ICM42688P_WriteRegister(0x76, &config, 1);
}

/**
 * @brief 软件复位
 *
 * 向设备发送软件复位命令，使设备恢复到默认状态
 */
void ICM42688P_Software_Reset(void)
{
    uint8_t address = 0x11;
    uint8_t txdata = 0x01;
    ICM42688P_Bank_Select(0);
    cs_low();
    spi_send_bytes(&address, 1);
    spi_send_bytes(&txdata, 1);
    cs_high();
}

/**
 * @brief 启动传感器
 *
 * 配置传感器进入正常工作模式
 */
void ICM42688P_Start(void)
{
    ICM42688P_Bank_Select(0);
    uint8_t address = 0x4e;
    uint8_t config = 0b00001111;
    ICM42688P_WriteRegister(address, &config, 1);
}

/**
 * @brief 停止传感器
 *
 * 配置传感器进入低功耗模式
 */
void ICM42688P_Stop(void)
{
    ICM42688P_Bank_Select(0);
    uint8_t address = 0x4e;
    uint8_t config = 0;
    ICM42688P_WriteRegister(address, &config, 1);
}

/**
 * @brief 配置输出数据速率(ODR)
 *
 * 配置加速度计和陀螺仪的输出数据速率
 */
void ICM42688P_ODR_Config(void)
{
    ICM42688P_Bank_Select(0);
    uint8_t config = 1;
    ICM42688P_WriteRegister(0x4f, &config, 1);
    ICM42688P_WriteRegister(0x50, &config, 1);
}

/**
 * @brief 配置时钟
 *
 * 配置传感器的时钟源和时钟设置
 */
void ICM42688P_Clock_Config(void)
{
    ICM42688P_Bank_Select(1);
    uint8_t config = 0x04;
    ICM42688P_WriteRegister(0x7b, &config, 1);
    ICM42688P_Bank_Select(0);
    config = 0x95;
    ICM42688P_WriteRegister(0x4d, &config, 1);
}

/**
 * @brief 配置中断
 *
 * 配置传感器的中断设置
 */
void ICM42688P_Interrupt_Config(void)
{
    ICM42688P_Bank_Select(0);
    uint8_t config = 0x2;
    ICM42688P_WriteRegister(0x14, &config, 1);
    config = 0x8;
    ICM42688P_WriteRegister(0x65, &config, 1);
}

/**
 * @brief 解析12字节数据为6个int16数据
 * @param data 输入的12字节数据
 * @param output 输出的6个int16数据数组
 */
void parse_12bytes_to_6int16(uint8_t *data, int16_t *output)
{
    for (int i = 0; i < 6; i++)
    {
        // 每个int16占两个字节
        // 先获取高位字节然后左移8位确定高位
        output[i] = (int16_t)(data[i * 2] << 8);
        // 然后把低位字节放在低位
        output[i] |= data[i * 2 + 1];
    }
}

/**
 * @brief 将IMU原始数据转换为物理量
 * @param input 输入的6个int16原始数据
 * @param output 输出的6个double物理量数据
 */
void parse_imu_data_to_physical(int16_t *input, double *output)
{
    for (int i = 0; i < 6; i++)
    {
        if (i < 3)
        {                                                     // 前三个是加速度计数据
            output[i] = (double)input[i] * ACCEL_SENSITIVITY; // 加速度计
        }
        else
        {                                                    // 后三个是陀螺仪数据
            output[i] = (double)input[i] * GYRO_SENSITIVITY; // 陀螺仪
        }
    }
}

/**
 * @brief 读取IMU数据（包含温度）
 * @param data 指向IMU_Data结构体的指针，用于存储读取的数据
 *
 * 从0x1D开始一次性读取14个字节：
 * - 0x1D: 温度高字节
 * - 0x1E: 温度低字节
 * - 0x1F-0x2A: 加速度计和陀螺仪数据（12字节）
 * 进行解析和转换，并应用零偏校准和阈值滤波
 */
void ICM42688P_ReadIMUData(IMU_Data *data)
{
    uint8_t raw_data[14]; // 2字节温度 + 12字节IMU数据
    int16_t int16_data[6];
    double physical_data[6];

    // 从0x1D开始一次性读取14个字节
    // ICM42688P_Bank_Select(0);
    ICM42688P_ReadRegister(0x1D, raw_data, 14);

    // 解析温度数据（前2个字节）
    // 温度寄存器：0x1D(高字节) 0x1E(低字节)
    uint16_t temp_raw = (raw_data[0] << 8) | raw_data[1];
    data->temperature = ((float)temp_raw / 132.48f) + 25.0f;

    // 解析IMU数据（后12个字节）
    parse_12bytes_to_6int16(raw_data + 2, int16_data);
    parse_imu_data_to_physical(int16_data, physical_data);

    data->accel_x = physical_data[0];
    data->accel_y = physical_data[1];
    data->accel_z = physical_data[2];
    data->gyro_x = physical_data[3];
    data->gyro_y = physical_data[4];
    data->gyro_z = physical_data[5];

    // 陀螺仪阈值滤波
    // float epsilon = FLT_EPSILON;
    // const float threshold = 0.2;

    // if (fabs(data->gyro_x) < threshold + epsilon)
    // {
    //     data->gyro_x = 0.0;
    // }

    // if (fabs(data->gyro_y) < threshold + epsilon)
    // {
    //     data->gyro_y = 0.0;
    // }

    // if (fabs(data->gyro_z) < threshold + epsilon)
    // {
    //     data->gyro_z = 0.0;
    // }
}

/**
 * @brief 读取寄存器（对外API接口）
 * @param reg_address 寄存器地址
 * @param rxdata 接收数据缓冲区
 * @param length 数据长度
 *
 * @note 这是对外的API接口函数，适用于以下场景：
 *       - 单字节寄存器读取（如读取WHO_AM_I）
 *       - 连续多字节寄存器读取（如读取12字节IMU数据）
 *       - 不需要频繁切换Bank的批量读取
 *
 * @example 读取单个寄存器：
 *          uint8_t whoami;
 *          ICM42688P_ReadRegister(ICM42688P_WHOAMI, &whoami, 1);
 *
 * @example 批量读取IMU数据（加速度+陀螺仪）：
 *          uint8_t imu_data[12];
 *          ICM42688P_ReadRegister(0x1F, imu_data, 12);
 */
void ICM42688P_ReadRegister(uint8_t reg_address, uint8_t *rxdata, uint8_t length)
{
    // 调用底层高性能突发读取实现
    ICM42688P_BurstRead_Internal(reg_address, rxdata, length);
}

/**
 * @brief 写入寄存器
 * @param reg_address 寄存器地址
 * @param txdata 发送数据缓冲区
 * @param length 数据长度
 * @return 0表示成功，1表示失败
 *
 * 写入寄存器后读取验证，确保写入成功
 */
uint8_t ICM42688P_WriteRegister(uint8_t reg_address, uint8_t *txdata, uint8_t length)
{
    uint8_t rx_buffer[256];
    uint8_t count = 0;
    uint8_t tx_data = reg_address;

    cs_low();
    spi_send_bytes(&tx_data, 1);
    spi_send_bytes(txdata, length);
    cs_high();

    // 读取验证
    ICM42688P_ReadRegister(reg_address, rx_buffer, length);
    for (count = 0; count < length; count++)
    {
        if (rx_buffer[count] != txdata[count])
        {
            return 1; // 写入失败
        }
    }
    return 0; // 写入成功
}
