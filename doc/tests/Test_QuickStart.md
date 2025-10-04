# ICM42688P 测试快速上手

## 5分钟快速开始

### 1. 编译和烧录
```bash
make clean && make -j8
# 烧录到开发板
```

### 2. 连接串口监视器
- 波特率: 115200
- 等待测试自动运行

### 3. 查看结果
看到以下输出表示成功：
```
✓ 所有测试通过！
```

## 两种测试模式

### 完整测试（默认）
```c
ICM42688P_RunAllTests();  // ~60秒，52个测试用例
```

### 快速测试
```c
ICM42688P_RunQuickTest();  // ~10秒，核心功能
```

修改位置：`Core/Src/main.c` 第234行

## 测试内容概览

| 类别 | 测试组 | 测试项 | 验证内容 |
|------|--------|--------|----------|
| **Level 1** | 3 | ~8 | 全局配置、读写验证、校验 |
| **Level 2** | 4 | ~12 | 增量更新、一致性检查 |
| **Level 3** | 4 | ~15 | 快速修改、性能测试 |
| **性能** | 2 | ~5 | 三级对比、批量修改 |
| **压力** | 2 | ~8 | 频繁切换、连续更新 |
| **边界** | 2 | ~4 | NULL保护、极值处理 |
| **场景** | 2 | ~6 | 实际应用模拟 |

## 预期性能指标

| 操作 | Level 1 | Level 2 | Level 3 | 提升 |
|------|---------|---------|---------|------|
| 修改1个ODR | ~700ms | ~15ms | **~1ms** | **700x** |
| 修改多参数 | ~700ms | ~50ms | ~3ms | **233x** |

## 故障排查速查

### 测试卡住不动
1. 检查SPI接线（CS/SCK/MISO/MOSI）
2. 读取WHO_AM_I，应为0x47
3. 检查电源（3.3V稳定）

### 部分测试失败
1. 降低SPI时钟速度
2. 增加电源去耦电容
3. 检查地线连接

### 性能异常
1. 确认HAL_GetTick()正常
2. 关闭不必要的中断
3. 检查系统时钟配置

## 输出示例

### 成功的测试输出
```
========================================
  ICM42688P 三级配置系统测试
========================================

=== Test Level 1: Basic Init ===
[PASS] InitConfig - Magic
[PASS] InitConfig - Version
[PASS] ApplyConfig - Success
  ApplyConfig time: 705 ms

=== Test Level 3: Fast ODR Modify ===
[PASS] FastODR - Success
  Average ODR change time: 1 ms (10 samples)

=== Performance Comparison ===
Level 1 (Full Apply):        705 ms
Level 2 (1 Runtime Param):   15 ms
Level 3 (Fast Modify):       1 ms

Speedup:
  Level 3 vs Level 1: 705.0x faster

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

### 失败的测试输出
```
[FAIL] FastODR - Success: SetGyroODR failed

... (其他测试) ...

========================================
  测试结果统计
========================================
总测试数:   52
通过:       48
失败:       4
通过率:     92.3%
========================================

✗ 部分测试失败，请检查日志
```

## 下一步

- 查看详细说明：[Test_Guide.md](Test_Guide.md)
- 了解三级系统：[ThreeTierConfig_Usage.md](ThreeTierConfig_Usage.md)
- 自定义测试：参考 `Test_Guide.md` 中的"自定义测试"章节

## 技术支持

遇到问题？检查以下文档：
1. [测试完整指南](Test_Guide.md)
2. [三级配置系统快速参考](ThreeTierConfig_QuickRef.md)
3. [技术文档](ThreeTierConfig_Technical.md)
4. [寄存器手册](ICM42688P_寄存器手册.md)

---

**版本**: v1.0 | **日期**: 2025-10-04

