#ifndef __OLED_H
#ifdef _is_definer
#undef _is_definer
#define EXT
#else
#define EXT  extern
#endif

void OLED_init(void);
void GLCD_Setting(void);
extern void ssd1306_WriteData(UI8 byte);
extern void ssd1306_WriteCommand(UI8 byte);
extern void setPageAddress(UI8 x, UI8 y);
extern void setColAddress(UI8 x, UI8 y);
extern void OLED_Clr(UI8 col);
extern void ssd1306_WriteData1(UI8 arr[], UI8 bytesToWrite);
extern void TransferBuffer(UI8 charArray[], UI8 size, UI8 x, UI8 y, UI8 underline);

// commands SSD1306 controller
#define LCD_SET_COL_HI			0x10
#define LCD_SET_COL_LO			0x00
#define LCD_SET_LINE				0x40
#define LCD_SET_CONTRAST		0x81
#define LCD_SET_BRIGHTNESS  0x82
#define LCD_SET_LOOKUP      0x91
#define LCD_SET_SEG_REMAP0  0xA0
#define LCD_SET_SEG_REMAP1	0xA1
#define LCD_EON_OFF					0xA4
#define LCD_EON_ON					0xA5
#define LCD_DISP_NOR				0xA6
#define LCD_DISP_REV				0xA7
#define LCD_MULTIPLEX       0xA8
#define LCD_MASTER_CONFIG   0xAD
#define LCD_CHARGE_PUMP    	0x8D
#define LCD_PUMP_OFF    		0x10
#define LCD_PUMP_ON     		0x14
#define LCD_DISP_OFF 				0xAE
#define LCD_DISP_ON					0xAF
#define LCD_SET_PAGE				0xB0
#define LCD_SET_SCAN_FLIP		0xC0
#define LCD_SET_SCAN_NOR		0xC8
#define LCD_SET_OFFSET			0xD3
#define LCD_SET_RATIO_OSC		0xD5
#define LCD_SET_AREA_COLOR  0xD8
#define LCD_SET_CHARGE  		0xD9
#define LCD_SET_PADS    		0xDA
#define LCD_SET_VCOM    		0xDB
#define LCD_NOP     				0xE3
#define LCD_SCROLL_RIGHT		0x26
#define LCD_SCROLL_LEFT			0x27
#define LCD_SCROLL_VR	   		0x29
#define LCD_SCROLL_VL				0x2A
#define LCD_SCROLL_OFF			0x2E
#define LCD_SCROLL_ON   		0x2F
#define LCD_SCROLL_ON   		0x2F
#define LCD_VERT_SCROLL_A  	0xA3
#define LCD_MEM_ADDRESSING 	0x20
#define LCD_SET_COL_ADDR		0x21
#define LCD_SET_PAGE_ADDR		0x22
#define LCD_SET_COM					0xC9


extern const UI8 BigFont[];
extern UI8 u8CursorX, u8CursorY;
extern void OLEDInstructionWrite (UI8 u8Instruction);

#define lcd_goto(x,y) { u8CursorX=(x); u8CursorY=(y); }

#undef EXT
#define __OLED_H
#endif

