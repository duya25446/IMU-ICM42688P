#include "ICM-42688P.h"






uint8_t ICM42688P_ReadRegister(uint8_t reg)
{
    uint8_t rx_data = 0;
    uint8_t tx_data = reg | ICM42688P_READ;  // 添加读取标志位
    extern SPI_HandleTypeDef hspi1;
    // 拉低CS引脚（假设CS引脚连接在GPIOA的PIN4上）
    cs_low();
    
    // 发送寄存器地址并读取数据
    HAL_SPI_Transmit(&hspi1, &tx_data, 1, 100);
    HAL_SPI_Receive(&hspi1, &rx_data, 1, 100);
    
    // 拉高CS引脚
    cs_high();
    
    return rx_data;
}


