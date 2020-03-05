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

extern void Clock_Setup( void );
extern void UART_Setup(UI32 baud);
extern void Set_Mode( GPIO_TypeDef * PORT, uint8_t pin, uint8_t direction );
extern void Set_Type( GPIO_TypeDef * PORT, uint8_t pin, uint8_t type );
extern void Set_Speed( GPIO_TypeDef * PORT, uint8_t pin, uint8_t speed );
extern void Set_Pull( GPIO_TypeDef * PORT, uint8_t pin, uint8_t pull );
extern void Set_Pin( GPIO_TypeDef * PORT, uint8_t pin, uint8_t direction, uint8_t type, uint8_t speed, uint8_t pull, uint8_t state );
extern void TIM17_Setup( void );
extern void TIM3_Setup( void );
extern void delayms(uint32_t delay);
extern void delayus(uint32_t delay);
extern void Ports_Setup( void );
extern void SPI_Setup(void);
extern void SysTick_Init(void);
extern void Setup( void );

#define DF_CS_SET() ( GPIOC->BSRR |= ( 1<<4 ) )
#define DF_CS_CLR() ( GPIOC->BRR |= ( 1<<4 ) )
#define CLK_SET() ( GPIOA->BSRR |= ( 1<<5 ) )
#define CLK_CLR() ( GPIOA->BRR |= ( 1<<5 ) )
#define MOSI_SET() ( GPIOA->BSRR |= ( 1<<7 ) )
#define MOSI_CLR() ( GPIOA->BRR |= ( 1<<7 ) )

#define OLED_CS_SET() ( GPIOB->BSRR |= ( 1<<1 ) )
#define OLED_CS_CLR() ( GPIOB->BRR |= ( 1<<1 ) )
#define OLED_DC_SET() ( GPIOC->BSRR |= ( 1<<5 ) )
#define OLED_DC_CLR() ( GPIOC->BRR |= ( 1<<5 ) )
#define OLED_RST_SET() ( GPIOB->BSRR |= ( 1<<0 ) )
#define OLED_RST_CLR() ( GPIOB->BRR |= ( 1<<0 ) )

#define LORA_CS_SET() ( GPIOB->BSRR |= ( 1<<2 ) )
#define LORA_CS_CLR() ( GPIOB->BRR |= ( 1<<2 ) )

#define LORA_RST_SET() ( GPIOB->BSRR |= ( 1<<10 ) )
#define LORA_RST_CLR() ( GPIOB->BRR |= ( 1<<10 ) )

#define ASK_SET() ( GPIOB->BSRR |= ( 1<<11 ) )
#define ASK_CLR() ( GPIOB->BRR |= ( 1<<11 ) )

#define MISO_READ() (GPIOA->IDR & (1<<6))

#define LED0_SET() ( GPIOB->BSRR |= ( 1<<5 ) )  //note this is the opposite way you "Set" to put the pin low and vice versa
#define LED0_CLR() ( GPIOB->BRR |= ( 1<<5 ) )
#define LED0_TOG() ( GPIOB->ODR ^= 1<<5 )
#define LED1_SET() ( GPIOB->BSRR |= ( 1<<6 ) )  //note this is the opposite way you "Set" to put the pin low and vice versa
#define LED1_CLR() ( GPIOB->BRR |= ( 1<<6 ) )
#define LED1_TOG() ( GPIOB->ODR ^= 1<<6 )
#define LED2_SET() ( GPIOB->BSRR |= ( 1<<7 ) )  //note this is the opposite way you "Set" to put the pin low and vice versa
#define LED2_CLR() ( GPIOB->BRR |= ( 1<<7 ) )
#define LED3_SET() ( GPIOB->BSRR |= ( 1<<8 ) )  //note this is the opposite way you "Set" to put the pin low and vice versa
#define LED3_CLR() ( GPIOB->BRR |= ( 1<<8 ) )

#define LED2_TOG() ( GPIOB->ODR ^= ( 1<<7 ) )
#define LED3_TOG() ( GPIOB->ODR ^= ( 1<<8 ) )

#define PB0_READ() (GPIOA->IDR & (1<<0))
#define PB1_READ() (GPIOA->IDR & (1<<1))
#define PB2_READ() (GPIOA->IDR & (1<<2))
#define PB3_READ() (GPIOA->IDR & (1<<3))

#define DIP0_READ() (GPIOC->IDR & (1<<13))
#define DIP1_READ() (GPIOC->IDR & (1<<14))
#define DIP2_READ() (GPIOC->IDR & (1<<15))
#define DIP3_READ() (GPIOF->IDR & (1<<0))
