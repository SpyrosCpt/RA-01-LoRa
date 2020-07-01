#include "preprocessor.h"

/**
	* @brief 	This function sets up a GPIO
	* @param 	*PORT - the port (GPIOA, GPIOB etc)
	*         pin - pin to use 0 - 15
	*					direction - INPUT or OUTPUT
	*					type - PushPull, OpenDrain
	*					speed - LoSpeed, MedSpeed, HiSpeed
	*					pull - PullUp, PullDwn, NoPull
	*					state - SET, RESET, NONE
	*	@retval None
	*/
void Set_Pin( GPIO_TypeDef * PORT, uint8_t pin, uint8_t direction, uint8_t type, uint8_t speed, uint8_t pull, uint8_t state )
{
	Set_Mode( PORT, pin, direction );
	Set_Type( PORT, pin, type );
	Set_Speed( PORT, pin, speed );
	Set_Pull( PORT, pin, pull );
	
	if( state == 1 ) PORT->BSRR |= ( 1 << pin );
	else if( state == 2 ) PORT->BRR |= ( 1 << pin );
}
void Set_Mode( GPIO_TypeDef * PORT, uint8_t pin, uint8_t direction ) 
{
	PORT->MODER |= ( direction<< ( pin * 2 ) );
}

void Set_Type( GPIO_TypeDef * PORT, uint8_t pin, uint8_t type )
{
	PORT->OTYPER |= ( type<<pin );
}

void Set_Speed(GPIO_TypeDef * PORT, uint8_t pin, uint8_t speed )
{
	PORT->OSPEEDR |= ( speed<<( pin * 2 ) );
}

void Set_Pull(GPIO_TypeDef * PORT, uint8_t pin, uint8_t pull )
{
	PORT->PUPDR |= ( pull<< ( pin * 2 ) );
}

/**
	* @brief 	This function sets up the GPIO
	* @param 	None
	*	@retval None
	*/
void Ports_Setup( void )
{
	Set_Pin(GPIOB,  5, OUTPUT, PushPull, HiSpeed, NoPull, SET);
	Set_Pin(GPIOB,  6, OUTPUT, PushPull, HiSpeed, NoPull, SET);
	Set_Pin(GPIOB,  7, OUTPUT, PushPull, HiSpeed, NoPull, SET);
	Set_Pin(GPIOB,  8, OUTPUT, PushPull, HiSpeed, NoPull, SET);

	Set_Pin(GPIOA,  0, INPUT, PushPull, HiSpeed, NoPull, NONE);
	Set_Pin(GPIOA,  1, INPUT, PushPull, HiSpeed, NoPull, NONE);
	Set_Pin(GPIOA,  2, INPUT, PushPull, HiSpeed, NoPull, NONE);
	Set_Pin(GPIOA,  3, INPUT, PushPull, HiSpeed, NoPull, NONE);
	
	Set_Pin(GPIOC,  13, INPUT, PushPull, HiSpeed, PullUp, NONE);
	Set_Pin(GPIOC,  14, INPUT, PushPull, HiSpeed, PullUp, NONE);
	Set_Pin(GPIOC,  15, INPUT, PushPull, HiSpeed, PullUp, NONE);
	Set_Pin(GPIOF,  0, INPUT, PushPull, HiSpeed, PullUp, NONE);
	
	Set_Pin(GPIOC,  4, OUTPUT, PushPull, HiSpeed, NoPull, RESET); //EEPROM_CS
	
	Set_Pin(GPIOA,  5, OUTPUT, PushPull, HiSpeed, NoPull, RESET); //CLK
	Set_Pin(GPIOA,  6, INPUT, PushPull, HiSpeed, NoPull, NONE);   //MISO
	Set_Pin(GPIOA,  7, OUTPUT, PushPull, HiSpeed, NoPull, RESET); //MOSI*/
	
	Set_Pin(GPIOB,  1, OUTPUT, PushPull, HiSpeed, NoPull, SET); //OLED_CS
	Set_Pin(GPIOC,  5, OUTPUT, PushPull, HiSpeed, NoPull, NONE); //OLED_DC
	Set_Pin(GPIOB,  0, OUTPUT, PushPull, HiSpeed, NoPull, SET); //OLED_RESET
	
	Set_Pin(GPIOB,  2, OUTPUT, PushPull, HiSpeed, NoPull, SET); //LORA_CS
	Set_Pin(GPIOB,  11, OUTPUT, PushPull, HiSpeed, NoPull, RESET); //LORA_CS
	Set_Pin(GPIOB,  10, OUTPUT, PushPull, HiSpeed, NoPull, SET); //LORA_RESET
}

/**
	* @brief 	This function sets up the Timer3 (1ms resolution)
	* @param 	None
	*	@retval None
	*/
void TIM3_Setup(void)//1ms resolution
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = (uint16_t) 24000 - 1;//23999;	        // Set prescaler to 24 000 (PSC + 1)
	TIM3->ARR = (uint16_t) 2 - 1;	          // Auto reload value 1000
	TIM3->DIER = TIM_DIER_UIE; // Enable update interrupt (timer level)
	TIM3->CR1 |= TIM_CR1_CEN;   // Enable timer
	NVIC_EnableIRQ( TIM3_IRQn ); // Enable interrupt from TIM3 (NVIC level)
}

/**
	* @brief 	This function sets up the Timer17 (10us resolution)
	* @param 	None
	*	@retval None
	*/
void TIM17_Setup(void)//10us resolution
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
	TIM17->PSC = (uint16_t) 8 - 1;//23999;	        // Set prescaler to 24 000 (PSC + 1)
	TIM17->ARR = (uint16_t) 2 - 1;//10-1;//10 - 1;	          // Auto reload value 1000
	TIM17->DIER = TIM_DIER_UIE; // Enable update interrupt (timer level)
	TIM17->CR1 &= ~TIM_CR1_CEN;   // Enable timer
	NVIC_EnableIRQ( TIM17_IRQn ); // Enable interrupt from TIM3 (NVIC level)
}

/**
	* @brief 	This function sets up systick (10uS)
	* @param 	None
	*	@retval None
	*/
void SysTick_Init(void) 
{
	/****************************************
	 *SystemFrequency/1000      1ms         *
	 *SystemFrequency/100000    10us        *
	 *SystemFrequency/1000000   1us         *
	 *****************************************/
	while ((SysTick_Config(SystemCoreClock / 100000)) != 0) {
	} // One SysTick interrupt now equals 1us

} 


/**
	* @brief 	This function sets up the USART at baud 115200
	* @param 	None
	*	@retval None
	*/
#if DEBUG
void UART_Setup(UI32 baud)
{
	// Enable GPIOA clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Configure PA9 and PA10 as Alternate function
	GPIOA->MODER &= ~(GPIO_MODER_MODER9_Msk | GPIO_MODER_MODER10_Msk);
	GPIOA->MODER |= (0x02 <<GPIO_MODER_MODER9_Pos) | (0x02 <<GPIO_MODER_MODER10_Pos);

	// Set PA9 and PA10 to AF1 (USART1)
	GPIOA->AFR[1] &= ~(0x00000FF0);
	GPIOA->AFR[1] |=  (0x00000110);

	// Enable USART1 clock
	RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

	// Clear USART1 configuration (reset state)
	// 8-bit, 1 start, 1 stop, CTS/RTS disabled
	USART1->CR1 = 0x00000000;
	USART1->CR2 = 0x00000000;
	USART1->CR3 = 0x00000000;

	// Select PCLK (APB1) as clock source
	// PCLK -> 48 MHz
	RCC->CFGR3 &= ~RCC_CFGR3_USART1SW_Msk;
	  RCC->CFGR3 |= RCC_CFGR3_USART1SW_0 | RCC_CFGR3_USART1SW_1;
	// Baud Rate = 115200
	// With OVER8=0 and Fck=48MHz, USARTDIV =   48E6/115200 = 416.6666
	// BRR = 417 -> Baud Rate = 115107.9137 -> 0.08% error
	//
	// With OVER8=1 and Fck=48MHz, USARTDIV = 2*48E6/115200 = 833.3333
	// BRR = 833 -> Baud Rate = 115246.0984 -> 0.04% error (better)

	USART1->CR1 |= USART_CR1_OVER8;
	USART1->BRR = 833;//139;//833;//139;//833;//694;// 139;//833;

	// Enable both Transmitter and Receiver
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;

	// Enable USART2
	USART1->CR1 |= USART_CR1_UE;
	
	NVIC_SetPriority(USART1_IRQn, 0);
	/* Enable RTC interrupt */
  NVIC_EnableIRQ(USART1_IRQn);
}
#endif

/**
	* @brief 	This function sets up the ADC
	* @param 	None
	*	@retval None
	*/
void ADC_Setup(void)
{
	// Enable GPIOC clock
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	// Configure pin PC1 as analog
	GPIOA->MODER &= ~GPIO_MODER_MODER4_Msk;
	GPIOA->MODER |= (0x03 <<GPIO_MODER_MODER4_Pos);

	// Enable ADC clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	// Reset ADC configuration
	ADC1->CR 	= 0x00000000;
	ADC1->CFGR1  = 0x00000000;
	ADC1->CFGR2  = 0x00000000;
	ADC1->CHSELR = 0x00000000;

	// Enable continuous conversion mode
	ADC1->CFGR1 |= ADC_CFGR1_CONT;

	// 12-bit resolution
	ADC1->CFGR1 |= (0x00 <<ADC_CFGR1_RES_Pos);

	// Select PCLK/4 as ADC clock
	ADC1->CFGR2 |= (0x01 <<ADC_CFGR2_CKMODE_Pos);
	
	/* calibration */
    ADC1->CR |= ADC_CR_ADCAL;               /* start ADc CALibration */
    while (ADC1->CR & ADC_CR_ADCAL);        /* wait for completion */
    ADC1->CR |= ADC_CR_ADEN;                /* ADc ENable */
    while (!(ADC1->ISR & ADC_ISR_ADRDY));   /* wait for completion */
		

	// Set sampling time to 28.5 ADC clock cycles
	ADC1->SMPR = 0x04;

  ADC->CCR |= ADC_CCR_VREFEN;
	
	// Select channel 11
	ADC1->CHSELR |= ADC_CHSELR_CHSEL17;

	
	// Enable ADC
	ADC1->CR |= ADC_CR_ADEN;

	// Start conversion
	ADC1->CR |= ADC_CR_ADSTART; 
}

/**
	* @brief 	This function sets up the main clock 
  *					Uses internal HSI (8MHz)
  *         Then PLL for 48MHz 
	* @param 	None
	*	@retval None
	*/
void SystemClock_Config(void)
{
	RCC->CR |= RCC_CR_HSION;																		//Turn on High Speed Internal clock (8MHz)
	while((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY) ; 				//Wait for HSI to be ready
	
	RCC->CR |= (RCC_CR_PLLON);																	//Enable PLL
	while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY) ;					//Wait for PLL to be ready
	
	RCC->CFGR = 0x00000000;                            				  //Clear the CFGR register
		
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_DIV2;             			    //HSI is 8MHz so now it is 4MHz
	
	RCC->CFGR |= RCC_CFGR_PLLMUL12;                     				//4MHz x 12 = 48MHz*/
	
	RCC->CFGR |= RCC_CFGR_PPRE_DIV1;                    				//48MHz for the PCLK
	
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;														//48MHz for HCLK
	
	RCC->CFGR |= RCC_CFGR_SW_PLL;																//Select PLL as system Clock
	
	while((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL) ; //Wait for PLL-SYSCLK to become ready
	
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;													//Enable the PORTA Peripheral Clock
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;													//Enable the PORTB Peripheral Clock
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;													//Enable the PORTC Peripheral Clock
	RCC->AHBENR |= RCC_AHBENR_GPIODEN;													//Enable the PORTC Peripheral Clock
	RCC->AHBENR |= RCC_AHBENR_GPIOFEN;													//Enable the PORTC Peripheral Clock
	
	SystemCoreClockUpdate();
}

void Setup( void )
{
	SystemClock_Config();  //Setup clock
	Ports_Setup();         //Setup GPIO
	TIM17_Setup();         //Setup Timer17
	TIM3_Setup();          //Setup Timer3
	SysTick_Init();        //Setup SysTick
	if(DEBUG)UART_Setup(115200); //Setup USART
	ADC_Setup();           //Setup ADC
	
	GetVddVal();           //Get the actual Vdd
	
	OLED_RST_SET();        //Reset the OLED, LORA 
	OLED_DC_SET();
	LORA_CS_SET();
	DF_CS_CLR();
	EE_EWEN();            //EEPROM Write Enable
	LORA_CS_SET();
	
	PrintfP("\nTesting, Hello World!");
	PrintfP("\ntemp = %d",EE_READ(0x22));

	delaymms(10);
	CLK_CLR();
	MOSI_CLR();
	OLED_CS_SET();
	OLED_RST_SET();
	OLED_DC_SET();
	LORA_CS_SET();

	OLED_init();          //Setup OLED
	
	setColAddress(0,127);
	setPageAddress(0,7);
	OLED_Clr(0);
	LoRaSetup();          //Setup LoRA
}
