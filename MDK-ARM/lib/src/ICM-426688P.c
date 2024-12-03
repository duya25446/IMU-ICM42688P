#include "ICM-42688P.h"






uint8_t ICM42688P_ReadRegister(uint8_t reg)
{
    uint8_t rx_data = 0;
    uint8_t tx_data = reg | ICM42688P_READ;  // ��Ӷ�ȡ��־λ
    extern SPI_HandleTypeDef hspi1;
    // ����CS���ţ�����CS����������GPIOA��PIN4�ϣ�
    cs_low();
    
    // ���ͼĴ�����ַ����ȡ����
    HAL_SPI_Transmit(&hspi1, &tx_data, 1, 100);
    HAL_SPI_Receive(&hspi1, &rx_data, 1, 100);
    
    // ����CS����
    cs_high();
    
    return rx_data;
}


