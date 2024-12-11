#include "ICM-42688P.h"




void spi_sendbytes(unsigned char *bytes,unsigned char length)
{
	extern SPI_HandleTypeDef hspi1;
	
	HAL_SPI_Transmit(&hspi1, bytes, length, 0xff);
	
}

void delay(unsigned int ms)
{
	HAL_Delay(ms);
}

void spi_readbytes(unsigned char *bytes,unsigned char length)
{
	extern SPI_HandleTypeDef hspi1;
	
	HAL_SPI_Receive(&hspi1, bytes, length, 0xff);
}

void ICM42688P_Init()
{
	cs_high();
	ICM42688P_SoftwareReset();
	delay(10);
	ICM42688P_CLK_Config();
	//ICM42688P_INT_Cfg();
	ICM42688P_ODRcfg();
	ICM42688P_Start();
	delay(10);
	
}

void ICM42688P_BankSEL(unsigned char bank)
{
	unsigned char config = bank;
	ICM42688P_WriteRegister(0x76,&config,1);
}

void ICM42688P_SoftwareReset()
{
	uint8_t address = 0x11;
	uint8_t txdata = 0x01;
	ICM42688P_BankSEL(0);
	cs_low();
  spi_sendbytes(&address,1);
	spi_sendbytes(&txdata,1);
  cs_high();
}

void ICM42688P_Start()
{
	ICM42688P_BankSEL(0);
	unsigned char address = 0x4e;
	unsigned char config = 0b00001111;
	ICM42688P_WriteRegister(address,&config,1);
}

void ICM42688P_Stop()
{
	ICM42688P_BankSEL(0);
	unsigned char address = 0x4e;
	unsigned char config = 0;
	ICM42688P_WriteRegister(address,&config,1);
}

void ICM42688P_ODRcfg()
{
	ICM42688P_BankSEL(0);
	unsigned char config = 1;
	ICM42688P_WriteRegister(0x4f,&config,1);
	ICM42688P_WriteRegister(0x50,&config,1);
}

void ICM42688P_CLK_Config()
{
	ICM42688P_BankSEL(1);
	unsigned char config = 0x04;
	ICM42688P_WriteRegister(0x7b,&config,1);
	ICM42688P_BankSEL(0);
	config = 0x95;
	ICM42688P_WriteRegister(0x4d,&config,1);
}

void ICM42688P_INT_Cfg()
{
	ICM42688P_BankSEL(0);
	unsigned char config = 0x2;
	ICM42688P_WriteRegister(0x14,&config,1);
	config = 0x8;
	ICM42688P_WriteRegister(0x65,&config,1);
	
}
#include <stdint.h>

void parse12BytesToSixInt16(uint8_t *data, int16_t *output) {
    for (int i = 0; i < 6; i++) {
        // 每个int16占两个字节
        // 先获取高位字节然后移位到正确的位置
        output[i] = (int16_t)(data[i * 2] << 8); 
        // 然后把低位字节放在低位
        output[i] |= data[i * 2 + 1];
    }
}

void parseImuDataToInt6(int16_t *input, double *output) {
    for(int i = 0; i < 6; i++) {
        if(i < 3) { // 前三个是加速度计数据
            output[i] = (double)input[i] * ACCEL_SENSITIVITY; // 加速度计
        } else { // 后三个是陀螺仪数据
            output[i] = (double)input[i] * GYRO_SENSITIVITY; // 陀螺仪
        }
    }
}

void ICM42688P_ReadIMUData(IMU_Data *data)
{
	unsigned char odata[12];
	short int16data[6];
	double f64data[6];
	ICM42688P_BankSEL(0);
	ICM42688P_ReadRegister(0x1F,odata,12);
	parse12BytesToSixInt16(odata,int16data);
	parseImuDataToInt6(int16data,f64data);
	data->accel_x = f64data[0];
	data->accel_y = f64data[1];
	data->accel_z = f64data[2];
	data->gyro_x = f64data[3];
	data->gyro_y = f64data[4];
	data->gyro_z = f64data[5];
}

	
float ICM42688P_GetTemp()
{
	float temp = 0;
	unsigned char buffer[2];
	unsigned char ex;
	unsigned short ptr;
	ICM42688P_BankSEL(0);
	ICM42688P_ReadRegister(0x1d,buffer+1,1);
	ICM42688P_ReadRegister(0x1e,buffer,1);
	ptr = *(unsigned short*)buffer;
	temp = (((ptr)/132.48))+25;
	
	return temp;
}	

void ICM42688P_ReadRegister(uint8_t reg_address,uint8_t* rxdata,uint8_t length)
{
  uint8_t tx_data = reg_address | ICM42688P_READ; 
	cs_low();
  spi_sendbytes(&tx_data,1);
	spi_readbytes(rxdata,length);
  cs_high();
}



uint8_t ICM42688P_WriteRegister(uint8_t reg_address,uint8_t* txdata,uint8_t length)
{
	uint8_t rxbuffer[256];
	uint8_t cont = 0;
	uint8_t tx_data = reg_address; 
	uint8_t error = 0;
	cs_low();
  spi_sendbytes(&tx_data,1);
	spi_sendbytes(txdata,length);
  cs_high();
	ICM42688P_ReadRegister(reg_address,rxbuffer,length);
	for(cont = 0; cont < length; cont++)
	{
		if(rxbuffer[cont]!=txdata[cont])
		{
			return 1;
		}
	}
	return 0;
}
