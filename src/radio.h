#define FREQ_STEP   	61.03515625

#define REG_FIFO         0x00
#define REGOPMODE      	 0x01
#define REGFRFMSB      	 0x06
#define REGFRFMID      	 0x07
#define REGFRFLSB      	 0x08
#define REGPACONFIG    	 0x09
#define REGPARAMP      	 0x0A
#define REGOCP         	 0x0B
#define REGLNA         	 0x0C
#define REGOOKPEAK     	 0x14
#define REGPREAMBLELSB   0x26
#define REGSYNCCONFIG    0x27
#define REGSYNCVALUE1    0x28
#define REGSYNCVALUE2    0x29
#define REGSYNCVALUE3  	 0x2A
#define REGSYNCVALUE4    0x2B
#define REGSYNCVALUE5  	 0x2C
#define REGSYNCVALUE6  	 0x2D
#define REGSYNCVALUE7  	 0x2E
#define REGSYNCVALUE8  	 0x2F
#define REGPACKETCONFIG1 0x30
#define REGPACKETCONFIG2 0x31
#define REGDIOMAPPING1 	 0x40
#define REG_FIFO_TX_BASE_ADDR    0x0E
#define REG_FIFO_RX_BASE_ADDR    0x0F
#define REG_LNA                  0x0C
#define REG_MODEM_CONFIG_3       0x26
#define MODE_LONG_RANGE_MODE     0x80
#define REG_PA_DAC               0x4d
#define REG_PA_CONFIG            0x09
#define REG_OCP                  0x0b
#define PA_BOOST                 0x80
#define REG_VERSION              0x42
#define REG_IRQ_FLAGS            0x12
// IRQ masks
#define IRQ_TX_DONE_MASK           0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK           0x40
#define REG_FIFO_ADDR_PTR        0x0D
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG_1       0x1D
#define MAX_PKT_LENGTH           255
#define REG_RX_NB_BYTES          0x13
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08

typedef enum MODE
{
    SLEEP_gc     = (0x00<<0),
    STANDBY_gc   = (0x01<<0),
    FSTx_gc      = (0x02<<0),
    TX_gc        = (0x03<<0),
    FSRx_gc      = (0x04<<0),
    RX_gc        = (0x05<<0),
	  RX_SINGLE_gc = (0x06<<0)
}MODE_t;

typedef unsigned char   UI8;
typedef signed char     SI8;
typedef unsigned short  UI16;
typedef signed short    SI16;
typedef unsigned int    UI32;
typedef signed int      SI32;

#define WRITE 0x01
#define READ 0x00

extern void SPI_Write(UI8 address, UI8 data);
extern UI8 SPI_Read(UI8 address);
extern void SetOpMode(MODE_t mode);
extern UI8 LoRaSetup(void);
extern void LoRaPrint(UI8 *buffer);
extern UI16 endPacket(void);
extern UI8 beginPacket(void);
extern void TestLoRaTransmitter(void);
extern void LoRaPrint(UI8 *buffer);
extern int parsePacket(void);
extern int LoraRead(void);
extern int LoRaAvailable(void);

extern UI16 _packetIndex;
