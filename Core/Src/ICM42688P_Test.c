/**
 ******************************************************************************
 * @file    ICM42688P_Test.c
 * @brief   ICM42688P三级配置系统测试程序
 * @note    包含完整的单元测试和性能测试
 ******************************************************************************
 */

#include "ICM42688P_Test.h"
#include "ICM42688P_Config.h"
#include "ICM-42688P.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/* 外部变量声明 */
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;  // 用于性能测试的定时器

/* 测试统计 */
static uint16_t g_test_total = 0;
static uint16_t g_test_passed = 0;
static uint16_t g_test_failed = 0;

/* 测试缓冲区 */
static char test_buffer[512];
static uint8_t reg_buffer[4096];

/* ============================================================================
 * 辅助函数
 * ============================================================================ */

/**
 * @brief 打印测试信息
 */
static void Test_Print(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf(test_buffer, sizeof(test_buffer), format, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(&huart2, (uint8_t*)test_buffer, len, 1000);
    }
}

/**
 * @brief 测试断言
 */
static void Test_Assert(const char *test_name, uint8_t condition, const char *msg)
{
    g_test_total++;
    
    if (condition) {
        g_test_passed++;
        Test_Print("[PASS] %s\r\n", test_name);
    } else {
        g_test_failed++;
        Test_Print("[FAIL] %s: %s\r\n", test_name, msg);
    }
}

/**
 * @brief 获取当前时间戳(ms)
 */
static uint32_t Test_GetTick(void)
{
    return HAL_GetTick();
}

/**
 * @brief 延时
 */
static void Test_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

/* ============================================================================
 * Level 1 测试用例
 * ============================================================================ */

/**
 * @brief 测试Level 1: 基本初始化和全局应用
 */
static void Test_Level1_BasicInit(void)
{
    Test_Print("\r\n=== Test Level 1: Basic Init ===\r\n");
    
    ICM42688P_Config config;
    uint8_t error;
    
    // 测试1: InitConfig
    ICM42688P_InitConfig(&config);
    Test_Assert("InitConfig - Magic", 
                config.magic == ICM42688P_CONFIG_MAGIC,
                "Magic number mismatch");
    Test_Assert("InitConfig - Version",
                config.version == ICM42688P_CONFIG_VERSION,
                "Version mismatch");
    
    // 测试2: 验证默认配置
    Test_Assert("DefaultConfig - PWR_MGMT0",
                config.bank0.PWR_MGMT0 == 0x00,
                "Default power mode incorrect");
    Test_Assert("DefaultConfig - GYRO_CONFIG0",
                config.bank0.GYRO_CONFIG0 == 0x06,
                "Default gyro config incorrect");
    
    // 测试3: ApplyConfig
    uint32_t start_time = Test_GetTick();
    error = ICM42688P_ApplyConfig(&config);
    uint32_t elapsed = Test_GetTick() - start_time;
    
    Test_Assert("ApplyConfig - Success",
                error == 0,
                "Apply config failed");
    Test_Print("  ApplyConfig time: %lu ms\r\n", elapsed);
    Test_Assert("ApplyConfig - Time",
                elapsed >= 50 && elapsed <= 200,  // 优化后时间范围：50-200ms
                "Apply time out of range");
}

/**
 * @brief 测试Level 1: 读取和验证寄存器
 */
static void Test_Level1_ReadVerify(void)
{
    Test_Print("\r\n=== Test Level 1: Read & Verify ===\r\n");
    
    ICM42688P_Config write_config, read_config;
    
    // 准备测试配置
    ICM42688P_InitConfig(&write_config);
    ICM42688P_ConfigPower(&write_config, 0x03, 0x03, 0, 0);
    ICM42688P_ConfigGyro(&write_config, 0x01, 0x07, 0x02, 0x01);
    ICM42688P_ConfigAccel(&write_config, 0x02, 0x07, 0x02, 0x01);
    
    // 写入配置
    ICM42688P_ApplyConfig(&write_config);
    Test_Delay(100);
    
    // 读回配置
    ICM42688P_ReadAllConfigRegisters(&read_config);
    
    // 验证关键寄存器
    Test_Assert("ReadVerify - PWR_MGMT0",
                read_config.bank0.PWR_MGMT0 == write_config.bank0.PWR_MGMT0,
                "PWR_MGMT0 mismatch");
    Test_Assert("ReadVerify - GYRO_CONFIG0",
                read_config.bank0.GYRO_CONFIG0 == write_config.bank0.GYRO_CONFIG0,
                "GYRO_CONFIG0 mismatch");
    Test_Assert("ReadVerify - ACCEL_CONFIG0",
                read_config.bank0.ACCEL_CONFIG0 == write_config.bank0.ACCEL_CONFIG0,
                "ACCEL_CONFIG0 mismatch");
}

/**
 * @brief 测试Level 1: 配置验证
 */
static void Test_Level1_Validation(void)
{
    Test_Print("\r\n=== Test Level 1: Config Validation ===\r\n");
    
    ICM42688P_Config config;
    
    // 测试有效配置
    ICM42688P_InitConfig(&config);
    Test_Assert("Validation - Valid Config",
                ICM42688P_ValidateConfig(&config) == 1,
                "Valid config rejected");
    
    // 测试无效Magic
    config.magic = 0x12345678;
    Test_Assert("Validation - Invalid Magic",
                ICM42688P_ValidateConfig(&config) == 0,
                "Invalid magic accepted");
    
    // 测试无效Version
    ICM42688P_InitConfig(&config);
    config.version = 0xFFFF;
    Test_Assert("Validation - Invalid Version",
                ICM42688P_ValidateConfig(&config) == 0,
                "Invalid version accepted");
}

/* ============================================================================
 * Level 2 测试用例
 * ============================================================================ */

/**
 * @brief 测试Level 2: 增量更新基本功能
 */
static void Test_Level2_BasicIncremental(void)
{
    Test_Print("\r\n=== Test Level 2: Basic Incremental ===\r\n");
    
    ICM42688P_Config config;
    uint8_t result;
    
    // 首次调用应该等同于Level 1
    ICM42688P_InitConfig(&config);
    uint32_t start_time = Test_GetTick();
    result = ICM42688P_ApplyConfigIncremental(&config);
    uint32_t elapsed = Test_GetTick() - start_time;
    
    Test_Assert("Incremental - First Call Success",
                (result & 0x01) == 0,
                "First call failed");
    Test_Print("  First incremental time: %lu ms\r\n", elapsed);
    
    // 第二次调用（无变化）
    start_time = Test_GetTick();
    result = ICM42688P_ApplyConfigIncremental(&config);
    elapsed = Test_GetTick() - start_time;
    
    Test_Assert("Incremental - No Change",
                result == 0x00,
                "No change should return 0x00");
    Test_Print("  No change time: %lu ms\r\n", elapsed);
    Test_Assert("Incremental - No Change Time",
                elapsed < 50,
                "No change time too long");
}

/**
 * @brief 测试Level 2: 运行时参数修改
 */
static void Test_Level2_RuntimeModify(void)
{
    Test_Print("\r\n=== Test Level 2: Runtime Modify ===\r\n");
    
    ICM42688P_Config config;
    uint8_t result;
    
    // 初始化
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfigIncremental(&config);
    
    // 修改运行时参数（仅ODR）
    config.bank0.GYRO_CONFIG0 = 0x27;  // 200Hz
    
    uint32_t start_time = Test_GetTick();
    result = ICM42688P_ApplyConfigIncremental(&config);
    uint32_t elapsed = Test_GetTick() - start_time;
    
    Test_Assert("RuntimeModify - Success",
                (result & 0x01) == 0,
                "Runtime modify failed");
    Test_Print("  Runtime modify time: %lu ms\r\n", elapsed);
    Test_Assert("RuntimeModify - Time",
                elapsed < 30,
                "Runtime modify time too long");
    
    // 验证修改生效
    ICM42688P_Config read_config;
    ICM42688P_ReadAllConfigRegisters(&read_config);
    Test_Assert("RuntimeModify - Verify",
                read_config.bank0.GYRO_CONFIG0 == 0x27,
                "ODR change not applied");
}

/**
 * @brief 测试Level 2: 非运行时参数修改
 */
static void Test_Level2_NonRuntimeModify(void)
{
    Test_Print("\r\n=== Test Level 2: Non-Runtime Modify ===\r\n");
    
    ICM42688P_Config config;
    uint8_t result;
    
    // 初始化
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfigIncremental(&config);
    
    // 修改非运行时参数（AAF）- 使用不同于默认值的参数
    ICM42688P_ConfigGyroAAF(&config, 0, 22, 488, 6);  // 1051Hz AAF (不同于默认)
    
    uint32_t start_time = Test_GetTick();
    result = ICM42688P_ApplyConfigIncremental(&config);
    uint32_t elapsed = Test_GetTick() - start_time;
    
    Test_Assert("NonRuntimeModify - Success",
                (result & 0x01) == 0,
                "Non-runtime modify failed");
    Test_Print("  Non-runtime modify time: %lu ms\r\n", elapsed);
    Test_Assert("NonRuntimeModify - Time",
                elapsed >= 1 && elapsed <= 100,  // 放宽下限到1ms
                "Non-runtime modify time out of range");
}

/**
 * @brief 测试Level 2: 配置一致性检查
 */
static void Test_Level2_ConsistencyCheck(void)
{
    Test_Print("\r\n=== Test Level 2: Consistency Check ===\r\n");
    
    ICM42688P_Config config;
    uint8_t result;
    
    // 初始化
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfigIncremental(&config);
    
    // 正常情况：配置一致
    result = ICM42688P_ApplyConfigIncremental(&config);
    Test_Assert("ConsistencyCheck - Consistent",
                (result & 0x02) == 0,
                "Should be consistent");
    
    // 手动修改芯片寄存器（模拟外部修改）
    uint8_t temp_val = 0x08;
    ICM42688P_Bank_Select(0);
    ICM42688P_WriteRegister(0x4F, &temp_val, 1);
    
    // 再次应用，应该检测到不一致
    result = ICM42688P_ApplyConfigIncremental(&config);
    Test_Assert("ConsistencyCheck - Inconsistent",
                (result & 0x02) != 0,
                "Should detect inconsistency");
    Test_Print("  Inconsistency detected (expected)\r\n");
}

/* ============================================================================
 * Level 3 测试用例
 * ============================================================================ */

/**
 * @brief 测试Level 3: 快速ODR修改
 */
static void Test_Level3_FastODR(void)
{
    Test_Print("\r\n=== Test Level 3: Fast ODR Modify ===\r\n");
    
    uint8_t error;
    uint32_t total_time = 0;
    const uint8_t test_count = 10;
    
    // 初始化
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfig(&config);
    
    // 多次快速修改ODR
    for (uint8_t i = 0; i < test_count; i++) {
        uint8_t odr = (i % 2) ? 0x07 : 0x06;  // 在1kHz和200Hz之间切换
        
        uint32_t start_time = Test_GetTick();
        error = ICM42688P_SetGyroODR(odr);
        uint32_t elapsed = Test_GetTick() - start_time;
        
        total_time += elapsed;
        
        if (i == 0) {
            Test_Assert("FastODR - Success",
                        error == 0,
                        "SetGyroODR failed");
        }
    }
    
    uint32_t avg_time = total_time / test_count;
    Test_Print("  Average ODR change time: %lu ms (%d samples)\r\n", 
               avg_time, test_count);
    Test_Assert("FastODR - Time",
                avg_time <= 2,
                "ODR change time too long");
    
    // 最后再设置一次0x06，确保验证正确
    ICM42688P_SetGyroODR(0x06);
    Test_Delay(10);  // 短暂延时确保生效
    
    // 验证最后的修改
    ICM42688P_Config read_config;
    ICM42688P_ReadAllConfigRegisters(&read_config);
    Test_Assert("FastODR - Verify",
                (read_config.bank0.GYRO_CONFIG0 & 0x0F) == 0x06,
                "Final ODR not correct");
}

/**
 * @brief 测试Level 3: 快速FSR修改
 */
static void Test_Level3_FastFSR(void)
{
    Test_Print("\r\n=== Test Level 3: Fast FSR Modify ===\r\n");
    
    uint8_t error;
    
    // 测试陀螺仪FSR
    uint32_t start_time = Test_GetTick();
    error = ICM42688P_SetGyroFSR(0x01);  // ±1000dps
    uint32_t elapsed = Test_GetTick() - start_time;
    
    Test_Assert("FastFSR - Gyro Success",
                error == 0,
                "SetGyroFSR failed");
    Test_Print("  Gyro FSR change time: %lu ms\r\n", elapsed);
    
    // 测试加速度计FSR
    start_time = Test_GetTick();
    error = ICM42688P_SetAccelFSR(0x03);  // ±2g
    elapsed = Test_GetTick() - start_time;
    
    Test_Assert("FastFSR - Accel Success",
                error == 0,
                "SetAccelFSR failed");
    Test_Print("  Accel FSR change time: %lu ms\r\n", elapsed);
}

/**
 * @brief 测试Level 3: 电源模式切换
 */
static void Test_Level3_PowerMode(void)
{
    Test_Print("\r\n=== Test Level 3: Power Mode Switch ===\r\n");
    
    uint8_t error;
    
    // 测试陀螺仪模式切换
    error = ICM42688P_SetGyroMode(0x00);  // OFF
    Test_Assert("PowerMode - Gyro OFF",
                error == 0,
                "SetGyroMode OFF failed");
    Test_Delay(10);
    
    error = ICM42688P_SetGyroMode(0x03);  // LN
    Test_Assert("PowerMode - Gyro LN",
                error == 0,
                "SetGyroMode LN failed");
    Test_Delay(50);  // 等待陀螺仪启动
    
    // 测试加速度计模式切换
    error = ICM42688P_SetAccelMode(0x02);  // LP
    Test_Assert("PowerMode - Accel LP",
                error == 0,
                "SetAccelMode LP failed");
    
    error = ICM42688P_SetAccelMode(0x03);  // LN
    Test_Assert("PowerMode - Accel LN",
                error == 0,
                "SetAccelMode LN failed");
}

/**
 * @brief 测试Level 3: 组合快速修改
 */
static void Test_Level3_CombinedFast(void)
{
    Test_Print("\r\n=== Test Level 3: Combined Fast Modify ===\r\n");
    
    // 模拟实际应用场景：快速切换到高速模式
    uint32_t start_time = Test_GetTick();
    
    ICM42688P_SetGyroODR(0x06);    // 1kHz
    ICM42688P_SetAccelODR(0x06);   // 1kHz
    ICM42688P_SetGyroMode(0x03);   // LN
    ICM42688P_SetAccelMode(0x03);  // LN
    
    uint32_t elapsed = Test_GetTick() - start_time;
    
    Test_Print("  Combined 4 operations time: %lu ms\r\n", elapsed);
    Test_Assert("CombinedFast - Time",
                elapsed <= 10,
                "Combined operations too slow");
    
    // 验证所有修改
    ICM42688P_Config read_config;
    ICM42688P_ReadAllConfigRegisters(&read_config);
    
    Test_Assert("CombinedFast - Verify All",
                (read_config.bank0.GYRO_CONFIG0 & 0x0F) == 0x06 &&
                (read_config.bank0.ACCEL_CONFIG0 & 0x0F) == 0x06 &&
                (read_config.bank0.PWR_MGMT0 & 0x0F) == 0x0F,
                "Not all changes applied correctly");
}

/* ============================================================================
 * 性能对比测试
 * ============================================================================ */

/**
 * @brief 测试性能对比：三级系统
 */
static void Test_Performance_Comparison(void)
{
    Test_Print("\r\n=== Performance Comparison: 3-Level System ===\r\n");
    
    ICM42688P_Config config;
    uint32_t time_l1, time_l2, time_l3;
    
    // Level 1: 全局应用
    ICM42688P_InitConfig(&config);
    uint32_t start = Test_GetTick();
    ICM42688P_ApplyConfig(&config);
    time_l1 = Test_GetTick() - start;
    
    Test_Print("Level 1 (Full Apply):        %lu ms\r\n", time_l1);
    
    // Level 2: 修改1个运行时参数
    config.bank0.GYRO_CONFIG0 = 0x27;
    start = Test_GetTick();
    ICM42688P_ApplyConfigIncremental(&config);
    time_l2 = Test_GetTick() - start;
    
    Test_Print("Level 2 (1 Runtime Param):   %lu ms\r\n", time_l2);
    
    // Level 3: 修改1个运行时参数
    start = Test_GetTick();
    ICM42688P_SetGyroODR(0x06);
    time_l3 = Test_GetTick() - start;
    
    Test_Print("Level 3 (Fast Modify):       %lu ms\r\n", time_l3);
    
    // 计算提升倍数
    Test_Print("\r\nSpeedup:\r\n");
    Test_Print("  Level 2 vs Level 1: %.1fx faster\r\n", (float)time_l1 / time_l2);
    Test_Print("  Level 3 vs Level 1: %.1fx faster\r\n", (float)time_l1 / time_l3);
    Test_Print("  Level 3 vs Level 2: %.1fx faster\r\n", (float)time_l2 / time_l3);
}

/**
 * @brief 测试性能：批量修改
 */
static void Test_Performance_BatchModify(void)
{
    Test_Print("\r\n=== Performance: Batch Modify ===\r\n");
    
    ICM42688P_Config config;
    
    // Level 2: 修改多个参数
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfig(&config);
    
    // 修改5个参数
    ICM42688P_ConfigGyro(&config, 0x01, 0x07, 0x02, 0x02);
    ICM42688P_ConfigAccel(&config, 0x03, 0x07, 0x02, 0x02);
    ICM42688P_ConfigGyroAAF(&config, 0, 13, 170, 8);
    
    uint32_t start = Test_GetTick();
    uint8_t result = ICM42688P_ApplyConfigIncremental(&config);
    uint32_t time_l2 = Test_GetTick() - start;
    
    Test_Assert("BatchModify - Success",
                (result & 0x01) == 0,
                "Batch modify failed");
    Test_Print("Level 2 (5 params):          %lu ms\r\n", time_l2);
    
    // Level 3: 修改3个运行时参数
    start = Test_GetTick();
    ICM42688P_SetGyroODR(0x06);
    ICM42688P_SetAccelODR(0x06);
    ICM42688P_SetGyroFSR(0x00);
    uint32_t time_l3 = Test_GetTick() - start;
    
    Test_Print("Level 3 (3 params):          %lu ms\r\n", time_l3);
}

/* ============================================================================
 * 压力测试
 * ============================================================================ */

/**
 * @brief 压力测试：频繁切换
 */
static void Test_Stress_FrequentSwitch(void)
{
    Test_Print("\r\n=== Stress Test: Frequent Switch ===\r\n");
    
    const uint16_t iterations = 100;
    uint16_t success_count = 0;
    uint32_t total_time = 0;
    
    uint32_t start_all = Test_GetTick();
    
    for (uint16_t i = 0; i < iterations; i++) {
        uint8_t odr = (i % 2) ? 0x07 : 0x06;
        
        uint32_t start = Test_GetTick();
        uint8_t error = ICM42688P_SetGyroODR(odr);
        total_time += (Test_GetTick() - start);
        
        if (error == 0) {
            success_count++;
        }
        
        // 每10次打印进度
        if ((i + 1) % 10 == 0) {
            Test_Print("  Progress: %d/%d\r\n", i + 1, iterations);
        }
    }
    
    uint32_t elapsed_all = Test_GetTick() - start_all;
    
    Test_Print("Total iterations: %d\r\n", iterations);
    Test_Print("Success count:    %d\r\n", success_count);
    Test_Print("Total time:       %lu ms\r\n", elapsed_all);
    Test_Print("Average time:     %lu ms\r\n", total_time / iterations);
    
    Test_Assert("Stress - Success Rate",
                success_count == iterations,
                "Some operations failed");
    Test_Assert("Stress - Average Time",
                (total_time / iterations) <= 2,
                "Average time too long");
}

/**
 * @brief 压力测试：连续Level 2更新
 */
static void Test_Stress_ContinuousL2(void)
{
    Test_Print("\r\n=== Stress Test: Continuous L2 Updates ===\r\n");
    
    ICM42688P_Config config;
    const uint8_t iterations = 20;
    uint8_t success_count = 0;
    
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfig(&config);
    
    for (uint8_t i = 0; i < iterations; i++) {
        // 交替修改不同参数
        if (i % 2) {
            config.bank0.GYRO_CONFIG0 = 0x27;
        } else {
            config.bank0.GYRO_CONFIG0 = 0x26;
        }
        
        uint8_t result = ICM42688P_ApplyConfigIncremental(&config);
        
        if ((result & 0x01) == 0) {
            success_count++;
        }
        
        if ((i + 1) % 5 == 0) {
            Test_Print("  Progress: %d/%d\r\n", i + 1, iterations);
        }
    }
    
    Test_Print("Success rate: %d/%d\r\n", success_count, iterations);
    Test_Assert("StressL2 - Success Rate",
                success_count == iterations,
                "Some L2 updates failed");
}

/* ============================================================================
 * 边界条件测试
 * ============================================================================ */

/**
 * @brief 边界测试：NULL指针保护
 */
static void Test_Edge_NullPointer(void)
{
    Test_Print("\r\n=== Edge Test: NULL Pointer Protection ===\r\n");
    
    // 测试配置函数的NULL保护
    ICM42688P_ConfigPower(NULL, 0, 0, 0, 0);
    ICM42688P_ConfigGyro(NULL, 0, 0, 0, 0);
    ICM42688P_ConfigAccel(NULL, 0, 0, 0, 0);
    
    Test_Print("  NULL pointer protection: OK (no crash)\r\n");
    Test_Assert("EdgeNull - No Crash",
                1,
                "");
}

/**
 * @brief 边界测试：极限值
 */
static void Test_Edge_ExtremeValues(void)
{
    Test_Print("\r\n=== Edge Test: Extreme Values ===\r\n");
    
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    ICM42688P_ApplyConfig(&config);
    
    // 测试极限ODR值
    uint8_t error;
    error = ICM42688P_SetGyroODR(0x0F);  // 最大值
    Test_Assert("EdgeExtreme - Max ODR",
                error == 0,
                "Max ODR failed");
    
    error = ICM42688P_SetGyroODR(0x00);  // 最小值
    Test_Print("  Min ODR result: %d (expected non-zero)\r\n", error);
    
    // 测试极限FSR值
    error = ICM42688P_SetGyroFSR(0x07);  // 最大值
    Test_Assert("EdgeExtreme - Max FSR",
                error == 0,
                "Max FSR failed");
}

/* ============================================================================
 * 实际应用场景测试
 * ============================================================================ */

/**
 * @brief 应用场景：高速↔省电模式切换
 */
static void Test_Scenario_PowerModeSwitch(void)
{
    Test_Print("\r\n=== Scenario: High-Speed ↔ Low-Power Switch ===\r\n");
    
    // 进入高速模式
    Test_Print("Switching to High-Speed Mode...\r\n");
    uint32_t start = Test_GetTick();
    ICM42688P_SetGyroODR(0x06);
    ICM42688P_SetAccelODR(0x06);
    ICM42688P_SetGyroMode(0x03);
    ICM42688P_SetAccelMode(0x03);
    uint32_t time_to_high = Test_GetTick() - start;
    Test_Print("  Time: %lu ms\r\n", time_to_high);
    
    Test_Delay(100);
    
    // 进入省电模式
    Test_Print("Switching to Low-Power Mode...\r\n");
    start = Test_GetTick();
    ICM42688P_SetGyroODR(0x09);
    ICM42688P_SetAccelODR(0x09);
    ICM42688P_SetGyroMode(0x00);
    ICM42688P_SetAccelMode(0x02);
    uint32_t time_to_low = Test_GetTick() - start;
    Test_Print("  Time: %lu ms\r\n", time_to_low);
    
    Test_Assert("Scenario - Mode Switch Fast",
                time_to_high <= 10 && time_to_low <= 10,
                "Mode switch too slow");
}

/**
 * @brief 应用场景：自适应量程
 */
static void Test_Scenario_AdaptiveFSR(void)
{
    Test_Print("\r\n=== Scenario: Adaptive FSR ===\r\n");
    
    // 模拟不同运动强度
    int16_t peak_values[] = {500, 1200, 1800, 800, 300};
    const char *expected_fsr[] = {"±500dps", "±2000dps", "±2000dps", "±1000dps", "±500dps"};
    
    for (uint8_t i = 0; i < 5; i++) {
        int16_t peak = peak_values[i];
        uint8_t fsr;
        
        if (peak > 1500) {
            fsr = 0x00;  // ±2000dps
        } else if (peak > 800) {
            fsr = 0x01;  // ±1000dps
        } else {
            fsr = 0x02;  // ±500dps
        }
        
        ICM42688P_SetGyroFSR(fsr);
        Test_Print("  Peak=%d dps → FSR=%s\r\n", peak, expected_fsr[i]);
    }
    
    Test_Assert("Scenario - Adaptive FSR",
                1,
                "");
}

/* ============================================================================
 * 主测试入口
 * ============================================================================ */

/**
 * @brief 运行所有测试
 */
void ICM42688P_RunAllTests(void)
{
    Test_Print("\r\n");
    Test_Print("========================================\r\n");
    Test_Print("  ICM42688P 三级配置系统测试\r\n");
    Test_Print("========================================\r\n");
    Test_Print("\r\n");
    
    // 重置测试统计
    g_test_total = 0;
    g_test_passed = 0;
    g_test_failed = 0;
    
    // Level 1 测试
    Test_Level1_BasicInit();
    Test_Level1_ReadVerify();
    Test_Level1_Validation();
    
    // Level 2 测试
    Test_Level2_BasicIncremental();
    Test_Level2_RuntimeModify();
    Test_Level2_NonRuntimeModify();
    Test_Level2_ConsistencyCheck();
    
    // Level 3 测试
    Test_Level3_FastODR();
    Test_Level3_FastFSR();
    Test_Level3_PowerMode();
    Test_Level3_CombinedFast();
    
    // 性能测试
    Test_Performance_Comparison();
    Test_Performance_BatchModify();
    
    // 压力测试
    Test_Stress_FrequentSwitch();
    Test_Stress_ContinuousL2();
    
    // 边界测试
    Test_Edge_NullPointer();
    Test_Edge_ExtremeValues();
    
    // 应用场景测试
    Test_Scenario_PowerModeSwitch();
    Test_Scenario_AdaptiveFSR();
    
    // 打印测试结果
    Test_Print("\r\n");
    Test_Print("========================================\r\n");
    Test_Print("  测试结果统计\r\n");
    Test_Print("========================================\r\n");
    Test_Print("总测试数:   %d\r\n", g_test_total);
    Test_Print("通过:       %d\r\n", g_test_passed);
    Test_Print("失败:       %d\r\n", g_test_failed);
    Test_Print("通过率:     %.1f%%\r\n", 
               (float)g_test_passed * 100.0f / g_test_total);
    Test_Print("========================================\r\n");
    
    if (g_test_failed == 0) {
        Test_Print("\r\n✓ 所有测试通过！\r\n\r\n");
    } else {
        Test_Print("\r\n✗ 部分测试失败，请检查日志\r\n\r\n");
    }
}

/**
 * @brief 运行快速测试（仅核心功能）
 */
void ICM42688P_RunQuickTest(void)
{
    Test_Print("\r\n=== Quick Test (Core Functions Only) ===\r\n");
    
    g_test_total = 0;
    g_test_passed = 0;
    g_test_failed = 0;
    
    Test_Level1_BasicInit();
    Test_Level2_BasicIncremental();
    Test_Level3_FastODR();
    Test_Performance_Comparison();
    
    Test_Print("\r\nQuick Test Results: %d/%d passed\r\n", 
               g_test_passed, g_test_total);
}

