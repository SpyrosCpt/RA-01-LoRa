#include "preprocessor.h"

/****************************************/
/***************PROTOTYPES***************/
/****************************************/

void Setup_ISR( void );
void SPI_Setup(void);
UI8 Checksum(volatile UI8 *data, UI8 length);
UI8 EE_StoreZones(UI8 zone, UI8 sensor);
void EE_EraseZone(UI8 zone);
UI8 EE_Store_All_Zones(void);
void EE_Erase_All_Zones(void);
void EE_ResoreZones(void);
void EELoadData(void);

/****************************************/
/***********END OF PROTOTYPES************/
/****************************************/

/****************************************/
/************GLOBAL VARIABLES************/
/****************************************/
static __IO uint32_t systick=0;
volatile UI32 MILLISECS=0;

void SysTick_Handler( void )
{
   if( systick ) systick--;
	 if( MILLISECS ) MILLISECS--;
}
void delayuus(int us)
{
	systick = us;
	
	while(systick) ;
}

void delaymms(int ms)
{
	MILLISECS = ms*100;
	
	while(MILLISECS) ;
}


/****************************************/
/**********GLOBAL VARIABLES END**********/
/****************************************/


void Setup_ISR( void )
{
	// Enable SYSCFG clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	SYSCFG->EXTICR[2] = 0x01<<12;//|=   SYSCFG_EXTICR2_EXTI5_PB;

	// Enable EXTI line 3
	EXTI->IMR |= EXTI_IMR_IM11;

	// Disable Rising / Enable Falling trigger
	EXTI->RTSR &=  ~EXTI_RTSR_RT11;
	EXTI->FTSR |=  EXTI_FTSR_FT11;
	
	NVIC_SetPriority( EXTI4_15_IRQn, 0 );
	NVIC_EnableIRQ( EXTI4_15_IRQn ); // Enable interrupt from TIM3 (NVIC level)
}


/**
	* @brief 	This is the Timer 3 interrupt handler 
	* @param 	None
	*	@retval None
	*/
void TIM3_IRQHandler( void )
{
	volatile UI8 i;
	if( TIM3->SR & TIM_SR_UIF ) 					/* If UIF flag is set */
  {
		TIM3->SR &= ~TIM_SR_UIF; 					/* Clear UIF flag */
  }
}

/**
	* @brief 	This is the Timer 17 Interrupt 
	* @param 	None
	*	@retval None
	*/
void TIM17_IRQHandler( void )
{
if( TIM17->SR & TIM_SR_UIF ) 					/* If UIF flag is set */
 {
		TIM17->SR &= ~TIM_SR_UIF; 					/* Clear UIF flag */
  }
}

/**
	* @brief 	This is the Timer 14 interrupt 
	* @param 	None
	*	@retval None
	*/

void TIM14_IRQHandler( void )
{
	if( ( TIM14->SR & TIM_SR_CC1IF ) != 0 )
	{
		TIM14->SR &= ~( TIM_SR_CC1IF | TIM_SR_CC1OF);		
	}
}


void TurnOnTempSensing(void)
{
	UI8 temp = 0;
	LORA_CS_CLR();
	BB_WriteData(0x06);
	temp = BB_ReadData();
	LORA_CS_SET();
	PrintfP("\ntemp = 0x%x",temp);
}
UI8 readInputs(void)
{
	UI8 temp = 0;
	
	if(DIP0_READ() == 0) temp |= (1<<0);
	else temp &= ~(1<<0);
	if(DIP1_READ() == 0) temp |= (1<<1);
	else temp &= ~(1<<1);
	if(DIP2_READ() == 0) temp |= (1<<2);
	else temp &= ~(1<<2);
	if(DIP3_READ() == 0) temp |= (1<<3);
	else temp &= ~(1<<3);
	if(PB0_READ() == 0) temp |= (1<<4);
	else temp &= ~(1<<4);
	if(PB1_READ() == 0) temp |= (1<<5);
	else temp &= ~(1<<5);
	if(PB2_READ() == 0) temp |= (1<<6);
	else temp &= ~(1<<6);
	if(PB3_READ() == 0) temp |= (1<<7);
	else temp &= ~(1<<7);
	
	return temp;
}

void getTemp(void)
{
	UI32 temp = 0;
	float temp2 = 0;
	
	while ((ADC1->ISR & ADC_ISR_EOC) != ADC_ISR_EOC);	
	temp = ADC1->DR;
	temp2 = temp*0.088;//(temp*10) / 4095;
	temp2 -= 32;
	temp2 *= 0.55555;
	//temp2 *= 34;
	
	PrintOLED(0, 84, 0, "%dC", (UI16)temp2);
}


int main( void )
{	
	Setup();                          																			 /* Setup the hardware and peripherals */  

	OLED_RST_SET();
	OLED_DC_SET();
	LORA_CS_SET();
	DF_CS_CLR();
	EE_EWEN(); 
	LORA_CS_SET();
	
	PrintfP("\nTesting, Hello World!");
	PrintfP("\ntemp = %d",EE_READ(0x22));

	delayms(10);
	CLK_CLR();
	MOSI_CLR();
	OLED_CS_SET();
	OLED_RST_SET();
	OLED_DC_SET();
	LORA_CS_SET();

	OLED_init();
	setColAddress(0,127);
	setPageAddress(0,7);
	OLED_Clr(0);
	ASK_CLR();
	LoRaSetup();
	
	while(1)
	{
		if(PB0_READ() == 0)
		{
			LED0_SET();
		  TestLoRaTransmitter();
			delaymms(2000);
			LED0_CLR();
		}
	  
		TestLoRaReceiver();
	}
}

