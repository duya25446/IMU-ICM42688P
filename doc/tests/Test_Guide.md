# ICM42688P 三级配置系统测试指南

## 概述

本文档说明如何使用完整的测试程序来验证ICM42688P三级配置系统的功能。

## 测试文件说明

### 核心文件
- `Core/Src/ICM42688P_Test.c` - 测试实现
- `Core/Inc/ICM42688P_Test.h` - 测试头文件
- `Core/Src/main.c` - 主程序（已集成测试调用）

## 快速开始

### 1. 编译并烧录

```bash
# 确保已配置好工具链
cd /home/duya25446/Documents/STM32Project/IMU-ICM42688P
make -j8
# 烧录到开发板
```

### 2. 连接串口

- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验**: 无
- **流控**: 无

推荐工具：
- Linux: `minicom`, `picocom`, `screen`
- Windows: PuTTY, Tera Term
- 跨平台: Serial Studio, CuteCom

### 3. 运行测试

上电后，测试会自动运行。你会看到类似以下的输出：

```
ICM42688P Init Success
启动测试...

========================================
  ICM42688P 三级配置系统测试
========================================

=== Test Level 1: Basic Init ===
[PASS] InitConfig - Magic
[PASS] InitConfig - Version
[PASS] DefaultConfig - PWR_MGMT0
[PASS] DefaultConfig - GYRO_CONFIG0
[PASS] ApplyConfig - Success
  ApplyConfig time: 705 ms
[PASS] ApplyConfig - Time

... (更多测试输出) ...

========================================
  测试结果统计
========================================
总测试数:   52
通过:       52
失败:       0
通过率:     100.0%
========================================

✓ 所有测试通过！
```

## 测试模式

### 完整测试模式（默认）

在 `main.c` 中使用：
```c
ICM42688P_RunAllTests();
```

**包含的测试**:
- Level 1 功能测试（3个测试组）
- Level 2 功能测试（4个测试组）
- Level 3 功能测试（4个测试组）
- 性能对比测试（2个测试组）
- 压力测试（2个测试组）
- 边界条件测试（2个测试组）
- 应用场景测试（2个测试组）

**预计耗时**: 约 30-60 秒

### 快速测试模式

如果只需要快速验证核心功能，可改为：
```c
ICM42688P_RunQuickTest();
```

**包含的测试**:
- Level 1 基本初始化
- Level 2 增量更新
- Level 3 快速ODR修改
- 性能对比

**预计耗时**: 约 5-10 秒

## 测试详解

### Level 1 测试（全局配置应用）

#### Test_Level1_BasicInit
**目的**: 验证配置初始化和全局应用功能

**测试内容**:
- `ICM42688P_InitConfig()` 正确性
- Magic number 和 Version 验证
- 默认配置值验证
- `ICM42688P_ApplyConfig()` 成功性
- 应用时间约700ms（符合预期）

**通过标准**: 
- 所有配置正确初始化
- ApplyConfig耗时 600-800ms

#### Test_Level1_ReadVerify
**目的**: 验证配置写入和读回的一致性

**测试内容**:
- 写入自定义配置
- 读取芯片配置
- 对比关键寄存器值

**通过标准**: 
- 写入值与读回值完全一致

#### Test_Level1_Validation
**目的**: 验证配置验证机制

**测试内容**:
- 有效配置被接受
- 无效Magic被拒绝
- 无效Version被拒绝

**通过标准**: 
- 验证逻辑正确

### Level 2 测试（增量更新）

#### Test_Level2_BasicIncremental
**目的**: 验证增量更新基本功能

**测试内容**:
- 首次调用（应等同Level 1）
- 无变化调用（应快速返回）

**通过标准**: 
- 首次调用成功
- 无变化调用 < 50ms

#### Test_Level2_RuntimeModify
**目的**: 验证运行时参数修改

**测试内容**:
- 修改单个ODR参数
- 验证修改时间
- 验证修改生效

**通过标准**: 
- 修改成功
- 耗时 < 30ms

#### Test_Level2_NonRuntimeModify
**目的**: 验证非运行时参数修改

**测试内容**:
- 修改AAF滤波器参数
- 验证需要关闭传感器

**通过标准**: 
- 修改成功
- 耗时 50-100ms

#### Test_Level2_ConsistencyCheck
**目的**: 验证配置一致性检查

**测试内容**:
- 正常情况检测一致
- 手动修改芯片寄存器后检测不一致

**通过标准**: 
- 一致性检查正确工作
- 返回值bit1正确标识不一致

### Level 3 测试（快速修改）

#### Test_Level3_FastODR
**目的**: 验证快速ODR修改性能

**测试内容**:
- 连续10次快速切换ODR
- 测量平均时间

**通过标准**: 
- 所有操作成功
- 平均时间 ≤ 2ms

#### Test_Level3_FastFSR
**目的**: 验证快速FSR修改

**测试内容**:
- 陀螺仪FSR切换
- 加速度计FSR切换

**通过标准**: 
- 所有操作成功

#### Test_Level3_PowerMode
**目的**: 验证电源模式切换

**测试内容**:
- 陀螺仪 OFF ↔ LN
- 加速度计 LP ↔ LN

**通过标准**: 
- 所有切换成功

#### Test_Level3_CombinedFast
**目的**: 验证组合快速修改

**测试内容**:
- 同时修改4个运行时参数
- 验证总时间

**通过标准**: 
- 所有修改成功
- 总时间 ≤ 10ms

### 性能测试

#### Test_Performance_Comparison
**目的**: 对比三级系统性能

**输出示例**:
```
Level 1 (Full Apply):        705 ms
Level 2 (1 Runtime Param):   15 ms
Level 3 (Fast Modify):       1 ms

Speedup:
  Level 2 vs Level 1: 47.0x faster
  Level 3 vs Level 1: 705.0x faster
  Level 3 vs Level 2: 15.0x faster
```

#### Test_Performance_BatchModify
**目的**: 测试批量修改性能

**测试内容**:
- Level 2 修改5个参数
- Level 3 修改3个运行时参数

### 压力测试

#### Test_Stress_FrequentSwitch
**目的**: 验证频繁切换的稳定性

**测试内容**:
- 连续100次快速切换ODR
- 记录成功率和平均时间

**通过标准**: 
- 成功率 100%
- 平均时间 ≤ 2ms

#### Test_Stress_ContinuousL2
**目的**: 验证连续Level 2更新

**测试内容**:
- 连续20次Level 2增量更新

**通过标准**: 
- 成功率 100%

### 边界测试

#### Test_Edge_NullPointer
**目的**: 验证NULL指针保护

**测试内容**:
- 向配置函数传入NULL

**通过标准**: 
- 不崩溃，安全返回

#### Test_Edge_ExtremeValues
**目的**: 验证极限值处理

**测试内容**:
- 最大/最小ODR值
- 最大FSR值

**通过标准**: 
- 合法极值被接受
- 非法值被拒绝

### 应用场景测试

#### Test_Scenario_PowerModeSwitch
**目的**: 模拟实际功耗切换场景

**测试内容**:
- 高速模式 ↔ 省电模式切换

**通过标准**: 
- 切换时间 ≤ 10ms

#### Test_Scenario_AdaptiveFSR
**目的**: 模拟自适应量程场景

**测试内容**:
- 根据运动强度动态调整FSR

**通过标准**: 
- FSR选择逻辑正确

## 故障排查

### 测试失败

如果测试失败，请检查：

1. **硬件连接**
   - SPI接线正确
   - 电源稳定（3.3V）
   - CS/SCK/MISO/MOSI连接无误

2. **芯片状态**
   - WHO_AM_I 读取是否为 0x47
   - 芯片是否正常初始化

3. **时序问题**
   - 确认使用了正确的延时函数
   - 确认系统时钟配置正确

4. **串口输出**
   - 检查串口波特率
   - 检查UART初始化

### 常见问题

#### Q: 测试卡在某处不动
**A**: 检查SPI通信是否正常。尝试在`ICM42688P_Init()`后加读取WHO_AM_I。

#### Q: 性能测试时间异常
**A**: 确认HAL_GetTick()工作正常，检查是否有其他中断影响。

#### Q: 部分测试失败但大部分通过
**A**: 可能是硬件问题或电源不稳定。尝试：
- 降低SPI速度
- 改善电源滤波
- 增加延时裕量

#### Q: 压力测试失败
**A**: 可能是芯片过热或通信干扰。尝试：
- 降低测试频率
- 增加散热
- 检查PCB布局

## 自定义测试

### 添加新测试用例

在 `ICM42688P_Test.c` 中添加：

```c
static void Test_MyCustomTest(void)
{
    Test_Print("\r\n=== My Custom Test ===\r\n");
    
    // 你的测试代码
    uint8_t result = ICM42688P_SetGyroODR(0x06);
    
    Test_Assert("MyTest - Description",
                result == 0,
                "Error message if failed");
}
```

然后在 `ICM42688P_RunAllTests()` 中调用：

```c
// 在适当位置添加
Test_MyCustomTest();
```

### 修改测试参数

例如修改压力测试的迭代次数：

```c
// 在 Test_Stress_FrequentSwitch() 中
const uint16_t iterations = 50;  // 改为50次
```

## 测试报告

测试完成后会自动生成统计报告：

```
========================================
  测试结果统计
========================================
总测试数:   52
通过:       52
失败:       0
通过率:     100.0%
========================================
```

建议保存串口日志用于分析和归档。

## 持续集成

### 自动化测试脚本（参考）

```bash
#!/bin/bash
# 编译
make clean && make -j8

# 烧录
st-flash write build/IMU_ICM42688P.bin 0x08000000

# 读取串口输出（需要tty设备）
timeout 120s cat /dev/ttyUSB0 | tee test_output.log

# 分析结果
if grep -q "所有测试通过" test_output.log; then
    echo "✓ 测试通过"
    exit 0
else
    echo "✗ 测试失败"
    exit 1
fi
```

## 参考资料

- [三级配置系统使用指南](ThreeTierConfig_Usage.md)
- [三级配置系统技术文档](ThreeTierConfig_Technical.md)
- [快速参考](ThreeTierConfig_QuickRef.md)
- [寄存器手册](ICM42688P_寄存器手册.md)

## 版本历史

- **v1.0** (2025-10-04): 初始版本
  - 完整测试套件
  - 52个测试用例
  - 性能和压力测试

---

**作者**: AI Assistant  
**日期**: 2025-10-04  
**许可**: 同项目主许可证

