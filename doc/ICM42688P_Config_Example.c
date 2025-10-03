/**
 ******************************************************************************
 * @file    ICM42688P_Config_Example.c
 * @brief   ICM-42688-P配置管理库使用示例
 * @note    本文件仅作为参考，不参与编译
 ******************************************************************************
 */

/*
 * 使用示例 1: 基本配置与应用
 * =========================================
 */
void Example1_BasicConfig(void)
{
    ICM42688P_Config config;
    
    // 初始化配置为默认值
    ICM42688P_InitConfig(&config);
    
    // 配置电源管理 - 陀螺仪和加速度计都使用低噪声模式
    ICM42688P_ConfigPower(&config, 
                          0x03,  // 陀螺仪LN模式
                          0x03,  // 加速度计LN模式
                          0,     // 温度传感器使能
                          0);    // IDLE模式关闭
    
    // 配置陀螺仪 - ±2000dps, 1kHz, 2阶滤波器
    ICM42688P_ConfigGyro(&config,
                         0x00,  // ±2000dps
                         0x06,  // 1kHz ODR
                         0x01,  // 2阶滤波器
                         0x01); // 默认带宽
    
    // 配置加速度计 - ±16g, 1kHz, 2阶滤波器
    ICM42688P_ConfigAccel(&config,
                          0x00,  // ±16g
                          0x06,  // 1kHz ODR
                          0x01,  // 2阶滤波器
                          0x01); // 默认带宽
    
    // 应用配置到芯片
    uint8_t result = ICM42688P_ApplyConfig(&config);
    if (result == 0) {
        // 配置成功
    } else {
        // 配置失败，处理错误
    }
}

/*
 * 使用示例 2: FIFO配置
 * =========================================
 */
void Example2_FIFOConfig(void)
{
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    
    // 配置基本传感器参数（省略...）
    
    // 配置FIFO - Stream模式，启用陀螺仪和加速度计
    ICM42688P_ConfigFIFO(&config,
                         0x01,   // Stream-to-FIFO模式
                         1,      // 陀螺仪数据使能
                         1,      // 加速度计数据使能
                         1,      // 温度数据使能
                         512);   // 水印512字节
    
    // 配置中断 - INT1推挽高电平有效锁存模式
    ICM42688P_ConfigInterrupt(&config,
                              1,    // INT1高电平有效
                              1,    // INT1推挽输出
                              1,    // INT1锁存模式
                              0, 0, 0); // INT2不使用
    
    // 配置中断源 - FIFO水印中断路由到INT1
    ICM42688P_ConfigInterruptSource(&config,
                                    0x04,  // FIFO_THS_INT1_EN
                                    0x00); // INT2不使用
    
    ICM42688P_ApplyConfig(&config);
}

/*
 * 使用示例 3: 用户偏移量配置（校准）
 * =========================================
 */
void Example3_OffsetCalibration(void)
{
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    
    // 配置基本传感器参数（省略...）
    
    // 假设通过校准得到以下偏移量
    // 陀螺仪偏移：X=1.5dps, Y=-0.8dps, Z=0.3dps
    // 转换为寄存器值：value = offset_dps * 32
    int16_t gyro_x_offset = (int16_t)(1.5f * 32);
    int16_t gyro_y_offset = (int16_t)(-0.8f * 32);
    int16_t gyro_z_offset = (int16_t)(0.3f * 32);
    
    ICM42688P_ConfigGyroOffset(&config, 
                               gyro_x_offset,
                               gyro_y_offset,
                               gyro_z_offset);
    
    // 加速度计偏移：X=10mg, Y=-5mg, Z=2mg
    // 转换为寄存器值：value = offset_mg * 2
    int16_t accel_x_offset = (int16_t)(10.0f * 2);
    int16_t accel_y_offset = (int16_t)(-5.0f * 2);
    int16_t accel_z_offset = (int16_t)(2.0f * 2);
    
    ICM42688P_ConfigAccelOffset(&config,
                                accel_x_offset,
                                accel_y_offset,
                                accel_z_offset);
    
    ICM42688P_ApplyConfig(&config);
}

/*
 * 使用示例 4: 运动唤醒(WOM)配置
 * =========================================
 */
void Example4_WOMConfig(void)
{
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    
    // 配置为低功耗模式
    ICM42688P_ConfigPower(&config,
                          0x00,  // 陀螺仪关闭
                          0x02,  // 加速度计低功耗模式
                          0, 0);
    
    // 配置加速度计低速率
    ICM42688P_ConfigAccel(&config,
                          0x03,  // ±2g (更灵敏)
                          0x09,  // 50Hz ODR
                          0x01, 0x01);
    
    // 配置WOM - 与前一样本比较，OR逻辑
    // 阈值约382mg (98 * 3.9mg)
    ICM42688P_ConfigWOM(&config,
                        1,     // 与前一样本比较
                        0,     // OR逻辑（任一轴触发）
                        98,    // X轴阈值
                        98,    // Y轴阈值
                        98);   // Z轴阈值
    
    // 配置中断源 - WOM中断路由到INT1
    ICM42688P_ConfigInterruptSource(&config,
                                    0x07,  // WOM_X/Y/Z_INT1_EN
                                    0x00);
    
    ICM42688P_ApplyConfig(&config);
}

/*
 * 使用示例 5: 保存和加载配置（EEPROM）
 * =========================================
 */
void Example5_SaveLoadConfig(void)
{
    ICM42688P_Config config;
    uint8_t eeprom_buffer[256];
    
    // === 保存配置到EEPROM ===
    
    // 1. 创建并配置
    ICM42688P_InitConfig(&config);
    ICM42688P_ConfigPower(&config, 0x03, 0x03, 0, 0);
    ICM42688P_ConfigGyro(&config, 0x00, 0x06, 0x01, 0x01);
    ICM42688P_ConfigAccel(&config, 0x00, 0x06, 0x01, 0x01);
    
    // 2. 更新校验和
    config.checksum = ICM42688P_CalculateChecksum(&config);
    
    // 3. 导出到缓冲区
    uint16_t size = ICM42688P_ExportConfig(&config, eeprom_buffer, sizeof(eeprom_buffer));
    
    if (size > 0) {
        // 4. 写入EEPROM（需要自己实现EEPROM_Write函数）
        // EEPROM_Write(0x0000, eeprom_buffer, size);
    }
    
    // === 从EEPROM加载配置 ===
    
    // 1. 从EEPROM读取（需要自己实现EEPROM_Read函数）
    // EEPROM_Read(0x0000, eeprom_buffer, sizeof(eeprom_buffer));
    
    // 2. 导入配置
    uint8_t result = ICM42688P_ImportConfig(&config, eeprom_buffer, sizeof(eeprom_buffer));
    
    if (result == 0) {
        // 3. 配置有效，应用到芯片
        ICM42688P_ApplyConfig(&config);
    } else if (result == 1) {
        // 数据不完整
    } else if (result == 2) {
        // 配置无效（魔数、版本或校验和错误）
    }
}

/*
 * 使用示例 6: 读取当前芯片配置
 * =========================================
 */
void Example6_ReadCurrentConfig(void)
{
    ICM42688P_Config config;
    
    // 从芯片读取当前配置
    uint8_t result = ICM42688P_ReadConfig(&config);
    
    if (result == 0) {
        // 读取成功，可以查看或保存配置
        
        // 获取配置大小
        uint16_t config_size = ICM42688P_GetConfigSize();
        
        // 验证配置有效性
        if (ICM42688P_ValidateConfig(&config)) {
            // 配置有效
            
            // 可以导出保存
            uint8_t buffer[256];
            uint16_t size = ICM42688P_ExportConfig(&config, buffer, sizeof(buffer));
            // 保存到EEPROM...
        }
    }
}

/*
 * 使用示例 7: 高级滤波器配置（AAF）
 * =========================================
 */
void Example7_AdvancedFilterConfig(void)
{
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    
    // 配置基本参数
    ICM42688P_ConfigPower(&config, 0x03, 0x03, 0, 0);
    ICM42688P_ConfigGyro(&config, 0x00, 0x06, 0x01, 0x01);
    ICM42688P_ConfigAccel(&config, 0x00, 0x06, 0x01, 0x01);
    
    // 配置陀螺仪AAF - 258Hz带宽
    // 根据数据手册表5.3: DELT=6, DELTSQR=36, BITSHIFT=10
    ICM42688P_ConfigGyroAAF(&config,
                            0,     // 使能AAF
                            6,     // DELT
                            36,    // DELTSQR
                            10);   // BITSHIFT
    
    // 配置加速度计AAF - 258Hz带宽
    ICM42688P_ConfigAccelAAF(&config,
                             0,     // 使能AAF
                             6,     // DELT
                             36,    // DELTSQR
                             10);   // BITSHIFT
    
    ICM42688P_ApplyConfig(&config);
}

/*
 * 使用示例 8: 外部时钟配置
 * =========================================
 */
void Example8_ExternalClockConfig(void)
{
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    
    // 配置使用外部32.768kHz时钟输入
    ICM42688P_ConfigClock(&config,
                          0x01,  // PLL模式（自动选择）
                          1,     // 需要外部RTC时钟
                          0x02); // 引脚9配置为CLKIN
    
    // 配置时间戳使用RTC时钟
    ICM42688P_ConfigTimestamp(&config,
                              1,    // 使能时间戳
                              1,    // RTC周期分辨率
                              0,    // 绝对时间模式
                              1);   // 使能FSYNC时间戳
    
    ICM42688P_ApplyConfig(&config);
}

/*
 * 配置参数快速参考
 * =========================================
 * 
 * 陀螺仪满量程 (fs_sel):
 *   0x00: ±2000dps (默认)
 *   0x01: ±1000dps
 *   0x02: ±500dps
 *   0x03: ±250dps
 *   0x04: ±125dps
 * 
 * 加速度计满量程 (fs_sel):
 *   0x00: ±16g (默认)
 *   0x01: ±8g
 *   0x02: ±4g
 *   0x03: ±2g
 * 
 * 输出数据率 (odr):
 *   0x01: 32kHz
 *   0x02: 16kHz
 *   0x03: 8kHz
 *   0x04: 4kHz
 *   0x05: 2kHz
 *   0x06: 1kHz (默认)
 *   0x07: 200Hz
 *   0x08: 100Hz
 *   0x09: 50Hz
 *   0x0A: 25Hz
 *   0x0F: 500Hz
 * 
 * 工作模式:
 *   陀螺仪模式: 00=OFF, 01=Standby, 11=LN
 *   加速度计模式: 00=OFF, 10=LP, 11=LN
 * 
 * FIFO模式:
 *   0x00: Bypass
 *   0x01: Stream-to-FIFO
 *   0x10/0x11: Stop-on-full
 */

