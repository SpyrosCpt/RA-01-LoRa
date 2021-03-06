# RA-01-LoRa Dev Board

## Features
- Max 15V DC input
- 5V DC USB input
- CP2102 USB interface
- STM32F030RCT6
- 0.96" 128 x 32 OLED
- RA-01 LoRA Module
- LM35 Temperature Sensor
- AT93C46 EEPROM
- 4 Way DIP Switch
- 4 Push Buttons
- STM32 programming by SWD

## Firmware Example Provided
- Press Push Button 1 to Send LoRA Packet, you can change the data to send by editing TestLoraTransmit("packet to send")
- When not transmitting LoRA radio is in Receiver mode and will display any messages receievd
- Temperature Monitoring of LM35 will be displayed on OLED

## Hardware Version 1.1
![alt text](https://github.com/SpyrosCpt/RA-01-LoRa/blob/master/Hardware/3d%20model.png)

## Firmware Change Log
- Changed frequency down from 915Mhz to 433MHz
- Oled working
- LoRa working
- EEPROM working
- Temp Sensor

## Harware Change Log
- Removed USB protection IC
- Swapped TX and RX lines
- Grounded pin 16 + 17 on OLED connector
- Moved LM34 Vcc from 3.3V to 5V
