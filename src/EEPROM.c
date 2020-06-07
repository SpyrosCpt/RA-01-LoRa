#include "preprocessor.h"

/**
	* @brief 	This function reads data via SPI (bit bang)
	* @param 	None
	* @retval data: data returned 
	*/
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

/**
	* @brief 	This function writes data via SPI (bit bang)
	* @param 	data: data to write
	* @retval none
	*/
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

/**
	* @brief 	This function enables erase and write 
	* @param 	None
	* @retval None
	*/
void EE_EWEN(void)
{
	DF_CS_CLR();
	DF_CS_SET(); 

	BB_WriteData(0x02);
	BB_WriteData(0x60);
	
	DF_CS_CLR();
	delayms(10);
}

/**
	* @brief 	This function erases a byte of data at a specific address
	* @param 	address: address of the datat to erase
	* @retval None
	*/
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

/**
	* @brief 	This function writes data to the EEPROM
	* @param 	address: address to write data
	*					data: byte of data to write	
	* @retval data: data returned 
	*/
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

/**
	* @brief 	This function reads a byte of data from the EEPROM
	* @param 	address: address to read byte from 
	* @retval None
	*/
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

/**
	* @brief 	This function disables erase and write
	* @param 	None
	* @retval None
	*/
void EE_EWDS(void)
{
	DF_CS_CLR();
	DF_CS_SET(); 

	BB_WriteData(0x08);
	BB_WriteData(0x00);
	
	DF_CS_CLR();
}

/**
	* @brief 	This function erases all data in EEPROM
	* @param 	None
	* @retval None
	*/
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
