/**
 ******************************************************************************
 * @file    ThreeTierConfig_Example.c
 * @brief   ICM42688P三级配置系统使用示例
 * @note    此文件仅供参考，展示如何使用三级配置系统
 ******************************************************************************
 */

#include "ICM42688P_Config.h"
#include <stdio.h>

/* ============================================================================
 * 示例1: Level 1 - 芯片初始化（完整配置）
 * ============================================================================ */
void Example_Level1_FullInit(void)
{
    ICM42688P_Config config;

    printf("========== Level 1: 完整初始化 ==========\n");

    // 初始化配置结构体
    ICM42688P_InitConfig(&config);

    // 配置电源管理
    ICM42688P_ConfigPower(&config,
                          0x03, // GYRO_MODE = 11 (低噪声模式)
                          0x03, // ACCEL_MODE = 11 (低噪声模式)
                          0,    // TEMP_DIS = 0 (温度传感器使能)
                          0);   // IDLE = 0

    // 配置陀螺仪
    ICM42688P_ConfigGyro(&config,
                         0,    // FS_SEL = 000 (±2000dps)
                         0x06, // ODR = 0110 (1kHz)
                         0x02, // UI_FILT_ORD = 10 (3阶滤波器)
                         0x01);// UI_FILT_BW = 1

    // 配置加速度计
    ICM42688P_ConfigAccel(&config,
                          0x02, // FS_SEL = 010 (±4g)
                          0x06, // ODR = 0110 (1kHz)
                          0x02, // UI_FILT_ORD = 10 (3阶滤波器)
                          0x01);// UI_FILT_BW = 1

    // 配置FIFO
    ICM42688P_ConfigFIFO(&config,
                         0x01, // FIFO_MODE = 01 (Stream-to-FIFO)
                         1,    // GYRO_EN
                         1,    // ACCEL_EN
                         1,    // TEMP_EN
                         512); // 水印512字节

    // 应用配置（耗时约700ms）
    printf("正在应用配置...\n");
    uint8_t error = ICM42688P_ApplyConfig(&config);

    if (error) {
        printf("错误：配置应用失败！\n");
    } else {
        printf("成功：配置已应用（耗时~700ms）\n");
    }
}

/* ============================================================================
 * 示例2: Level 3 - 运行时快速切换采样率
 * ============================================================================ */
void Example_Level3_FastODRSwitch(void)
{
    printf("\n========== Level 3: 快速切换采样率 ==========\n");

    // 场景：检测到高速运动，切换到高采样率
    printf("检测到高速运动，切换到1kHz采样...\n");

    uint8_t error = 0;

    // 快速切换陀螺仪ODR（无延时）
    error |= ICM42688P_SetGyroODR(0x06); // 1kHz

    // 快速切换加速度计ODR（无延时）
    error |= ICM42688P_SetAccelODR(0x06); // 1kHz

    if (error) {
        printf("警告：ODR设置失败，可能芯片损坏！\n");
    } else {
        printf("成功：已切换到1kHz（耗时~2ms）\n");
    }

    // 等待一段时间...

    // 场景：检测到静止，切换到低采样率省电
    printf("\n检测到静止，切换到100Hz省电...\n");

    error = 0;
    error |= ICM42688P_SetGyroODR(0x08);   // 100Hz
    error |= ICM42688P_SetAccelODR(0x08);  // 100Hz
    error |= ICM42688P_SetAccelMode(0x02); // 切换到LP模式省电

    if (!error) {
        printf("成功：已切换到低功耗模式（耗时~3ms）\n");
    }
}

/* ============================================================================
 * 示例3: Level 3 - 动态量程调整
 * ============================================================================ */
void Example_Level3_AdaptiveFSR(float max_gyro_rate)
{
    printf("\n========== Level 3: 自适应量程调整 ==========\n");

    uint8_t error;

    // 根据实际测量的最大角速度动态调整量程
    if (max_gyro_rate > 1500.0f) {
        // 需要更大量程
        error = ICM42688P_SetGyroFSR(0x00); // ±2000dps
        printf("切换到±2000dps量程\n");
    } else if (max_gyro_rate > 800.0f) {
        error = ICM42688P_SetGyroFSR(0x01); // ±1000dps
        printf("切换到±1000dps量程（提高精度）\n");
    } else if (max_gyro_rate > 400.0f) {
        error = ICM42688P_SetGyroFSR(0x02); // ±500dps
        printf("切换到±500dps量程（进一步提高精度）\n");
    } else {
        error = ICM42688P_SetGyroFSR(0x03); // ±250dps
        printf("切换到±250dps量程（最高精度）\n");
    }

    if (error) {
        printf("警告：量程设置失败！\n");
    }
}

/* ============================================================================
 * 示例4: Level 2 - 修改滤波器配置
 * ============================================================================ */
void Example_Level2_ReconfigureFilters(void)
{
    printf("\n========== Level 2: 重新配置滤波器 ==========\n");

    ICM42688P_Config new_config;

    // 从内部存储复制当前配置
    // 注意：g_internal_config是库内部变量，实际应用中应该维护自己的配置副本
    // 这里仅作示例，实际使用时应该：
    // memcpy(&new_config, &my_current_config, sizeof(ICM42688P_Config));

    // 修改陀螺仪AAF滤波器带宽到1051Hz
    ICM42688P_ConfigGyroAAF(&new_config,
                            0,   // enable = 0 (使能AAF)
                            22,  // delt = 22
                            488, // deltsqr = 488
                            6);  // bitshift = 6

    // 修改加速度计AAF滤波器带宽到536Hz
    ICM42688P_ConfigAccelAAF(&new_config,
                             0,   // enable = 0 (使能AAF)
                             12,  // delt = 12
                             144, // deltsqr = 144
                             8);  // bitshift = 8

    // 应用增量配置
    printf("正在应用增量配置...\n");
    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);

    // 检查结果
    if (result & 0x01) {
        printf("错误：配置写入失败！\n");
    } else if (result & 0x02) {
        printf("警告：检测到配置不一致，但已成功覆写\n");
        printf("成功：滤波器已重新配置（耗时~50ms）\n");
    } else {
        printf("成功：滤波器已重新配置（耗时~50ms）\n");
    }
}

/* ============================================================================
 * 示例5: Level 2 - 只修改运行时参数（不需要关闭传感器）
 * ============================================================================ */
void Example_Level2_RuntimeParamsOnly(void)
{
    printf("\n========== Level 2: 只修改运行时参数 ==========\n");

    ICM42688P_Config new_config;

    // 假设已有当前配置
    // memcpy(&new_config, &my_current_config, sizeof(ICM42688P_Config));

    // 修改ODR和FSR（都是运行时参数）
    ICM42688P_ConfigGyro(&new_config, 0x01, 0x07, 0x02, 0x01); // ±1000dps, 200Hz
    ICM42688P_ConfigAccel(&new_config, 0x03, 0x07, 0x02, 0x01); // ±2g, 200Hz

    // 应用配置（由于只修改运行时参数，不会关闭传感器）
    printf("正在应用配置（仅运行时参数）...\n");
    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);

    if (!(result & 0x01)) {
        printf("成功：配置已更新，传感器未中断（耗时~5ms）\n");
    }
}

/* ============================================================================
 * 示例6: Level 2 - 混合修改（需要关闭传感器）
 * ============================================================================ */
void Example_Level2_MixedParams(void)
{
    printf("\n========== Level 2: 混合参数修改 ==========\n");

    ICM42688P_Config new_config;

    // 假设已有当前配置
    // memcpy(&new_config, &my_current_config, sizeof(ICM42688P_Config));

    // 修改ODR（运行时参数）
    ICM42688P_ConfigGyro(&new_config, 0, 0x0F, 0x02, 0x01); // ±2000dps, 500Hz

    // 修改FIFO配置（非运行时参数）
    ICM42688P_ConfigFIFO(&new_config, 0x02, 1, 1, 1, 1024); // STOP-on-FULL, 1024字节水印

    // 应用配置（会自动关闭传感器→修改→开启）
    printf("正在应用混合配置（需要关闭传感器）...\n");
    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);

    if (!(result & 0x01)) {
        printf("成功：配置已更新（耗时~100ms，包含传感器重启）\n");
    }
}

/* ============================================================================
 * 示例7: 三级系统组合使用
 * ============================================================================ */
void Example_CombinedUsage(void)
{
    printf("\n========== 组合使用示例 ==========\n");

    // 阶段1：初始化（Level 1）
    printf("\n阶段1：系统初始化\n");
    Example_Level1_FullInit();

    // 等待传感器稳定
    delay_ms(100);

    // 阶段2：运行时动态调整（Level 3）
    printf("\n阶段2：运行时动态调整\n");
    for (int i = 0; i < 5; i++) {
        printf("  循环 %d: ", i);

        // 模拟检测运动状态，动态切换采样率
        if (i % 2 == 0) {
            ICM42688P_SetGyroODR(0x06);  // 1kHz
            printf("高速采样\n");
        } else {
            ICM42688P_SetGyroODR(0x08);  // 100Hz
            printf("低速采样\n");
        }

        delay_ms(1000); // 每秒切换一次
    }

    // 阶段3：重新配置滤波器（Level 2）
    printf("\n阶段3：重新配置滤波器参数\n");
    Example_Level2_ReconfigureFilters();

    printf("\n========== 测试完成 ==========\n");
}

/* ============================================================================
 * 性能测试示例
 * ============================================================================ */
void Example_PerformanceTest(void)
{
    printf("\n========== 性能测试 ==========\n");

    uint32_t start_time, end_time;

    // 测试1：Level 1性能
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);

    start_time = HAL_GetTick();
    ICM42688P_ApplyConfig(&config);
    end_time = HAL_GetTick();
    printf("Level 1 完整应用耗时: %lu ms\n", end_time - start_time);

    // 测试2：Level 3性能
    start_time = HAL_GetTick();
    ICM42688P_SetGyroODR(0x07);
    end_time = HAL_GetTick();
    printf("Level 3 单参数修改耗时: %lu ms\n", end_time - start_time);

    // 测试3：Level 2性能（仅运行时参数）
    ICM42688P_Config new_config;
    memcpy(&new_config, &config, sizeof(ICM42688P_Config));
    new_config.bank0.GYRO_CONFIG0 = 0x26; // 修改ODR

    start_time = HAL_GetTick();
    ICM42688P_ApplyConfigIncremental(&new_config);
    end_time = HAL_GetTick();
    printf("Level 2 增量更新（运行时）耗时: %lu ms\n", end_time - start_time);

    // 测试4：Level 2性能（需要关闭传感器）
    new_config.bank0.FIFO_CONFIG = 0x40; // 修改FIFO模式

    start_time = HAL_GetTick();
    ICM42688P_ApplyConfigIncremental(&new_config);
    end_time = HAL_GetTick();
    printf("Level 2 增量更新（需关闭传感器）耗时: %lu ms\n", end_time - start_time);
}

/* ============================================================================
 * 错误处理示例
 * ============================================================================ */
void Example_ErrorHandling(void)
{
    printf("\n========== 错误处理示例 ==========\n");

    // Level 3错误处理
    uint8_t error = ICM42688P_SetGyroODR(0x06);
    if (error) {
        printf("错误：陀螺仪ODR设置失败！\n");
        printf("可能原因：\n");
        printf("  1. 芯片未正确初始化\n");
        printf("  2. SPI通信故障\n");
        printf("  3. 芯片损坏\n");
        printf("建议：检查硬件连接和电源\n");
    }

    // Level 2错误处理
    ICM42688P_Config new_config;
    ICM42688P_InitConfig(&new_config);

    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);

    if (result & 0x01) {
        printf("错误：增量配置写入失败！\n");
    }

    if (result & 0x02) {
        printf("警告：检测到配置不一致\n");
        printf("可能原因：\n");
        printf("  1. 上次配置写入失败但未检查返回值\n");
        printf("  2. 其他代码直接修改了寄存器\n");
        printf("  3. 芯片意外复位\n");
        printf("已自动覆写为新配置，可以继续使用\n");
    }

    if (result == 0x00) {
        printf("成功：配置一致且写入成功\n");
    }
}

/* ============================================================================
 * 实际应用场景示例
 * ============================================================================ */

/**
 * @brief 场景1：自适应采样率控制
 * @note 根据运动状态动态调整采样率，节省功耗
 */
void Scenario_AdaptiveSampling(float motion_magnitude)
{
    if (motion_magnitude > 100.0f) {
        // 高速运动：使用高采样率
        ICM42688P_SetGyroODR(0x05);   // 2kHz
        ICM42688P_SetAccelODR(0x05);  // 2kHz
    } else if (motion_magnitude > 10.0f) {
        // 中速运动：使用标准采样率
        ICM42688P_SetGyroODR(0x06);   // 1kHz
        ICM42688P_SetAccelODR(0x06);  // 1kHz
    } else {
        // 低速运动：降低采样率省电
        ICM42688P_SetGyroODR(0x08);   // 100Hz
        ICM42688P_SetAccelODR(0x08);  // 100Hz
        ICM42688P_SetAccelMode(0x02); // 切换到LP模式
    }
}

/**
 * @brief 场景2：运动检测模式切换
 * @note 在激活模式和待机模式之间切换
 */
void Scenario_MotionDetection(uint8_t motion_detected)
{
    if (motion_detected) {
        // 检测到运动，激活传感器
        ICM42688P_SetGyroMode(0x03);  // 陀螺仪LN模式
        ICM42688P_SetAccelMode(0x03); // 加速度计LN模式
    } else {
        // 无运动，进入低功耗
        ICM42688P_SetGyroMode(0x00);  // 关闭陀螺仪
        ICM42688P_SetAccelMode(0x02); // 加速度计LP模式
    }
}

/**
 * @brief 场景3：完整的应用流程
 */
void Scenario_CompleteFlow(void)
{
    printf("\n========== 完整应用流程 ==========\n");

    // 步骤1：系统初始化（Level 1）
    printf("步骤1：初始化系统...\n");
    ICM42688P_Config config;
    ICM42688P_InitConfig(&config);
    ICM42688P_ConfigPower(&config, 0x03, 0x03, 0, 0);
    ICM42688P_ConfigGyro(&config, 0, 0x06, 0x02, 0x01);
    ICM42688P_ConfigAccel(&config, 0x02, 0x06, 0x02, 0x01);
    ICM42688P_ApplyConfig(&config); // ~700ms
    printf("  完成（耗时~700ms）\n");

    delay_ms(100);

    // 步骤2：运行时调整（Level 3）
    printf("\n步骤2：运行时快速调整...\n");
    for (int i = 0; i < 10; i++) {
        // 每100ms切换一次采样率
        ICM42688P_SetGyroODR((i % 2) ? 0x06 : 0x08);
        delay_ms(100);
    }
    printf("  完成（10次切换，总耗时~1秒）\n");

    // 步骤3：批量修改配置（Level 2）
    printf("\n步骤3：批量修改配置...\n");
    ICM42688P_Config new_config;
    memcpy(&new_config, &config, sizeof(ICM42688P_Config));

    // 修改多个参数
    ICM42688P_ConfigGyroAAF(&new_config, 0, 22, 488, 6);
    ICM42688P_ConfigAccelAAF(&new_config, 0, 12, 144, 8);
    ICM42688P_ConfigFIFO(&new_config, 0x01, 1, 1, 1, 512);

    uint8_t result = ICM42688P_ApplyConfigIncremental(&new_config);
    if (result == 0x00) {
        printf("  完成（耗时~50ms）\n");
    } else {
        printf("  结果码: 0x%02X\n", result);
    }

    printf("\n流程结束\n");
}

/* ============================================================================
 * 主测试函数
 * ============================================================================ */
void ThreeTierConfig_RunAllExamples(void)
{
    printf("\n");
    printf("=============================================\n");
    printf("  ICM42688P 三级配置系统测试示例\n");
    printf("=============================================\n");

    // 运行所有示例
    Example_Level1_FullInit();
    delay_ms(500);

    Example_Level3_FastODRSwitch();
    delay_ms(500);

    Example_Level3_AdaptiveFSR(1200.0f);
    delay_ms(500);

    Example_Level2_ReconfigureFilters();
    delay_ms(500);

    Example_ErrorHandling();
    delay_ms(500);

    // 性能测试
    Example_PerformanceTest();

    printf("\n所有测试完成！\n");
}

