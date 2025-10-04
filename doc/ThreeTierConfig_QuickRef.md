# ICM42688P 三级配置系统快速参考

## 📋 三级系统速查

| 级别 | 函数 | 耗时 | 使用场景 | 限制 |
|------|------|------|---------|------|
| **Level 1** | `ICM42688P_ApplyConfig()` | ~700ms | 初始化/复位 | 耗时最长 |
| **Level 2** | `ICM42688P_ApplyConfigIncremental()` | 10-100ms | 批量修改 | 首次调用=Level 1 |
| **Level 3** | `ICM42688P_SetXXX()` | ~1ms | 单参数快改 | 仅6个参数 |

## 🚀 Level 3 快速函数（最常用）

### 陀螺仪
```c
ICM42688P_SetGyroODR(0x06);   // 切换采样率：0x06=1kHz, 0x07=200Hz, 0x08=100Hz
ICM42688P_SetGyroFSR(0x00);   // 切换量程：0x00=±2000dps, 0x01=±1000dps
ICM42688P_SetGyroMode(0x03);  // 切换模式：0x00=OFF, 0x01=Standby, 0x03=LN
```

### 加速度计
```c
ICM42688P_SetAccelODR(0x06);  // 切换采样率：0x06=1kHz, 0x07=200Hz, 0x08=100Hz
ICM42688P_SetAccelFSR(0x02);  // 切换量程：0x00=±16g, 0x01=±8g, 0x02=±4g, 0x03=±2g
ICM42688P_SetAccelMode(0x03); // 切换模式：0x00=OFF, 0x02=LP, 0x03=LN
```

## 📊 常用参数速查表

### ODR值对照表
| 代码 | 频率 | 适用场景 |
|------|------|---------|
| 0x01 | 32kHz | 极高速 |
| 0x02 | 16kHz | 极高速 |
| 0x03 | 8kHz | 高速 |
| 0x04 | 4kHz | 高速 |
| 0x05 | 2kHz | 高速 |
| 0x06 | 1kHz | **标准（默认）** |
| 0x07 | 200Hz | 低速 |
| 0x08 | 100Hz | 低速 |
| 0x09 | 50Hz | 低功耗 |
| 0x0A | 25Hz | 低功耗 |
| 0x0B | 12.5Hz | 超低功耗 |
| 0x0F | 500Hz | 常用 |

### FSR值对照表

#### 陀螺仪
| 代码 | 量程 | 灵敏度 | 适用场景 |
|------|------|--------|---------|
| 0x00 | ±2000dps | 16.4 LSB/(°/s) | **标准（默认）** |
| 0x01 | ±1000dps | 32.8 | 中速运动 |
| 0x02 | ±500dps | 65.5 | 慢速运动 |
| 0x03 | ±250dps | 131 | 精密测量 |

#### 加速度计
| 代码 | 量程 | 灵敏度 | 适用场景 |
|------|------|--------|---------|
| 0x00 | ±16g | 2048 LSB/g | **标准（默认）** |
| 0x01 | ±8g | 4096 | 中等冲击 |
| 0x02 | ±4g | 8192 | 姿态检测 |
| 0x03 | ±2g | 16384 | 精密倾角 |

### 电源模式对照表
| 模式代码 | 陀螺仪 | 加速度计 | 功耗 |
|---------|--------|----------|------|
| 0x00 | OFF | OFF | 7.5μA |
| 0x01 | Standby | - | 中 |
| 0x02 | - | LP | 低 |
| 0x03 | LN | LN | 0.88mA |

## 💡 典型使用场景代码片段

### 场景1: 高速模式 ↔ 省电模式
```c
// 进入高速模式
ICM42688P_SetGyroODR(0x06);    // 1kHz
ICM42688P_SetAccelODR(0x06);   // 1kHz
ICM42688P_SetGyroMode(0x03);   // LN
ICM42688P_SetAccelMode(0x03);  // LN

// 进入省电模式
ICM42688P_SetGyroODR(0x09);    // 50Hz
ICM42688P_SetAccelODR(0x09);   // 50Hz
ICM42688P_SetGyroMode(0x00);   // OFF
ICM42688P_SetAccelMode(0x02);  // LP
```

### 场景2: 自适应量程
```c
if (peak_gyro > 1500) {
    ICM42688P_SetGyroFSR(0x00);  // ±2000dps
} else if (peak_gyro > 800) {
    ICM42688P_SetGyroFSR(0x01);  // ±1000dps
} else {
    ICM42688P_SetGyroFSR(0x02);  // ±500dps (更高精度)
}
```

### 场景3: 运动检测唤醒
```c
if (motion_detected) {
    ICM42688P_SetGyroMode(0x03);   // 唤醒陀螺
    ICM42688P_SetAccelMode(0x03);  // 唤醒加速
} else {
    ICM42688P_SetGyroMode(0x00);   // 关闭陀螺省电
    ICM42688P_SetAccelMode(0x02);  // 加速LP模式
}
```

## ⚠️ 常见错误

### 错误1: 用Level 3修改滤波器
```c
// ❌ 错误：滤波器不能用Level 3修改
// ICM42688P_SetGyroFilter(...);  // 不存在此函数

// ✅ 正确：使用Level 2
ICM42688P_Config cfg;
// ... 修改cfg的滤波器参数 ...
ICM42688P_ApplyConfigIncremental(&cfg);
```

### 错误2: 忽略Level 2警告
```c
// ⚠️ 应该检查返回值
uint8_t result = ICM42688P_ApplyConfigIncremental(&cfg);
if (result & 0x02) {
    printf("警告：配置不一致，建议检查\n");
}
```

### 错误3: Level 3未检查错误
```c
// ❌ 不检查返回值
ICM42688P_SetGyroODR(0x06);

// ✅ 应该检查
if (ICM42688P_SetGyroODR(0x06) != 0) {
    // 可能芯片损坏，需要处理
}
```

## 🔧 调试技巧

### 检查配置一致性
```c
// 定期检查配置是否与芯片一致
ICM42688P_Config test_config;
// ... 设置test_config为当前应该的配置 ...
uint8_t result = ICM42688P_ApplyConfigIncremental(&test_config);
if (result & 0x02) {
    printf("检测到配置漂移！\n");
}
```

### 性能分析
```c
uint32_t start = HAL_GetTick();
ICM42688P_SetGyroODR(0x06);
uint32_t elapsed = HAL_GetTick() - start;
printf("Level 3耗时: %lu ms\n", elapsed);  // 应该约1ms
```

### 验证写入成功
```c
// Level 3函数内建写后读验证，返回非0即失败
uint8_t error = ICM42688P_SetGyroODR(0x06);
if (error) {
    // 读取实际寄存器验证
    uint8_t actual_value;
    ICM42688P_ReadRegister(0x4F, &actual_value, 1);
    printf("期望: 0x06, 实际: 0x%02X\n", actual_value);
}
```

## 📈 性能对比速查

| 操作 | Level 1 | Level 2 | Level 3 | 最快方式 |
|------|---------|---------|---------|---------|
| 改1个ODR | 700ms | 15ms | **1ms** | ⭐ Level 3 |
| 改ODR+FSR | 700ms | 20ms | **2ms** | ⭐ Level 3 |
| 改3个运行时参数 | 700ms | 25ms | **3ms** | ⭐ Level 3 |
| 改滤波器 | 700ms | **50ms** | N/A | ⭐ Level 2 |
| 改FIFO+中断 | 700ms | **80ms** | N/A | ⭐ Level 2 |
| 完整重置 | **700ms** | 200ms | N/A | ⭐ Level 1 |

## 🎯 快速决策树

```
需要修改配置？
├─ 芯片刚上电/复位？
│  └─ YES → Level 1 (ApplyConfig)
│
├─ 只修改1-3个运行时参数？
│  └─ YES → Level 3 (SetXXX)
│
├─ 修改多个参数或非运行时参数？
│  └─ YES → Level 2 (ApplyConfigIncremental)
│
└─ 不确定？
   └─ 安全选择 → Level 2
```

## 💾 内存占用

| 项目 | 大小 | 位置 |
|------|------|------|
| g_internal_config | ~90B | 静态RAM |
| g_diff | ~206B | 静态RAM (68×3+2) |
| 寄存器映射表 | ~200B | Flash (const) |
| **总计** | **~496B** | 296B RAM + 200B Flash |

## 📝 代码模板

### 模板1: 基本初始化
```c
void IMU_Init(void) {
    ICM42688P_Config cfg;
    ICM42688P_InitConfig(&cfg);
    ICM42688P_ConfigPower(&cfg, 0x03, 0x03, 0, 0);
    ICM42688P_ConfigGyro(&cfg, 0, 0x06, 0x02, 0x01);
    ICM42688P_ConfigAccel(&cfg, 0x02, 0x06, 0x02, 0x01);
    ICM42688P_ApplyConfig(&cfg);
}
```

### 模板2: 运行时调整
```c
void IMU_AdjustSpeed(uint8_t high_speed) {
    if (high_speed) {
        ICM42688P_SetGyroODR(0x06);   // 1kHz
        ICM42688P_SetAccelODR(0x06);
    } else {
        ICM42688P_SetGyroODR(0x09);   // 50Hz
        ICM42688P_SetAccelODR(0x09);
    }
}
```

### 模板3: 批量修改
```c
void IMU_Reconfigure(void) {
    ICM42688P_Config new_cfg;
    memcpy(&new_cfg, &my_cfg, sizeof(ICM42688P_Config));
    
    // 修改多个参数...
    ICM42688P_ConfigGyroAAF(&new_cfg, ...);
    ICM42688P_ConfigFIFO(&new_cfg, ...);
    
    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_cfg);
    if (result & 0x01) printf("失败\n");
    if (result & 0x02) printf("警告：不一致\n");
}
```

## 🔍 故障排查

| 症状 | 可能原因 | 解决方案 |
|------|---------|---------|
| Level 3返回非0 | 芯片损坏/通信故障 | 检查硬件和SPI |
| Level 2返回0x02 | 配置漂移 | 正常，已自动修复 |
| Level 2返回0x03 | 写入失败+不一致 | 检查硬件 |
| 配置后无效 | 未等待足够时间 | Level 1后延时100ms |

## 📞 快速联系

**相关文档**:
- 使用指南: `ThreeTierConfig_Usage.md`
- 技术文档: `ThreeTierConfig_Technical.md`
- 示例代码: `ThreeTierConfig_Example.c`
- Bug修复报告: `BugFix_Report.md`
- 寄存器手册: `ICM42688P_寄存器手册.md`
- 配置参数: `ICM42688P_配置参数手册.md`

**版本**: v1.2 | **日期**: 2025-10-03 | **更新**: 安全加固版（NULL检查+缓冲区保护+完整寄存器支持）

**⚠️ API变更提示**: 
- `ICM42688P_ConfigTimestamp` 新增参数（建议默认值=1）
- `ICM42688P_ConfigGyroAAF/AccelAAF` 参数名 enable→disable（语义更清晰）

