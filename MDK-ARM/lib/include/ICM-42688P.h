#ifndef ICM42688P_H
#define ICM42688P_H

#include "main.h"

#define cs_high() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);
#define cs_low() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);

#define GYRO_FULL_SCALE 2000.0 // 陀螺仪的全量程（±2000dps）
#define GYRO_SENSITIVITY (GYRO_FULL_SCALE / 32768.0) // 陀螺仪的分辨率为16位

#define ACCEL_FULL_SCALE 16.0 // 加速度计的全量程（±16g）
#define ACCEL_SENSITIVITY (ACCEL_FULL_SCALE / 32768.0) // 加速度计的分辨率为16位

#define ICM42688P_READ 0x80


typedef struct {
    // 三轴加速度计数据，单位为g（重力加速度）
    double accel_x;  // X轴加速度
    double accel_y;  // Y轴加速度
    double accel_z;  // Z轴加速度

    // 三轴陀螺仪数据，单位为度每秒（dps）
    double gyro_x;   // X轴角速度
    double gyro_y;   // Y轴角速度
    double gyro_z;   // Z轴角速度

    // 四元数，用于表示设备的旋转状态
    double q0;  // q0，代表实部
    double q1;  // q1
    double q2;  // q2
    double q3;  // q3

    // 时间戳（可以是系统时间或自某个基准点起的秒数）
    uint64_t timestamp;

} IMU_Data;







void ICM42688P_Init();
void ICM42688P_Stop();
void ICM42688P_Start();
void ICM42688P_ODRcfg();
void ICM42688P_INT_Cfg();
float ICM42688P_GetTemp();
void ICM42688P_CLK_Config();
void ICM42688P_SoftwareReset();
void ICM42688P_ReadIMUData(IMU_Data *data);
void ICM42688P_BankSEL(unsigned char bank);
void ICM42688P_ReadRegister(uint8_t reg_address,uint8_t* rxdata,uint8_t length);
uint8_t ICM42688P_WriteRegister(uint8_t reg_address,uint8_t* txdata,uint8_t length);







#endif