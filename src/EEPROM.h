
extern void EE_EWEN(void);
extern UI8 EE_ERASE(UI8 address);
extern UI8 EE_WRITE(UI8 address, UI8 data);
extern UI8 EE_READ(UI8 address);
extern void EE_EWDS(void);
extern void SPI_Send(UI8 data);
extern UI8 SPI_Receive(UI8 data);
extern void BB_WriteData(UI8 data);
extern UI8 BB_ReadData(void);
extern UI8 EE_ERASE_ALL(void);
