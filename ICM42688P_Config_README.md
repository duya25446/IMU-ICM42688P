# ICM-42688-P 配置管理库

## 概述

本库提供了完整的 ICM-42688-P IMU 传感器配置管理功能，支持：

- **模块化配置**：提供针对各功能模块的独立配置函数
- **寄存器映射存储**：按照芯片寄存器地址直接映射存储配置
- **EEPROM序列化**：支持配置导出/导入，可保存到EEPROM
- **校验和验证**：使用CRC16校验确保配置完整性
- **Bank管理**：自动处理寄存器Bank切换

## 文件结构

```
Core/
├── Inc/
│   └── ICM42688P_Config.h          # 头文件（结构体、宏、函数声明）
└── Src/
    └── ICM42688P_Config.c          # 源文件（函数实现）
ICM42688P_Config_Example.c          # 使用示例（不参与编译）
```

## 配置覆盖范围

本库管理以下配置（不包含APEX运动功能）：

### Bank 0 (25个寄存器)
- 电源管理（陀螺仪/加速度计/温度传感器）
- 陀螺仪配置（量程、ODR、滤波器）
- 加速度计配置（量程、ODR、滤波器）
- FIFO配置（模式、水印、内容）
- 中断配置（引脚、源、时序）
- 接口配置（SPI/I2C/I3C）
- 时钟配置
- 时间戳配置
- FSYNC配置
- WOM配置
- 自检配置

### Bank 1 (13个寄存器)
- 传感器轴使能/禁用
- 陀螺仪AAF配置
- 陀螺仪陷波滤波器配置
- 接口配置（SPI模式、引脚功能、I3C）

### Bank 2 (3个寄存器)
- 加速度计AAF配置

### Bank 4 (12个寄存器，不含APEX）
- WOM阈值
- 陀螺仪用户偏移量（X/Y/Z）
- 加速度计用户偏移量（X/Y/Z）

**总计：53个配置寄存器**

## 快速开始

### 1. 基本配置

```c
#include "ICM42688P_Config.h"

ICM42688P_Config config;

// 初始化为默认值
ICM42688P_InitConfig(&config);

// 配置电源管理
ICM42688P_ConfigPower(&config, 0x03, 0x03, 0, 0);  // 陀螺仪和加速度计LN模式

// 配置陀螺仪：±2000dps, 1kHz
ICM42688P_ConfigGyro(&config, 0x00, 0x06, 0x01, 0x01);

// 配置加速度计：±16g, 1kHz
ICM42688P_ConfigAccel(&config, 0x00, 0x06, 0x01, 0x01);

// 应用到芯片
ICM42688P_ApplyConfig(&config);
```

### 2. FIFO配置

```c
// 配置FIFO：Stream模式，陀螺仪+加速度计，水印512字节
ICM42688P_ConfigFIFO(&config, 0x01, 1, 1, 1, 512);

// 配置中断：INT1推挽高电平锁存
ICM42688P_ConfigInterrupt(&config, 1, 1, 1, 0, 0, 0);

// 配置中断源：FIFO水印中断到INT1
ICM42688P_ConfigInterruptSource(&config, 0x04, 0x00);

ICM42688P_ApplyConfig(&config);
```

### 3. 保存/加载配置

```c
uint8_t eeprom_buffer[256];

// === 保存配置 ===
config.checksum = ICM42688P_CalculateChecksum(&config);
uint16_t size = ICM42688P_ExportConfig(&config, eeprom_buffer, sizeof(eeprom_buffer));
// EEPROM_Write(0x0000, eeprom_buffer, size);

// === 加载配置 ===
// EEPROM_Read(0x0000, eeprom_buffer, sizeof(eeprom_buffer));
if (ICM42688P_ImportConfig(&config, eeprom_buffer, sizeof(eeprom_buffer)) == 0) {
    ICM42688P_ApplyConfig(&config);
}
```

## API参考

### 初始化函数

| 函数 | 说明 |
|------|------|
| `ICM42688P_InitConfig()` | 初始化配置结构体并加载默认值 |
| `ICM42688P_LoadDefaultConfig()` | 加载默认配置值 |

### 模块配置函数

| 函数 | 说明 |
|------|------|
| `ICM42688P_ConfigPower()` | 配置电源管理 |
| `ICM42688P_ConfigGyro()` | 配置陀螺仪 |
| `ICM42688P_ConfigAccel()` | 配置加速度计 |
| `ICM42688P_ConfigFIFO()` | 配置FIFO |
| `ICM42688P_ConfigInterrupt()` | 配置中断引脚 |
| `ICM42688P_ConfigInterruptSource()` | 配置中断源 |
| `ICM42688P_ConfigGyroAAF()` | 配置陀螺仪抗混叠滤波器 |
| `ICM42688P_ConfigAccelAAF()` | 配置加速度计抗混叠滤波器 |
| `ICM42688P_ConfigGyroOffset()` | 配置陀螺仪偏移量 |
| `ICM42688P_ConfigAccelOffset()` | 配置加速度计偏移量 |
| `ICM42688P_ConfigWOM()` | 配置运动唤醒 |
| `ICM42688P_ConfigClock()` | 配置时钟 |
| `ICM42688P_ConfigTemperature()` | 配置温度传感器滤波器 |
| `ICM42688P_ConfigFSYNC()` | 配置FSYNC |
| `ICM42688P_ConfigTimestamp()` | 配置时间戳 |

### 应用与读取函数

| 函数 | 说明 | 返回值 |
|------|------|--------|
| `ICM42688P_ApplyConfig()` | 将配置应用到芯片 | 0=成功 |
| `ICM42688P_ReadConfig()` | 从芯片读取当前配置 | 0=成功 |

### 序列化函数

| 函数 | 说明 | 返回值 |
|------|------|--------|
| `ICM42688P_ExportConfig()` | 导出配置到缓冲区 | 字节数 |
| `ICM42688P_ImportConfig()` | 从缓冲区导入配置 | 0=成功, 1=数据不完整, 2=无效 |

### 工具函数

| 函数 | 说明 | 返回值 |
|------|------|--------|
| `ICM42688P_CalculateChecksum()` | 计算CRC16校验和 | 校验和 |
| `ICM42688P_ValidateConfig()` | 验证配置有效性 | 1=有效 |
| `ICM42688P_GetConfigSize()` | 获取配置结构体大小 | 字节数 |

## 配置参数快速参考

### 陀螺仪满量程 (fs_sel)
| 值 | 量程 | 灵敏度 |
|----|------|--------|
| 0x00 | ±2000 dps | 16.4 LSB/(°/s) |
| 0x01 | ±1000 dps | 32.8 LSB/(°/s) |
| 0x02 | ±500 dps | 65.5 LSB/(°/s) |
| 0x03 | ±250 dps | 131 LSB/(°/s) |

### 加速度计满量程 (fs_sel)
| 值 | 量程 | 灵敏度 |
|----|------|--------|
| 0x00 | ±16 g | 2048 LSB/g |
| 0x01 | ±8 g | 4096 LSB/g |
| 0x02 | ±4 g | 8192 LSB/g |
| 0x03 | ±2 g | 16384 LSB/g |

### 输出数据率 (odr)
| 值 | ODR | 说明 |
|----|-----|------|
| 0x01 | 32 kHz | 高速采样 |
| 0x06 | 1 kHz | 标准(默认) |
| 0x07 | 200 Hz | 低速 |
| 0x09 | 50 Hz | 低功耗 |
| 0x0F | 500 Hz | 常用 |

### 工作模式
| 传感器 | 模式值 | 说明 |
|--------|--------|------|
| 陀螺仪 | 0x00 | OFF |
| 陀螺仪 | 0x01 | Standby |
| 陀螺仪 | 0x03 | 低噪声(LN) |
| 加速度计 | 0x00 | OFF |
| 加速度计 | 0x02 | 低功耗(LP) |
| 加速度计 | 0x03 | 低噪声(LN) |

## 注意事项

1. **配置顺序**：建议先配置基本参数（电源、量程、ODR），再配置高级功能（FIFO、中断、滤波器）
2. **寄存器Bank**：`ICM42688P_ApplyConfig()` 会自动处理Bank切换，无需手动管理
3. **校验和**：保存配置前务必调用 `ICM42688P_CalculateChecksum()` 更新校验和
4. **EEPROM大小**：配置结构体约80字节，建议EEPROM缓冲区至少256字节
5. **工厂校准值**：部分寄存器（如陀螺仪陷波滤波器）包含工厂校准值，读取芯片配置时会保留这些值

## 依赖项

- `ICM-42688P.h` - 基础驱动库（提供 `ICM42688P_BankSEL()`, `ICM42688P_WriteRegister()`, `ICM42688P_ReadRegister()` 函数）
- `<string.h>` - memset, memcpy
- `<stddef.h>` - offsetof

## 更多示例

详细使用示例请参考 `ICM42688P_Config_Example.c` 文件，包含：

1. 基本配置与应用
2. FIFO配置
3. 用户偏移量配置（校准）
4. 运动唤醒(WOM)配置
5. 保存和加载配置（EEPROM）
6. 读取当前芯片配置
7. 高级滤波器配置（AAF）
8. 外部时钟配置

## 许可证

本库基于现有项目，遵循项目原有许可证。

## 版本

- **v1.0** - 2024年10月 - 初始版本
- 基于 ICM-42688-P Datasheet Rev 1.8

