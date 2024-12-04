#include "ICM-42688P.h"


void spi_sendbytes(unsigned char *bytes,unsigned char length)
{
	extern SPI_HandleTypeDef hspi1;
	
	HAL_SPI_Transmit(&hspi1, bytes, length, 0xff);
	
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
}

void ICM42688P_SoftwareReset()
{
	uint8_t address = 0x11;
	uint8_t txdata = 0x01;
	cs_low();
  spi_sendbytes(&address,1);
	spi_sendbytes(&txdata,1);
  cs_high();
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
