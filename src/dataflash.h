/******** Example Code : Accessing Atmel AT45Dxxx dataflash on STK500 *******

Device      : 	AT90S8515

File name   : 	dataflash.h

Description : 	Defines and prototypes for AT45Dxxx

Last change:    16 Aug 2001   AS

****************************************************************************/

#ifndef __DATAFLASH_INCLUDED
#define __DATAFLASH_INCLUDED
#endif



//Dataflash opcodes
#define FlashPageRead							0xD2	// Main memory page read
#define FlashToBuf1Transfer 			0x53	// Main memory page to buffer 1 transfer
#define Buf1Read									0xD4	// Buffer 1 read
#define FlashToBuf2Transfer 			0x55	// Main memory page to buffer 2 transfer
#define Buf2Read									0xD6	// Buffer 2 read
#define StatusReg									0xD7	// Status register
#define AutoPageReWrBuf1					0x58	// Auto page rewrite through buffer 1
#define AutoPageReWrBuf2					0x59	// Auto page rewrite through buffer 2
#define FlashToBuf1Compare  			0x60	// Main memory page to buffer 1 compare
#define FlashToBuf2Compare	   		0x61	// Main memory page to buffer 2 compare
#define ContArrayRead							0xE8	// Continuous Array Read (Note : Only A/B-parts supported)
#define FlashProgBuf1							0x82	// Main memory page program through buffer 1
#define Buf1ToFlashWE   					0x83	// Buffer 1 to main memory page program with built-in erase
#define Buf1Write									0x84	// Buffer 1 write
#define FlashProgBuf2							0x85	// Main memory page program through buffer 2
#define Buf2ToFlashWE   					0x86	// Buffer 2 to main memory page program with built-in erase
#define Buf2Write									0x87	// Buffer 2 write
#define Buf1ToFlash     					0x88	// Buffer 1 to main memory page program without built-in erase
#define Buf2ToFlash		       			0x89	// Buffer 2 to main memory page program without built-in erase
//Dataflash macro definitions
#define DF_CS_active	DF_CS_CLR()

#define DF_CS_inactive	DF_CS_SET()

#define BYTES_PER_PAGE 264
#define TOTAL_PAGES 2048



//Function definitions
void DF_SPI_init (void);
UI8 DF_ContReadByte(void);
void Cont_Flash_Read_End(void);
UI16 DF_SPI_RW (unsigned char output);
unsigned char Read_DF_status (void);
void Page_To_Buffer (unsigned int PageAdr);
UI16 Buffer_Read_Byte (unsigned int IntPageAdr);
void Buffer_Read_Str (unsigned int IntPageAdr, unsigned int No_of_bytes, unsigned char *BufferPtr);
void Buffer_Write_Enable (unsigned int IntPageAdr);
void Buffer_Write_Byte (unsigned int IntPageAdr, unsigned char Data);
void Buffer_Write_Str (unsigned int IntPageAdr, unsigned int No_of_bytes, unsigned char *BufferPtr);
void Buffer_To_Page (unsigned int PageAdr);
void Cont_Flash_Read_Enable (unsigned int PageAdr, unsigned int IntPageAdr);


/**** Added by DCC Jackson ****/

void FlashWriteBuffer(unsigned char *buffer, unsigned int size, unsigned int startaddress);
void FlashReadToBuffer(unsigned char *buffer, unsigned int size, unsigned int startaddress);
void FlashPageIndexPageCalculate(unsigned long absaddr, unsigned int *page, unsigned int *pageindex);


//added by carel
void Buffer_Read_Str_Pos (unsigned int IntPageAdr, unsigned int IntBuffAdr, unsigned int No_of_bytes, unsigned char *BufferPtr);
