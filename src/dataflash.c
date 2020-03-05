/******** Example Code : Accessing Atmel AT45Dxxx dataflash on STK500 *******

Device      : 	AT90S8515

File name   : 	dataflash.c

Description : 	Functions to access the Atmel AT45Dxxx dataflash series
      				  Supports 512Kbit - 64Mbit

Last change:    16 Aug 2001   AS

****************************************************************************/


#include "preprocessor.h"

#include "dataflash.h"

// Constants
//Look-up table for these sizes ->  512k, 1M, 2M, 4M, 8M, 16M, 32M, 64M
const unsigned char DF_pagebits[]  ={  9,  9,  9,  9,  9,  10,  10,  11};	//index of internal page address bits

// Globals
unsigned char PageBits;

// Functions

/*****************************************************************************
*
*	Function name : DF_SPI_init
*
*	Returns :		None
*
*	Parameters :	None
*
*	Purpose :		Sets up the HW SPI in Master mode, Mode 3
*					Note -> Uses the SS line to control the DF CS-line.
*
******************************************************************************/
void DF_SPI_init (void)
{
//                  6          4          2            3
//	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA) | (1<<CPOL);	//Enable SPI in Master mode, mode 3
}


/*****************************************************************************
*
*	Function name : DF_SPI_RW
*
*	Returns :		Byte read from SPI data register (any value)
*
*	Parameters :	Byte to be written to SPI data register (any value)
*
*	Purpose :		Read and writes one byte from/to SPI master
*
******************************************************************************/
UI16 DF_SPI_RW (unsigned char output)
{
	/*unsigned char input;

	SPIC.DATA = output;							//put byte 'output' in SPI data register
	while(!(SPIC.STATUS & 0x80));					//wait for transfer complete, poll SPIF-flag
	input = SPIC.DATA;							//read value in SPI data reg.

	return input;							//return the byte clocked in from SPI slave*/
	
	/*UI8 data1=0;

	SPI_SendData8(SPI1, ( output ) );
	while( ( SPI1->SR & SPI_SR_TXE ) == 0 );	//wait for data to be sent
	
	data1 = SPI_ReceiveData8( SPI1 );
	while( ( SPI1->SR & SPI_SR_RXNE ) == 0 );
	
	for( int i = 0; i < 55; i++ );
	
	return data1;*/
	
	
	UI16 data1=0;
	
	SPI_SendData8(SPI1, ( output ) );
	while( ( SPI1->SR & SPI_SR_TXE ) == 0 );	//wait for data to be sent
	
	data1 = SPI_I2S_ReceiveData16( SPI1 );// SPI_ReceiveData8( SPI1 );
	while( ( SPI1->SR & SPI_SR_RXNE ) == 0 );
	for( int i = 0; i < 55; i++ );
	
	return data1;
}


/*****************************************************************************
*
*	Function name : Read_DF_status
*
*	Returns :		One status byte. Consult Dataflash datasheet for further
*					decoding info
*
*	Parameters :	None
*
*	Purpose :		Status info concerning the Dataflash is busy or not.
*					Status info concerning compare between buffer and flash page
*					Status info concerning size of actual device
*
******************************************************************************/
unsigned char Read_DF_status (void)
{
	UI16 result,index_copy;

//	DF_CS_inactive;							//make sure to toggle CS signal in order
//	DF_CS_active;							//to reset dataflash command decoder
	
//	GPIOB->BSRR |= (1<<7);
	//GPIOB->BRR |= (1<<7);
	
	result = DF_SPI_RW(StatusReg);			//send status register read op-code
	result = DF_SPI_RW(0x00);				//dummy write to get result

	index_copy = ((result & 0x38) >> 3);	//get the size info from status register
	PageBits   = DF_pagebits[index_copy];	//get number of internal page address bits from look-up table

	return result;							//return the read status register value
}

/*****************************************************************************
*
*	Function name : Page_To_Buffer
*
*	Returns :		None
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					PageAdr		->	Address of page to be transferred to buffer
*
*	Purpose :		Transfers a page from flash to dataflash SRAM buffer
*
******************************************************************************/
void Page_To_Buffer (unsigned int PageAdr)
{
//	DF_CS_inactive;												//make sure to toggle CS signal in order
	//DF_CS_active;												//to reset dataflash command decoder
	
		DF_SPI_RW(FlashToBuf1Transfer);							//transfer to buffer 1 op-code
		DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));	//upper part of page address
		DF_SPI_RW((unsigned char)(PageAdr << (PageBits - 8)));	//lower part of page address
		DF_SPI_RW(0x00);										//don't cares

//	DF_CS_inactive;												//initiate the transfer
//	DF_CS_active;

	while(!(Read_DF_status() & 0x80));							//monitor the status register, wait until busy-flag is high

}
//#define DF_CS_active	DF_CS_CLR()

//#define DF_CS_inactive	DF_CS_SET()


/*****************************************************************************
*
*	Function name : Buffer_Read_Byte
*
*	Returns :		One read byte (any value)
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					IntPageAdr	->	Internal page address
*
*	Purpose :		Reads one byte from one of the dataflash
*					internal SRAM buffers
*
******************************************************************************/
UI16 Buffer_Read_Byte (unsigned int IntPageAdr)
{
	UI16 data;

	//DF_CS_inactive;								//make sure to toggle CS signal in order
	//DF_CS_active;								//to reset dataflash command decoder
	
		DF_SPI_RW(Buf1Read);					//buffer 1 read op-code
		DF_SPI_RW(0x00);						//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
		DF_SPI_RW(0x00);						//don't cares
		data = DF_SPI_RW(0x00);					//read byte

	return data;								//return the read data byte
}



/*****************************************************************************
*
*	Function name : Buffer_Read_Str
*
*	Returns :		None
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					IntPageAdr	->	Internal page address
*					No_of_bytes	->	Number of bytes to be read
*					*BufferPtr	->	address of buffer to be used for read bytes
*
*	Purpose :		Reads one or more bytes from one of the dataflash
*					internal SRAM buffers, and puts read bytes into
*					buffer pointed to by *BufferPtr
*
******************************************************************************/
void Buffer_Read_Str (unsigned int IntPageAdr, unsigned int No_of_bytes, unsigned char *BufferPtr)
{
	unsigned int i;

//	DF_CS_inactive;								//make sure to toggle CS signal in order
//	DF_CS_active;								//to reset dataflash command decoder

		DF_SPI_RW(Buf1Read);					//buffer 1 read op-code
		DF_SPI_RW(0x00);						//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
		DF_SPI_RW(0x00);						//don't cares
		for( i=0; i<No_of_bytes; i++)
		{
			*(BufferPtr) = DF_SPI_RW(0x00);		//read byte and put it in AVR buffer pointed to by *BufferPtr
			BufferPtr++;						//point to next element in AVR buffer
		}

}




//added by carel..
void Buffer_Read_Str_Pos (unsigned int IntPageAdr, unsigned int IntBuffAdr, unsigned int No_of_bytes, unsigned char *BufferPtr)
{
	unsigned int i;

//	DF_CS_inactive;								//make sure to toggle CS signal in order
	//DF_CS_active;								//to reset dataflash command decoder

BufferPtr+=IntBuffAdr;//advance pointer to correct location

		DF_SPI_RW(Buf1Read);					//buffer 1 read op-code
		DF_SPI_RW(0x00);						//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
		DF_SPI_RW(0x00);						//don't cares
		for( i=0; i<No_of_bytes; i++)
		{
			*(BufferPtr) = DF_SPI_RW(0x00);		//read byte and put it in AVR buffer pointed to by *BufferPtr
			BufferPtr++;						//point to next element in AVR buffer
		}

}


//NB : Sjekk at (IntAdr + No_of_bytes) < buffersize, hvis ikke blir det bare ball..



/*****************************************************************************
*
*	Function name : Buffer_Write_Enable
*
*	Returns :		None
*
*	Parameters :	IntPageAdr	->	Internal page address to start writing from
*					BufferAdr	->	Decides usage of either buffer 1 or 2
*
*	Purpose :		Enables continous write functionality to one of the dataflash buffers
*					buffers. NOTE : User must ensure that CS goes high to terminate
*					this mode before accessing other dataflash functionalities
*
******************************************************************************/
//void Buffer_Write_Enable (unsigned char BufferNo, unsigned int IntPageAdr)
//{
//	DF_CS_inactive;								//make sure to toggle CS signal in order
//	DF_CS_active;								//to reset dataflash command decoder
//
//	if (1 == BufferNo)							//write enable to buffer 1
//	{
//		DF_SPI_RW(Buf1Write);					//buffer 1 write op-code
//		DF_SPI_RW(0x00);						//don't cares
//		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
//		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
//	}
//	else
//	if (2 == BufferNo)							//write enable to buffer 2
//	{
//		DF_SPI_RW(Buf2Write);					//buffer 2 write op-code
//		DF_SPI_RW(0x00);						//don't cares
//		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
//		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
//	}
//}
//


/*****************************************************************************
*
*	Function name : Buffer_Write_Byte
*
*	Returns :		None
*
*	Parameters :	IntPageAdr	->	Internal page address to write byte to
*					BufferAdr	->	Decides usage of either buffer 1 or 2
*					Data		->	Data byte to be written
*
*	Purpose :		Writes one byte to one of the dataflash
*					internal SRAM buffers
*
******************************************************************************/
void Buffer_Write_Byte (unsigned int IntPageAdr, unsigned char Data)
{
	//DF_CS_inactive;								//make sure to toggle CS signal in order
//	DF_CS_active;								//to reset dataflash command decoder
	
		DF_SPI_RW(Buf1Write);					//buffer 1 write op-code
		DF_SPI_RW(0x00);						//don't cares
		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
		DF_SPI_RW(Data);						//write data byte

}



/*****************************************************************************
*
*	Function name : Buffer_Write_Str
*
*	Returns :		None
*
*	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
*					IntPageAdr	->	Internal page address
*					No_of_bytes	->	Number of bytes to be written
*					*BufferPtr	->	address of buffer to be used for copy of bytes
*									from AVR buffer to dataflash buffer 1 (or 2)
*
*	Purpose :		Copies one or more bytes to one of the dataflash
*					internal SRAM buffers from AVR SRAM buffer
*					pointed to by *BufferPtr
*
******************************************************************************/
//void Buffer_Write_Str (unsigned char BufferNo, unsigned int IntPageAdr, unsigned int No_of_bytes, unsigned char *BufferPtr)
//{
//	unsigned int i;
//
//	DF_CS_inactive;								//make sure to toggle CS signal in order
//	DF_CS_active;								//to reset dataflash command decoder
//
//	if (1 == BufferNo)							//write byte(s) to buffer 1
//	{
//		DF_SPI_RW(Buf1Write);					//buffer 1 write op-code
//		DF_SPI_RW(0x00);						//don't cares
//		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
//		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
//		for( i=0; i<No_of_bytes; i++)
//		{
//			DF_SPI_RW(*(BufferPtr));			//write byte pointed at by *BufferPtr to dataflash buffer 1 location
//			BufferPtr++;						//point to next element in AVR buffer
//		}
//	}
//	else
//	if (2 == BufferNo)							//write byte(s) to buffer 2
//	{
//		DF_SPI_RW(Buf2Write);					//buffer 2 write op-code
//		DF_SPI_RW(0x00);						//don't cares
//		DF_SPI_RW((unsigned char)(IntPageAdr>>8));//upper part of internal buffer address
//		DF_SPI_RW((unsigned char)(IntPageAdr));	//lower part of internal buffer address
//		for( i=0; i<No_of_bytes; i++)
//		{
//			DF_SPI_RW(*(BufferPtr));			//write byte pointed at by *BufferPtr to dataflash buffer 2 location
//			BufferPtr++;						//point to next element in AVR buffer
//		}
//	}
//}
//NB : Monitorer busy-flag i status-reg.
//NB : Sjekk at (IntAdr + No_of_bytes) < buffersize, hvis ikke blir det bare ball..



/*****************************************************************************
*
*	Function name : Buffer_To_Page
*
*	Returns :		None
*
*	Parameters :	BufferAdr	->	Decides usage of either buffer 1 or 2
*					PageAdr		->	Address of flash page to be programmed
*
*	Purpose :		Transfers a page from dataflash SRAM buffer to flash
*
******************************************************************************/
void Buffer_To_Page (unsigned int PageAdr)
{


//	DF_CS_inactive;												//make sure to toggle CS signal in order
//	DF_CS_active;												//to reset dataflash command decoder

		DF_SPI_RW(Buf1ToFlashWE);								//buffer 1 to flash with erase op-code
		DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));	//upper part of page address
		DF_SPI_RW((unsigned char)(PageAdr << (PageBits - 8)));	//lower part of page address
		DF_SPI_RW(0x00);										//don't cares
	
//	DF_CS_inactive;												//initiate flash page programming
//	DF_CS_active;

while(!(Read_DF_status() & 0x80));							//monitor the status register, wait until busy-flag is high

}


UI8 DF_ContReadByte(void)
{
return DF_SPI_RW(0x00);
}

/*****************************************************************************
*
*	Function name : Cont_Flash_Read_Enable
*
*	Returns :		None
*
*	Parameters :	PageAdr		->	Address of flash page where cont.read starts from
*					IntPageAdr	->	Internal page address where cont.read starts from
*
*	Purpose :		Initiates a continuous read from a location in the DataFlash
*
******************************************************************************/
//void Cont_Flash_Read_Enable (unsigned int PageAdr, unsigned int IntPageAdr)
//{
//	DF_CS_inactive;																//make sure to toggle CS signal in order
//	DF_CS_active;																//to reset dataflash command decoder
//
//	DF_SPI_RW(ContArrayRead);													//Continuous Array Read op-code
//	DF_SPI_RW((unsigned char)(PageAdr >> (16 - PageBits)));						//upper part of page address
//	DF_SPI_RW((unsigned char)((PageAdr << (PageBits - 8))+ (IntPageAdr>>8)));	//lower part of page address and MSB of int.page adr.
//	DF_SPI_RW((unsigned char)(IntPageAdr));										//LSB byte of internal page address
//	DF_SPI_RW(0x00);															//perform 4 dummy writes
//	DF_SPI_RW(0x00);															//in order to intiate DataFlash
//	DF_SPI_RW(0x00);															//address pointers
//	DF_SPI_RW(0x00);
//}
//
//
//void Cont_Flash_Read_End(void)
//{
//	DF_CS_inactive;																//make sure to toggle CS signal in order
//	DF_CS_active;																//to reset dataflash command decoder
//}

// *****************************[ End Of DATAFLASH.C ]*************************

/******************************************************************************
															Added by DCC Jackson
******************************************************************************/

/******************************************************************************
FlashWriteBuffer

Writes <size> bytes from buffer <buffer> to flash starting at absolute 
address <startaddress>

******************************************************************************/
void FlashWriteBuffer(unsigned char *buffer, unsigned int size, unsigned int startaddress)
{
		unsigned int page = 0xFFFF; 
		unsigned int pageindex;
		
		unsigned int newpage;
		
		unsigned int i;
		
		for(i = 0; i<size; i++)
		{
				FlashPageIndexPageCalculate((startaddress+i), (&newpage), (&pageindex)); //calculate page and index for this abs addr
				
				if(newpage != page) //Old buffered page must be written back to flash
				{
						
						if(i) 
						{
								Buffer_To_Page( page); //I'm finished with this page, so write buffer back to flash
						}
						
						page = newpage;												
						Page_To_Buffer(page);//get new page from flash and put into buffer

					
				}
				
				Buffer_Write_Byte( pageindex, buffer[i]); //write the data byte into the buffer
				
		}
		
		Buffer_To_Page( page); //make sure to write buffer back to flash before returning
		
	
}


/******************************************************************************
FlashReadToBuffer

Reads <size> bytes from flash and saves to buffer <buffer> starting at absolute 
address <startaddress>

******************************************************************************/
void FlashReadToBuffer(unsigned char *buffer, unsigned int size, unsigned int startaddress)
{
		unsigned int page = 0xFFFF; 
		unsigned int pageindex;
		
		unsigned int newpage;
		
		unsigned char temp;
		
		unsigned int i;
		
		for(i = 0; i<size; i++)
		{
				FlashPageIndexPageCalculate((startaddress+i), (&newpage), (&pageindex)); //calculate page and index for this abs addr
				
				if(newpage != page) //Old buffered page must be written back to flash
				{	
						page = newpage;												
						Page_To_Buffer(page);//get new page from flash and put into buffer
					
				}
				
				temp = Buffer_Read_Byte ( pageindex); //read data from flash buffer
				
				buffer[i] = temp;
				
		}
		

		
	
}

/******************************************************************************
FlashPageIndexPageCalculate

Calculates the page number and internal page address for absolute byte address
<absaddr>.

Result is stored in parameters <page> and <pagebyteindex>

******************************************************************************/
void FlashPageIndexPageCalculate(unsigned long absaddr, unsigned int *page, unsigned int *pageindex)
{
		unsigned int temp;
		unsigned long tempabsaddr;		
		
		tempabsaddr = absaddr/BYTES_PER_PAGE;
		temp = tempabsaddr;
		*page = temp;
		
		tempabsaddr = absaddr%BYTES_PER_PAGE;
		temp = tempabsaddr;
		*pageindex = temp;	
	
}

