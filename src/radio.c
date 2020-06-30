#include "preprocessor.h"

//This LoRa lib is based of sandeepmistry LoRa lib
//Found at: https://github.com/sandeepmistry/arduino-LoRa

UI16 _packetIndex=0;

/**
* @brief 	This function checks for any incomming LoRA packets (Note: max size is 256 bytes)
	* @param 	None
	*	@retval None
	*/
void TestLoRaReceiver(void)
{
	UI8 ReceivedArr[256];
	UI16 i=0;
	
	for(i = 0; i < sizeof(ReceivedArr); i++) ReceivedArr[i] = 0x00; //Clear array
	
	int packetSize = parsePacket();
  if (packetSize)                         //Packet is available
	{
		LED3_SET();
    
		// received a packet
    PrintfP("\nReceived Packet \n");
		PrintOLED( 0, 2, "Got Packet:");
		delaymms(10);
		
		i=0;
    // read packet
    while (LoRaAvailable())              //While there is data read the packet
		{  
			ReceivedArr[i] = LoraRead(); 
			PrintfP("0x%x, ", ReceivedArr[i]); 
			delaymms(10); 
			if(i > 256) i=0;                  //Packet bigger than 256 so wrap
			else i++;
		} 
		
		PrintOLED(0, 4, ReceivedArr);       //Print to OLED (remember we can display all 256 bytes)
		LED3_CLR();
  }
}

/**
	* @brief 	This function sends out a LoRA packet
	* @param 	str- string to send
	*	@retval None
	*/
void TestLoRaTransmitter(UI8 *str)
{
	LED0_SET();
	OLED_ClearLine(4);
	PrintfP("\nSending packet: ");
	delaymms(10);
  PrintOLED(0, 2, "Send Packet!");
	
	// send packet
  beginPacket();
  LoRaPrint(str);
  endPacket();
	delaymms(500);
	OLED_ClearLine(2);
	
	LED0_CLR();
}

/**
	* @brief 	This function sends out a packet
	* @param 	*buffer - buffer to transmit
	*	@retval None
	*/
void LoRaPrint(UI8 *buffer)
{
	UI8 size;
	UI16 currentLength;
	
	size = strlen((const char *)buffer);

	currentLength = SPI_Read(REG_PAYLOAD_LENGTH);

  // check size
  if ((currentLength + size) > MAX_PKT_LENGTH) {
    size = MAX_PKT_LENGTH - currentLength;
  }

  // write data
  for (size_t i = 0; i < size; i++) {
    SPI_Write(REG_FIFO, buffer[i]);
  }
 
  // update length
  SPI_Write(REG_PAYLOAD_LENGTH, currentLength + size);
}

UI16 endPacket(void)
{
	SPI_Write(REGOPMODE, MODE_LONG_RANGE_MODE | TX_gc);

	// wait for TX done
	while ((SPI_Read(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0) { }

	SPI_Write(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
	
	return 1;
}

/**
	* @brief 	This function write via SPI to the RA-01
	* @param 	address - address to write to
	*					data - data to write
	*	@retval None
	*/
void SPI_Write(UI8 address, UI8 data)
{
		LORA_CS_CLR();
		BB_WriteData(( WRITE<<7 ) | address);
		BB_WriteData(data);
		LORA_CS_SET();
}

/**
	* @brief 	This function reads data from RA-01 via SPI
	* @param 	address - address to read from
	*	@retval data1 - data returned
	*/
UI8 SPI_Read(UI8 address)
{
		uint32_t data1=0;
	  LORA_CS_CLR();
		BB_WriteData(address);
		data1 = BB_ReadData();
		LORA_CS_SET();
		return data1;
}

/**
	* @brief 	This function sets the operating mode of the RA-01
	* @param 	mode - the mode to set
	*	@retval None
	*/
void SetOpMode(MODE_t mode)
{
    SPI_Write( 0x01, (0x20|mode));//( SPI_Read( 0x01 ) & 0xF8 ) | mode );
}

/**
	* @brief 	This function sets up the LoRA frequency (433MHz)
	* @param 	freq - frequency to set
	*	@retval None
	*/
void Set_Frequency(UI32 freq)
{
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
  idle();
	explicitHeaderMode();

  // reset FIFO address and paload length
  SPI_Write(REG_FIFO_ADDR_PTR, 0);
  SPI_Write(REG_PAYLOAD_LENGTH, 0);

  return 1;
}

/**
	* @brief 	This function sets up the RA-01
	* @param 	None
	*	@retval 1 - successful, 0 - not successful
	*/
UI8 LoRaSetup(void)
{
	LORA_RST_CLR();                       //reset LoRa radio
	for(int i =0; i < 1000; i++) ;
	LORA_RST_SET();
	
	// check version
  uint8_t version = SPI_Read(REG_VERSION);
  if (version != 0x12) return 0;
	
	sleep(); //Set to sleep mode
	Set_Frequency(434E6);//Set_Frequency(915E6);
	
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
