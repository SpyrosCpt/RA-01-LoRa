#include "preprocessor.h"

UI16 _packetIndex=0;

void TestLoRaReceiver(void)
{
	int packetSize = parsePacket();
  if (packetSize) 
	{
		LED3_TOG();
    // received a packet
    PrintfP("\nReceived packet \n");
		delayms(10);
    // read packet
    while (LoRaAvailable()) {PutChar(LoraRead()); }
  }
}

void TestLoRaTransmitter(void)
{
	PrintfP("\nSending packet: ");
	delayms(10);
  PrintOLED(0, 0, 0, "Sendd Pkt!");
	// send packet
  beginPacket();
  LoRaPrint("hello ");
  LoRaPrint("world");
  endPacket();
}
void LoRaPrint(UI8 *buffer)
{
	UI8 size;
	UI16 currentLength;
	
	size = strlen(buffer);
	PrintfP("\nString size = %d", size);
	delayms(10);
	currentLength = SPI_Read(REG_PAYLOAD_LENGTH);
	PrintfP("\nCurrent length = %d", currentLength);	
	delayms(10);
  // check size
  if ((currentLength + size) > MAX_PKT_LENGTH) {
    size = MAX_PKT_LENGTH - currentLength;
  }

  // write data
  for (size_t i = 0; i < size; i++) {
    SPI_Write(REG_FIFO, buffer[i]);
  }
  PrintfP("\nNew length = %d", currentLength + size);
	delayms(10);
  // update length
  SPI_Write(REG_PAYLOAD_LENGTH, currentLength + size);
}

UI16 endPacket(void)
{
	PrintfP("\nTX Mode");
	delayms(10);
	// put in TX mode
	SPI_Write(REGOPMODE, MODE_LONG_RANGE_MODE | TX_gc);

	// wait for TX done
	while ((SPI_Read(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0) { }
  PrintfP("\nTX done");
	delayms(10);
	// clear IRQ's
	PrintfP("\nClear IRQ\n");
	delayms(10);
	SPI_Write(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
}

void SPI_Write(UI8 address, UI8 data)
{
		LORA_CS_CLR();
		BB_WriteData(( WRITE<<7 ) | address);
		BB_WriteData(data);
		LORA_CS_SET();
}

UI8 SPI_Read(UI8 address)
{
		uint32_t data1=0;
	  LORA_CS_CLR();
		BB_WriteData(address);
		data1 = BB_ReadData();
		LORA_CS_SET();
		return data1;
}

void SetOpMode(MODE_t mode)
{
    SPI_Write( 0x01, (0x20|mode));//( SPI_Read( 0x01 ) & 0xF8 ) | mode );
}

void Set_Frequency(UI32 freq)
{
    /*freq = ( UI32 )( ( double )freq / ( double ) FREQ_STEP );

    SPI_Write( 0x06, ( UI8 )( ( freq >> 16 ) & 0xFF ) );
    SPI_Write( 0x07, ( UI8 )( ( freq >> 8 ) & 0xFF ) );
    SPI_Write( 0x08, ( UI8 )( freq & 0xFF ) );*/

  uint64_t frf = ((uint64_t)freq << 19) / 32000000;

  SPI_Write(REG_FRF_MSB, (uint8_t)(frf >> 16));
  SPI_Write(REG_FRF_MID, (uint8_t)(frf >> 8));
  SPI_Write(REG_FRF_LSB, (uint8_t)(frf >> 0));
}


void setOCP(UI8 mA)
{
  uint8_t ocpTrim = 27;

  if (mA <= 120) ocpTrim = (mA - 45) / 5; 
	else if (mA <=240) ocpTrim = (mA + 30) / 10;

  SPI_Write(REG_OCP, 0x20 | (0x1F & ocpTrim));
}

void setTxPower(UI16 level)
{
	//Default value PA_HF/LF or +17dBm
	SPI_Write(REG_PA_DAC, 0x84);
	setOCP(100);
	SPI_Write(REG_PA_CONFIG, PA_BOOST | (level - 2));
}

void idle(void)
{
  SPI_Write(REGOPMODE, MODE_LONG_RANGE_MODE | STANDBY_gc);
}

void sleep(void)
{
  SPI_Write(REGOPMODE, MODE_LONG_RANGE_MODE | SLEEP_gc);
}
void explicitHeaderMode(void)
{
  SPI_Write(REG_MODEM_CONFIG_1, SPI_Read(REG_MODEM_CONFIG_1) & 0xFE);
}

UI8 beginPacket(void)
{
	// put in standby mode
	PrintfP("\nPut in idle mode");
	delayms(10);
  idle();
	PrintfP("\nExplicit header mode");
	delayms(10);
	explicitHeaderMode();

	PrintfP("\nReset FIFO's");
	delayms(10);
  // reset FIFO address and paload length
  SPI_Write(REG_FIFO_ADDR_PTR, 0);
  SPI_Write(REG_PAYLOAD_LENGTH, 0);

  return 1;
}

UI8 LoRaSetup(void)
{
	LORA_RST_CLR();                       //reset LoRa radio
	for(int i =0; i < 1000; i++) ;
	LORA_RST_SET();
	
	// check version
  uint8_t version = SPI_Read(REG_VERSION);
  if (version != 0x12) return 0;
	
	sleep(); //Set to sleep mode
	Set_Frequency(915E6);
	
	  // set base addresses
  SPI_Write(REG_FIFO_TX_BASE_ADDR, 0);
  SPI_Write(REG_FIFO_RX_BASE_ADDR, 0);
	
	// set LNA boost
  SPI_Write(REG_LNA, SPI_Read(REG_LNA) | 0x03);

  // set auto AGC
  SPI_Write(REG_MODEM_CONFIG_3, 0x04);

  // set output power to 17 dBm
  setTxPower(17);//17);

  // put in standby mode
  idle();
	
	PrintOLED(0, 0, 0, "TEMP: %d", SPI_Read(REG_MODEM_CONFIG_3));
	return 1;
}

int LoRaAvailable(void)
{
  return (SPI_Read(REG_RX_NB_BYTES) - _packetIndex);
}

int LoraRead(void)
{
  if (!LoRaAvailable()) {
    return -1;
  }

  _packetIndex++;

  return SPI_Read(REG_FIFO);
}

int parsePacket(void)
{
  int packetLength = 0;
  int irqFlags = SPI_Read(REG_IRQ_FLAGS);


  explicitHeaderMode();

  // clear IRQ's
  SPI_Write(REG_IRQ_FLAGS, irqFlags);

  if ((irqFlags & IRQ_RX_DONE_MASK) && (irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0) 
	{
		PrintOLED(0, 4, 0, "Somethng!");
    // received a packet
    _packetIndex = 0;

    // read packet length

    packetLength = SPI_Read(REG_RX_NB_BYTES);

    // set FIFO address to current RX address
    SPI_Write(REG_FIFO_ADDR_PTR, SPI_Read(REG_FIFO_RX_CURRENT_ADDR));

    // put in standby mode
    idle();
  } 
	else if (SPI_Read(REGOPMODE) != (MODE_LONG_RANGE_MODE | RX_SINGLE_gc)) 
	{
    // not currently in RX mode

    // reset FIFO address
    SPI_Write(REG_FIFO_ADDR_PTR, 0);

    // put in single RX mode
    SPI_Write(REGOPMODE, MODE_LONG_RANGE_MODE | RX_SINGLE_gc);
  }

  return packetLength;
}