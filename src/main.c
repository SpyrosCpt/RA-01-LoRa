#include "preprocessor.h"

#define ADC_TEMPERATURE_CH ADC_CHSELR_CHSEL4

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
UI32 adcraw = 0;
UI32 Vdd = 0;
UI16 VddCal = 0;
UI32 TempVdd = 0;
__IO UI16 adcval = 0;
UI16 TemperatureTimer = 0;
UI16 AddressTimer = 0;

/****************************************/
/**********GLOBAL VARIABLES END**********/
/****************************************/

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
		if(AddressTimer) AddressTimer--;
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

void GetDeviceAddress(void)
{
	UI8 DIPS[4] = {0,0,0,0};
	UI8 Address = 0;
	UI8 i = 0;
	
	if(!AddressTimer)
	{
		if(DIP0_READ() == 0) DIPS[3] = 1;
		if(DIP1_READ() == 0) DIPS[2] = 1;
		if(DIP2_READ() == 0) DIPS[1] = 1;
		if(DIP3_READ() == 0) DIPS[0] = 1;
		
		for(i = 0; i < 4; i++)
		{
			if(DIPS[i] == 1) Address |= (1<<i);
		}
		PrintOLED(0,0, "Address = %d", Address);
		AddressTimer = 100;
	}
}

void getTemp(void)
{
	UI32 temp = 0;
	float temp2 = 0;
	
	if(TemperatureTimer == 0)
	{
		temp = GetAnalogVal(ADC_TEMPERATURE_CH);
		
		#if (TEMP_SENSOR == LM34)
		temp2 = temp*0.088;
		temp2 -= 32;
		temp2 *= 0.55555;
		#else 
		temp2 = (temp*Vdd);
		temp2 /= 4095;
		temp2/=10;
		#endif
		
		PrintOLED(0, 6, "Temp = %dC", (UI16)temp2);
		
		TemperatureTimer = 100;
	}
}

int main( void )
{	
	Setup();                          																			 /* Setup the hardware and peripherals */  
	
	while(1)
	{
		if(PB0_READ() == 0) { TestLoRaTransmitter("Hello World"); }
	  
		getTemp();
		GetDeviceAddress();
		TestLoRaReceiver();
	}
}

