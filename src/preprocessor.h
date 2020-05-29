#include <stm32f0xx.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef unsigned char   UI8;
typedef signed char     SI8;
typedef unsigned short  UI16;
typedef signed short    SI16;
typedef unsigned int    UI32;
typedef signed int      SI32;

#define INPUT         	0x00  //MODE
#define OUTPUT          0x01 
#define ALTFUNCT				0x02
#define ANALOG					0x03
#define PushPull 				0x00	//OTYPE
#define OutDrain				0x01  
#define LowSpeed				0x00	//SPEED
#define MedSpeed				0x01
#define HiSpeed					0x03  
#define NoPull					0x00	//PULL
#define PullUp					0x01
#define PullDwn					0x02
#define NONE						0x00
#define RESET						0x01
#define SET							0x02

#define DEBUG 					0x01

#include "hardware.h"
#include "radio.h"
#include "EEPROM.h"
#include "oled.h"
#include "com_io.h"
#include "main.h"
