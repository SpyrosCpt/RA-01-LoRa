#include "preprocessor.h"

void SPI_Send(UI8 data)
{
	while (!(SPI1->SR & SPI_SR_TXE)); // Wait until TX buffer is empty
	SPI1->DR = data; // Send byte to SPI (TXE cleared)
	while (SPI1->SR & SPI_SR_BSY); // Wait until the transmission is complete
	
	
}

UI8 SPI_Receive(UI8 data)
{
	SPI1->DR = data; // Send byte to SPI (TXE cleared)
	delayus(10);
	//while (!(SPI1->SR & SPI_SR_RXNE)); // Wait while receive buffer is empty

	return (SPI1->DR); // Return received byte
}
/* BB= BitBang */
UI8 BB_ReadData(void)
{
	UI8 i = 0;
	UI8 j = 0;
	UI8 k = 7;
	UI8 data = 0;
	UI8 temp = 0;
	for(i = 0; i < 8; i++)
	{
		CLK_SET();
		if(MISO_READ()!=0) 
		{
			temp = 1<<k;
			data |= temp;
		}
		for(j = 0; j < 20; j++) ;
		CLK_CLR();
		for(j = 0; j < 20; j++) ;
		k--;
	}
	CLK_CLR();
	MOSI_CLR();
	
	for(j = 0; j < 50; j++) ;
	
	return data;
}

UI8 BB_ReadData_9(void)
{
	UI8 i = 0;
	UI8 j = 0;
	UI8 k = 8;
	UI16 data = 0;
	UI16 temp = 0;
	for(i = 0; i < 9; i++)
	{
		CLK_SET();
		if(MISO_READ() != 0) 
		{
			temp = 1<<k;
			data |= temp;
		}
		for(j = 0; j < 20; j++) ;
		CLK_CLR();
		for(j = 0; j < 20; j++) ;
		k--;
	}
	CLK_CLR();
	MOSI_CLR();
	
	for(j = 0; j < 50; j++) ;
	
	return (UI8)data;
}

void BB_WriteData(UI8 data)
{
	UI8 i = 0;
	UI16 j = 0;
	UI16 k = 7;
	
	for(i = 0; i < 8; i++)
	{
		if(((data>>k) & 0x01) == 1) MOSI_SET();
		else MOSI_CLR();
		CLK_SET();
		for(j = 0; j < 2; j++) ;
		CLK_CLR();
		for(j = 0; j < 2; j++) ;
		k--;
	}
	CLK_CLR();
	MOSI_CLR();
	
	for(j = 0; j < 5; j++) ;
}

/* Functions to interface with the AT93C46 */

void EE_EWEN(void)
{
	DF_CS_CLR();
	DF_CS_SET(); 

	BB_WriteData(0x02);
	BB_WriteData(0x60);
	
	DF_CS_CLR();
	delayms(10);
}

UI8 EE_ERASE(UI8 address)
{
	UI8 temp = 0;
	
	EE_EWEN();
	
	DF_CS_CLR();
	DF_CS_SET(); 
	
	BB_WriteData(0x03);
	BB_WriteData(0x80|address);
	
	DF_CS_CLR(); 
	DF_CS_SET();

	while(MISO_READ() == 0) 
	{
		temp++;
		
		delayms(1);
		
		if(temp == 255) break;
	}
	
	DF_CS_CLR();
	
	delayms(10);
	
	if(temp == 255) return 0;
	
	return 1;
}

UI8 EE_WRITE(UI8 address, UI8 data)
{
	UI8 temp = 0;
	
	DF_CS_CLR();
	DF_CS_SET(); 
	
	EE_EWEN();
	
	DF_CS_CLR();
	DF_CS_SET();
	
	BB_WriteData(0x02);
	BB_WriteData(0x80|address);
	BB_WriteData(data);
	
	DF_CS_CLR(); 
	DF_CS_SET();

	while(MISO_READ() == 0) 
	{
		temp++;
		
		delayms(1);
		
		if(temp == 255) break;
	}
	
	DF_CS_CLR();
	
	delayms(10);
	
	if(temp == 255) return 0;
	
	return 1;
}

UI8 EE_READ(UI8 address)
{
	UI8 data = 0;
//	UI8 data2 = 0;
//	UI16 temp = 0;
	DF_CS_CLR();
	DF_CS_SET(); 
	
  BB_WriteData(0x03);
	BB_WriteData(address);
	delayms(1);
	
	data = BB_ReadData();
	
	DF_CS_CLR();

 /* data = data << 1; // bitwise shift to get rid of the preceding 0
	
	if ((data2>>7) == 1)
		{
      data = data + 1; // change LSB if needed
    }*/
	return data;
}

void EE_EWDS(void)
{
	DF_CS_CLR();
	DF_CS_SET(); 

	BB_WriteData(0x08);
	BB_WriteData(0x00);
	
	DF_CS_CLR();
}

UI8 EE_ERASE_ALL(void)
{
	UI8 temp = 0;
	
	DF_CS_CLR();
	DF_CS_SET(); 
	BB_WriteData(0x02);
	BB_WriteData(0x40);
	DF_CS_CLR();
	DF_CS_SET(); 
	while(MISO_READ() == 0) 
	{
		temp++;
		delayms(1);
		if(temp == 255) break;
	}
	
	DF_CS_CLR();
	
	delayms(10);
	
	if(temp == 255) return 0;
	
	return 1;
}
