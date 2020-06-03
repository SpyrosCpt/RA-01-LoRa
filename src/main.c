#include "preprocessor.h"

#define ADC_TEMPERATURE_CH ADC_CHSELR_CHSEL4

UI32 adcraw = 0;
UI32 Vdd = 0;
UI16 VddCal = 0;
UI32 TempVdd = 0;
__IO UI16 adcval = 0;
UI16 TemperatureTimer = 0;


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
		
		if(TemperatureTimer) TemperatureTimer--;
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
void EnableADC(void)
{
	/* (1) Clear the ADRDY bit */
	/* (2) Enable the ADC */
	/* (3) Wait until ADC ready */
	ADC1->ISR |= ADC_ISR_ADRDY; /* (1) */
	ADC1->CR |= ADC_CR_ADEN; /* (2) */
	if ((ADC1->CFGR1 & ADC_CFGR1_AUTOFF) == 0)
	{
		while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) /* (3) */
		{
		/* For robust implementation, add here time-out management */
		}
	}
}

/**
	* @brief 	This disables the ADC
	* @param 	none		  
	* @retval none
	*/
/*MOVE*/
void DisableADC(void)
{
	/* (1) Ensure that no conversion on going */
	/* (2) Stop any ongoing conversion */
	/* (3) Wait until ADSTP is reset by hardware i.e. conversion is stopped */
	/* (4) Disable the ADC */
	/* (5) Wait until the ADC is fully disabled */
	if ((ADC1->CR & ADC_CR_ADSTART) != 0) /* (1) */
	{
		ADC1->CR |= ADC_CR_ADSTP; /* (2) */
	}
	while ((ADC1->CR & ADC_CR_ADSTP) != 0) /* (3) */
	{
	/* For robust implementation, add here time-out management */
	}
	ADC1->CR |= ADC_CR_ADDIS; /* (4) */
	while ((ADC1->CR & ADC_CR_ADEN) != 0) /* (5) */
	{
	/* For robust implementation, add here time-out management */
	}
}

UI16 Get_ADC_Val( void )
{ 
	UI8 samples = 0;                					    /* Each sample takes 1uS so we should get this over and done with in about 100uS */
	UI8 NumSamples = 60;
	UI32 adcreg=0;
	UI16 Val16;
	
	ADC1->CR |= ADC_CR_ADSTART;                 /* Start conversion */
	for( samples = 0; samples < NumSamples; samples++ )   /* Take 5 samples and add them together */
	{
		
		while( ( ADC1->ISR & ADC_ISR_EOC ) == 0 );  /* Wait for conversion to finish */
		
		adcreg += ADC1->DR;								        /* Get value from ADC data reg */
	}
	adcreg/=NumSamples;											    /* Get an average off all the samples */	
	Val16=adcreg;
		
	return Val16;

}

void GetVddVal(void)
{
	adcraw = 0;

	DisableADC();
	ADC1->CHSELR = 0x00000000;
	ADC1->CHSELR |= ADC_CHSELR_CHSEL17;                         /* Set ADC channel to Vref (CH17) */
	EnableADC();
	
	adcraw = Get_ADC_Val();

	Vdd = 3300UL * (*VREFINT_CAL) / adcraw;
	
	PrintfP("\nVdd = %d", Vdd);
	PrintfP("\nADC = %d", adcraw);

	if(Vdd>3500) Vdd=3300; //sanity
	if(Vdd<3000) Vdd=3300;
	
}


UI16 GetAnalogVal(UI8 ch)
{
	adcval = 0;

	DisableADC();
	
	ADC1->CHSELR = 0x00000000;
	ADC1->CHSELR |= ch;                         /* Set ADC channel to Vref (CH17) */

	EnableADC();
	
	adcval = Get_ADC_Val();

	return adcval;
}

void getTemp(void)
{
	UI32 temp = 0;
	float temp2 = 0;
	
	if(TemperatureTimer == 0)
	{
		temp = GetAnalogVal(ADC_TEMPERATURE_CH);
		temp2 = temp*0.088;
		temp2 -= 32;
		temp2 *= 0.55555;
		
		
		PrintOLED(0, 0, 0, "TEMP = %dC", (UI16)temp2);
		//TransferBuffer("HELLOWORLD", 10, 63, 2, 0);
		TemperatureTimer = 500;
	}
}


int main( void )
{	
	Setup();                          																			 /* Setup the hardware and peripherals */  
	
	GetVddVal();
	
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
		if(PB0_READ() == 0) TestLoRaTransmitter();
	  
		getTemp();
		//TestLoRaReceiver();
	}
}

