# 如何运行测试程序

## 📋 快速启用测试（3步）

### 步骤 1: 启用测试文件编译

编辑 `CMakeLists.txt` (第49行)：

```cmake
# 取消注释这一行
Core/Src/ICM42688P_Test.c  # 测试程序
```

### 步骤 2: 包含测试头文件

编辑 `Core/Src/main.c` (第26行)：

```c
// 取消注释这一行
#include "ICM42688P_Test.h"  // 测试程序
```

### 步骤 3: 启用测试代码

编辑 `Core/Src/main.c` (第83-85行和230-242行)：

**变量声明**：
```c
// 取消这两行的注释
ICM42688P_Config ReadAllConfig;
ICM42688P_Config TestConfig;
```

**测试变量**（第176-177行）：
```c
// 取消这两行的注释
uint8_t regbuffer[4096];
uint16_t size = 0;
```

**测试调用**（第230-242行）：
```c
// 取消所有测试相关代码的注释
HAL_UART_Transmit(&huart2, (const unsigned char *)"\r\n启动测试验证...\r\n", 
                  strlen("\r\n启动测试验证...\r\n"), 1000);
HAL_Delay(100);

// 运行完整测试（如需快速验证可改为 ICM42688P_RunQuickTest）
ICM42688P_RunAllTests();

// 可选：打印最终配置状态
ICM42688P_ReadAllConfigRegisters(&ReadAllConfig);
size = ICM42688P_FormatRegisters(&ReadAllConfig, (char *)regbuffer, 4096);
HAL_UART_Transmit(&huart2, (const unsigned char *)"\r\n=== 最终配置状态 ===\r\n",
                  strlen("\r\n=== 最终配置状态 ===\r\n"), 1000);
HAL_UART_Transmit(&huart2, (const unsigned char *)regbuffer, size, 1000);
```

### 步骤 4: 编译并运行

```bash
make clean && make -j8
# 烧录到开发板
# 通过串口（115200波特率）查看测试结果
```

## ⚡ 快速测试模式

如果只需要快速验证核心功能（~10秒），可以使用：

```c
ICM42688P_RunQuickTest();  // 代替 ICM42688P_RunAllTests()
```

## 🔄 禁用测试（恢复当前状态）

只需重新注释掉上述3个位置的代码即可。

## 📝 提示

- ✅ 测试文件保留在 `Core/Src/ICM42688P_Test.c` 方便随时启用
- ✅ 存档在 `doc/tests/` 作为备份
- ✅ 测试通过后记得禁用以减少Flash占用（约6-7KB）

---

**快速参考**: 
- [测试指南](Test_Guide.md)
- [测试报告](Test_Report_v1.0.md)
- [快速上手](Test_QuickStart.md)

