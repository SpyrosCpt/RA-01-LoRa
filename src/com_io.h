#define USB_RX_BUFFSIZE 128
#define USB_TX_BUFFSIZE 128

extern UI8         USB_RX_Buffer[USB_RX_BUFFSIZE];
extern UI8         USB_TX_Buffer[USB_TX_BUFFSIZE];
                                                    
extern UI16        USB_RX_end, USB_TX_end, USB_RX_start, USB_TX_start;

extern UI8 is_Char(void);
extern UI8 comms_getch(void);
extern void ClrTX(void);
extern void ClrRX(void);
extern UI8 PrintfP(const UI8 *input, ...);
extern void int_asci_print(UI32 input);
extern UI8 PutChar(UI8 data);
extern void int_hexasci_print(UI8 input);
extern void int_asci_print(UI32 input);
extern UI8 PrintOLED(UI8 underline, UI8 x, UI8 y, const UI8 *input, ...);
