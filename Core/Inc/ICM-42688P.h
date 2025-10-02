#ifndef ICM42688P_H
#define ICM42688P_H

#include "main.h"
#include "math.h"
#include "float.h"

#define cs_high() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);
#define cs_low() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);

#define GYRO_FULL_SCALE 2000.0 // �����ǵ�ȫ���̣���2000dps��
#define GYRO_SENSITIVITY (GYRO_FULL_SCALE / 32768.0) // �����ǵķֱ���Ϊ16λ

#define ACCEL_FULL_SCALE 16.0 // ���ٶȼƵ�ȫ���̣���16g��
#define ACCEL_SENSITIVITY (ACCEL_FULL_SCALE / 32768.0) // ���ٶȼƵķֱ���Ϊ16λ


//#define axzeroffset -4.192f
//#define ayzeroffset 0.266f
//#define azzeroffset 0.128f //IMU1

//#define gxzeroffset 0.886f
//#define gyzeroffset 0.5f
//#define gzzeroffset -0.18f //IMU1


#define axzeroffset 0.0f
#define ayzeroffset 0.0f
#define azzeroffset 0.0f //IMU2

#define gxzeroffset 0.0f
#define gyzeroffset 0.0f
#define gzzeroffset 0.0f //IMU2

#define ICM42688P_READ 0x80


typedef struct {
    // ������ٶȼ����ݣ���λΪg���������ٶȣ�
    double accel_x;  // X����ٶ�
    double accel_y;  // Y����ٶ�
    double accel_z;  // Z����ٶ�

    // �������������ݣ���λΪ��ÿ�루dps��
    double gyro_x;   // X����ٶ�
    double gyro_y;   // Y����ٶ�
    double gyro_z;   // Z����ٶ�

    // ��Ԫ�������ڱ�ʾ�豸����ת״̬
    double q0;  // q0������ʵ��
    double q1;  // q1
    double q2;  // q2
    double q3;  // q3

    // ʱ�����������ϵͳʱ�����ĳ����׼�����������
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