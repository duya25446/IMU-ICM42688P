# ICM42688P 三级配置系统使用指南

## 概述

三级配置系统为ICM42688P提供了灵活的配置管理方案，根据不同场景选择最优的配置方法：

- **Level 1 (全局应用)**: 完整配置写入，用于芯片初始化/复位后 (~700ms)
- **Level 2 (增量更新)**: 智能增量写入，自动管理传感器电源 (按需延时)
- **Level 3 (快速运行)**: 运行时参数修改，无延时最快速度 (~1ms)

## Level 1: 全局配置应用

### 使用场景
- 芯片上电初始化
- 软件复位后
- 恢复出厂设置
- 从EEPROM加载完整配置

### 示例代码
```c
ICM42688P_Config config;

// 初始化并加载默认配置
ICM42688P_InitConfig(&config);

// 自定义配置
ICM42688P_ConfigPower(&config, 0x03, 0x03, 0, 0);  // 陀螺LN, 加速LN
ICM42688P_ConfigGyro(&config, 0, 6, 2, 1);         // ±2000dps, 1kHz, 3阶滤波
ICM42688P_ConfigAccel(&config, 2, 6, 2, 1);        // ±4g, 1kHz, 3阶滤波

// 应用配置（耗时约700ms）
uint8_t error = ICM42688P_ApplyConfig(&config);
if (error) {
    // 处理错误
}
```

**特点**:
- ✓ 写入所有寄存器，确保完整性
- ✓ 每个寄存器写入后延时10ms，最稳定
- ✓ 自动更新内部配置存储
- ✗ 耗时最长（~700ms）

## Level 2: 智能增量更新

### 使用场景
- 运行时调整滤波器参数
- 修改FIFO配置
- 调整中断设置
- 修改APEX功能配置
- 需要修改多个非运行时参数

### 示例代码
```c
ICM42688P_Config new_config;

// 获取当前配置（从lib内部存储）
memcpy(&new_config, &g_internal_config, sizeof(ICM42688P_Config));

// 修改需要的参数
ICM42688P_ConfigGyroAAF(&new_config, 0, 13, 170, 8);  // 修改陀螺AAF滤波器
ICM42688P_ConfigFIFO(&new_config, 1, 1, 1, 1, 512);   // 修改FIFO配置

// 增量应用（仅写入变化的寄存器）
uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);

if (result & 0x01) {
    // 写入失败（可能芯片损坏）
    printf("配置写入失败！\n");
}

if (result & 0x02) {
    // 检测到配置不一致（上次配置可能未正确应用）
    printf("警告：检测到配置不一致\n");
}
```

**特点**:
- ✓ 只写入变化的寄存器，节省时间和器件寿命
- ✓ 自动检测是否需要关闭传感器
- ✓ 自动保存和恢复电源状态
- ✓ 内建配置一致性检查
- ✓ 耗时取决于变化数量（通常10-100ms）
- ⚠ 首次调用会自动执行Level 1全局应用

**返回值说明**:
- `0x00`: 成功，配置一致
- `0x01`: 写入失败
- `0x02`: 警告，芯片配置与内部存储不一致
- `0x03`: 写入失败 + 不一致警告

## Level 3: 运行时快速修改

### 使用场景
- 动态切换采样率（ODR）
- 动态切换量程（FSR）
- 动态开关传感器
- 高频率参数调整（如自适应采样率）

### 示例代码

#### 示例1: 动态切换陀螺仪采样率
```c
// 从1kHz切换到200Hz
uint8_t error = ICM42688P_SetGyroODR(0x07);  // 0x07 = 200Hz
if (error) {
    // 失败可能表明芯片损坏
    printf("警告：陀螺仪ODR设置失败，请检查硬件！\n");
}
```

#### 示例2: 动态调整量程
```c
// 根据当前运动强度自适应调整量程
if (max_gyro_value > 1000) {
    // 切换到±2000dps
    ICM42688P_SetGyroFSR(0x00);
} else {
    // 切换到±1000dps以提高精度
    ICM42688P_SetGyroFSR(0x01);
}
```

#### 示例3: 省电模式切换
```c
// 检测到静止，关闭陀螺仪省电
ICM42688P_SetGyroMode(0x00);  // 关闭陀螺仪

// 检测到运动，开启陀螺仪
ICM42688P_SetGyroMode(0x03);  // 低噪声模式
```

#### 示例4: 组合使用多个快速函数
```c
// 高速模式：1kHz采样
ICM42688P_SetGyroODR(0x06);   // 1kHz
ICM42688P_SetAccelODR(0x06);  // 1kHz
ICM42688P_SetGyroMode(0x03);  // LN模式
ICM42688P_SetAccelMode(0x03); // LN模式

// 低功耗模式：50Hz采样
ICM42688P_SetAccelODR(0x09);  // 50Hz
ICM42688P_SetAccelMode(0x02); // LP模式
ICM42688P_SetGyroMode(0x00);  // 关闭陀螺仪
```

**特点**:
- ✓ 最快速度（~1ms，无延时）
- ✓ 可在传感器运行时调用
- ✓ 自动更新内部配置
- ✓ 内建写后读验证
- ✗ 仅支持6个运行时参数

### 可用的Level 3函数

| 函数 | 参数 | 说明 |
|------|------|------|
| `ICM42688P_SetGyroODR()` | 0x06=1kHz, 0x07=200Hz, 0x08=100Hz, 0x09=50Hz等 | 陀螺仪采样率 |
| `ICM42688P_SetGyroFSR()` | 0x00=±2000dps, 0x01=±1000dps, 0x02=±500dps等 | 陀螺仪量程 |
| `ICM42688P_SetGyroMode()` | 0x00=OFF, 0x01=Standby, 0x03=LN | 陀螺仪模式 |
| `ICM42688P_SetAccelODR()` | 0x06=1kHz, 0x07=200Hz, 0x08=100Hz, 0x09=50Hz等 | 加速度计采样率 |
| `ICM42688P_SetAccelFSR()` | 0x00=±16g, 0x01=±8g, 0x02=±4g, 0x03=±2g | 加速度计量程 |
| `ICM42688P_SetAccelMode()` | 0x00=OFF, 0x02=LP, 0x03=LN | 加速度计模式 |

## 三级系统选择指南

### 何时使用Level 1
- ✓ 芯片刚上电
- ✓ 执行软复位后
- ✓ 需要确保所有配置都正确
- ✓ 时间不敏感的场景

### 何时使用Level 2
- ✓ 修改滤波器参数（AAF, NF, UI滤波器）
- ✓ 调整FIFO模式和水印
- ✓ 修改中断源路由
- ✓ 配置APEX功能
- ✓ 修改多个参数
- ✓ 不确定是否需要关闭传感器

### 何时使用Level 3
- ✓ 频繁切换采样率
- ✓ 动态调整量程
- ✓ 快速开关传感器
- ✓ 实时性要求高
- ✓ 单个参数快速修改

## 性能对比

| 操作 | Level 1 | Level 2 | Level 3 |
|------|---------|---------|---------|
| 修改1个运行时参数 | ~700ms | ~5ms | ~1ms |
| 修改3个运行时参数 | ~700ms | ~10ms | ~3ms |
| 仅修改GYRO_MODE | ~700ms | ~2ms (v1.1优化) | ~1ms |
| 修改TEMP_DIS | ~700ms | ~55ms | 不支持 |
| 修改5个非运行时参数 | ~700ms | ~65ms | 不支持 |
| 完整配置重写 | ~700ms | ~200ms | 不支持 |

**v1.1性能改进**:
- PWR_MGMT0运行时修改从5ms降至2ms（去除不必要延时）
- 避免PWR_MGMT0重复写入（原50ms额外延时已消除）

## 注意事项

### 内部配置存储
- 库内部维护一份配置副本（`g_internal_config`）
- 首次调用任何Apply函数时初始化
- **v1.1新增**: Level 3首次调用也会自动初始化（读取芯片配置）
- Level 2会检查芯片配置与内部存储是否一致
- Level 3自动同步更新内部配置

### 错误处理
- Level 3函数返回非0表示写入失败，**可能是芯片损坏**
- Level 2函数bit1=1表示配置不一致，但仍会继续执行
- 所有写入函数都内建写后读验证

### PWR_MGMT0特殊处理（v1.1重要说明）
- PWR_MGMT0在Level 2中**单独处理**，不在常规writes列表中
- 仅修改GYRO_MODE/ACCEL_MODE：写入1次，**无延时**
- 修改TEMP_DIS/IDLE：写入1次，延时50ms
- 避免了v1.0的重复写入问题

### 配置一致性
如果Level 2检测到不一致（bit1=1），可能原因：
1. 上次配置写入失败但未检查返回值
2. 其他代码直接修改了寄存器
3. 芯片意外复位

建议定期使用Level 2验证配置一致性。

## 示例：完整的应用流程

```c
void IMU_Application_Init(void)
{
    ICM42688P_Config config;
    
    // 1. 首次初始化使用Level 1
    ICM42688P_InitConfig(&config);
    ICM42688P_ConfigPower(&config, 0x03, 0x03, 0, 0);
    ICM42688P_ConfigGyro(&config, 0, 6, 2, 1);
    ICM42688P_ConfigAccel(&config, 2, 6, 2, 1);
    ICM42688P_ApplyConfig(&config);  // Level 1: ~700ms
}

void IMU_Runtime_AdjustODR(uint8_t high_speed)
{
    // 2. 运行时快速切换采样率 - Level 3
    if (high_speed) {
        ICM42688P_SetGyroODR(0x06);   // 1kHz
        ICM42688P_SetAccelODR(0x06);  // 1kHz
    } else {
        ICM42688P_SetGyroODR(0x08);   // 100Hz
        ICM42688P_SetAccelODR(0x08);  // 100Hz
    }
    // Level 3: ~2ms
}

void IMU_Reconfigure_Filters(void)
{
    ICM42688P_Config new_config;
    
    // 3. 需要修改滤波器等非运行时参数 - Level 2
    memcpy(&new_config, &g_internal_config, sizeof(ICM42688P_Config));
    ICM42688P_ConfigGyroAAF(&new_config, 0, 22, 488, 6);  // 1051Hz AAF
    
    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);
    if (result & 0x02) {
        printf("警告：配置不一致\n");
    }
    // Level 2: ~50ms（需要关闭传感器）
}
```

## 技术细节

### 寄存器延时策略
- **PWR_MGMT0**: 50ms（陀螺仪启动需要45ms）
- **DEVICE_CONFIG**: 10ms（软复位需要1ms × 10倍）
- **其他寄存器**: 无延时

### 运行时可修改寄存器
只有以下寄存器可在传感器运行时修改：
- `GYRO_CONFIG0` (0x4F): 所有位
- `ACCEL_CONFIG0` (0x50): 所有位
- `PWR_MGMT0` (0x4E): 仅bits[3:0] (GYRO_MODE, ACCEL_MODE)

### Level 2增量更新流程
1. 读取芯片当前配置
2. 对比芯片配置 vs 内部存储（检查一致性）
3. 对比内部存储 vs 新配置（生成差异列表，PWR_MGMT0单独标记）
4. 判断是否需要关闭传感器
5. 如需关闭：关闭传感器 → 延时2ms
6. 写入变化的寄存器，不包括PWR_MGMT0（按需延时）
7. **如PWR_MGMT0变化**：写入PWR_MGMT0，仅在关闭传感器时延时50ms
8. 更新内部存储

**v1.1关键改进**:
- PWR_MGMT0单独处理，避免重复写入
- 运行时修改PWR_MGMT0无延时（仅bits[3:0]变化时）
- 边界检查防止数组越界

## 常见问题

**Q: 可以混用三级函数吗？**  
A: 可以。但建议遵循以下原则：
- 初始化用Level 1
- 批量修改用Level 2
- 单个快速修改用Level 3

**Q: Level 3修改后，Level 2会知道吗？**  
A: 会。Level 3会自动更新内部配置，Level 2能正确识别。

**Q: 如果Level 2返回0x02（不一致警告），应该怎么办？**  
A: 通常可以忽略，因为新配置已成功覆写。如果频繁出现，建议检查：
- 是否有其他代码直接操作寄存器
- 芯片是否意外复位
- 电源是否稳定

**Q: 为什么不能用Level 3修改滤波器？**  
A: 数据手册明确规定，除了ODR/FSR/MODE外的寄存器需要关闭传感器才能修改，Level 3不支持关闭传感器的操作。

**Q: Level 2比Level 1快多少？**  
A: 如果只修改少数寄存器，Level 2可以快10-20倍。例如只修改3个寄存器时，Level 2约30ms vs Level 1的700ms。

**Q: Level 3首次调用会比较慢吗？（v1.1新增）**  
A: 是的。如果在Level 1/2之前直接调用Level 3，会自动读取芯片配置并初始化（约10ms）。后续调用正常速度（~1ms）。建议在初始化阶段先调用Level 1。

**Q: PWR_MGMT0运行时修改为什么不延时？（v1.1新增）**  
A: 当只修改PWR_MGMT0的bits[3:0]（GYRO_MODE和ACCEL_MODE）时，数据手册允许运行时修改，无需延时。只有修改bits[7:4]（TEMP_DIS或IDLE）时才需要关闭传感器并延时50ms。

---

**版本**: v1.2  
**日期**: 2025-10-03  
**更新**: 安全加固版，新增NULL检查、缓冲区保护和完整寄存器支持

---

## ⚠️ v1.2 安全更新说明

### API变更（向后兼容）
1. **ICM42688P_ConfigTimestamp** 新增参数 `tmst_to_regs_en`
   - 旧代码需添加第5个参数（建议值：1）
2. **ICM42688P_ConfigGyroAAF/AccelAAF** 参数名变更
   - `enable` → `disable`（0=使能AAF，1=禁用AAF）
   - 旧代码逻辑不变，仅参数名更清晰

### 新增安全特性
- ✅ 所有配置函数自动NULL检查，传入NULL不会崩溃
- ✅ FormatRegisters自动防止缓冲区溢出
- ✅ FIFO watermark自动保护（0会被修正为1）
- ✅ 结构体序列化跨平台可靠（packed属性）

