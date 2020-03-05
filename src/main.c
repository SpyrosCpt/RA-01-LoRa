#include "preprocessor.h"

#define FLASH_FKEY1  0x45670123
#define FLASH_FKEY2	 0xCDEF89AB
#define FLASH_START	 0x08033F000 //this is page 126 
#define isTXE     (SPI1->SR & SPI_SR_TXE)
#define isRXNE    (SPI1->SR & SPI_SR_RXNE)
#define SPIBUF8   *(__IO uint8_t  *)&SPI1->DR
#define SPIBUF16  SPI1->DR
#define SPECIAL_REMOTE	1
/*#define PULSE_HIGH_HIGH         ASK_SET(); delayuus(PULSE_DELAY_1); ASK_CLR(); delayuus(PULSE_DELAY_2); ASK_SET(); delayuus(PULSE_DELAY_1); ASK_CLR(); delayuus(PULSE_DELAY_2)
#define PULSE_LOW_HIGH          ASK_SET(); delayuus(PULSE_DELAY_3); ASK_CLR(); delayuus(PULSE_DELAY_4); ASK_SET(); delayuus(PULSE_DELAY_1); ASK_CLR(); delayuus(PULSE_DELAY_2)
#define PULSE_END               delayuus(PULSE_DELAY_5); ASK_SET(); delayuus(PULSE_DELAY_2); ASK_CLR()
#define PULSE_PACKET_END        ASK_SET(); delayuus(PULSE_DELAY_6); ASK_CLR(); delayuus(PULSE_DELAY_7)//660*/
#define PULSE_HIGH_HIGH         ASK_SET(); delayuus(60); ASK_CLR(); delayuus(26); ASK_SET(); delayuus(60); ASK_CLR(); delayuus(26)
#define PULSE_LOW_HIGH          ASK_SET(); delayuus(18); ASK_CLR(); delayuus(68); ASK_SET(); delayuus(60); ASK_CLR(); delayuus(26)
#define PULSE_END               delayuus(143); ASK_SET(); delayuus(26); ASK_CLR()
#define PULSE_PACKET_END        ASK_SET(); delayuus(18); ASK_CLR(); delayuus(660)
#define PULSE_DELAY_1           70       //high on
#define PULSE_DELAY_2           38       //high off
#define PULSE_DELAY_3           17       //low high on
#define PULSE_DELAY_4           98       //low high off
#define PULSE_DELAY_5           150       //pulse end high
#define PULSE_DELAY_6           20       //
#define PULSE_DELAY_7           650       //
/****************************************/
/***************PROTOTYPES***************/
/****************************************/

uint8_t Flash_LEDS( uint8_t numsensor );
void vprint( const char *fmt, va_list argp );
void Printf( const char *fmt, ... );
void Setup_ISR( void );
void SPI_Setup(void);

UI8 Array_Match( void );
UI8 Scan_Buttons( void );
void Activate_Inputs( UI8 trig );
void Deactivate_Inputs( void );
void Set_Array_Defaults( void );
UI8 Prepare_Trigger( void );

UI16 ArrayToNumber(UI8 arr[]);
void NumberToArray(UI16 number, UI8 zone, UI8 sensor);

void Assign_To_Zone( UI8 zone );
void Clear_Zone( UI8 zone );
void Clear_All_Zones( void );


UI8 Checksum(volatile UI8 *data, UI8 length);
//void DoChecksum(void);

//UI8 ReadChecksumStartup(void);

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
volatile static uint16_t Timer1 = 0;
volatile static uint16_t Timer2 = 0;
volatile static uint16_t Timer3 = 0;
volatile uint32_t tempreg = 0;
volatile uint32_t tempreg2 = 0;

volatile UI8 capture_flag=0;
volatile UI8 RFHunting = 1;
volatile UI8 WaitingForFallingEdge = 1;
volatile UI8 WaitingForFalling;
volatile UI8 Pulses;
volatile UI16 PArray[24];
volatile UI8	VArray[24];
volatile UI8	SArray[12];
volatile UI8 Buttons[10];
volatile UI8 PulseHunting=0;
volatile UI16 Zone_Flags = 0;
volatile UI8 Flashing = 0;
volatile UI16 Active_Zones = 0;
volatile UI32 MILLISECS=0;
volatile UI16 Timers[10];
volatile UI8 SPI_Arr[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile UI8 pos = 0;
UI8 arr1[8] = {0,0,0,0,0,0,1,1}; //0x03
UI8 arr2[8] = {0,0,0,0,0,0,1,0}; //address 0x03	
UI8 arr3[8] = {0,0,0,0,0,0,0,0}; //dummy
	
UI8 testArr[8] = {0,0,0,1,0,0,1,0};

char SensorAddress[12] = {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}; //Sensor address that is sent (0 to 10 - address, 11 - battery low/ok)

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

void Send_Packet( void )
{
	int i =0;
	int j = 0;
	UI16 PacketsToSend=40;
	
  SetOpMode( TX_gc );
	
	delayuus(600); //give the boost some time to get going
	for( i = 0; i < PacketsToSend; i++ )                             /* send packets */
	{
		for( j = 0; j < 12; j++ )
		{
			if( SensorAddress[j] == 1 )
			{
				ASK_SET(); 
				delayuus(60);   //60
				ASK_CLR(); 
				delayuus(26);   //26
				ASK_SET(); 
				delayuus(60);   //60
				ASK_CLR(); 
				delayuus(26);   //26
			}
			else { PULSE_LOW_HIGH; }
		}
		PULSE_PACKET_END;
	}
	PULSE_END;
	
	SetOpMode( SLEEP_gc );
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
	* @brief 	This is the External Interrupt handler to capture the Sensor messages
	* @param 	None
	*	@retval None
	*/
void EXTI4_15_IRQHandler( void )
{
		volatile UI8 TempL,TempH;
    volatile UI16 Interval;
    volatile UI16 Temp16;
    volatile UI16 Period;
    volatile UI8 i;
    volatile UI16 MinInt,MaxInt;
    volatile UI16 Dev;
	  volatile UI16 TempTimer1, TempTimer2;
	
	  TIM17->CR1 &= ~TIM_CR1_CEN;     // stop timer
	  if ((EXTI->PR & EXTI_PR_PR11_Msk) != 0)
		{
			EXTI->PR |= EXTI_PR_PR11;
			TIM17->CR1 &= ~TIM_CR1_CEN;     // stop timer
			//LED3_SET();
			if(PulseHunting == 0)						//waiting for 6.6mS low (this means we miss the first packet, but who cares)
			{
				if(RFHunting)
				{
					//LED3_TOG();
					if(WaitingForFallingEdge)
					{
					  Timer1 = 0x00;
					  TIM17->CR1 |= TIM_CR1_CEN;      // restart Timer
					
            WaitingForFallingEdge=0;
					
					  EXTI->RTSR |=  EXTI_RTSR_RT11;//next time we want a rising edge..
	          EXTI->FTSR &=  ~EXTI_FTSR_FT11;				
					}
					else //this was a rising edge
					{
					  TIM17->CR1 &= ~TIM_CR1_CEN;      // stop Timer
					  Interval = Timer1;
					
            if(Interval>900	) //600
            {	  
							if(Interval<2000)
							{
								
								RFHunting = 0;
								Timer1 = 0x00;
								TIM17->CR1 |= TIM_CR1_CEN;      // restart Timer
								WaitingForFalling  = 1;
								Pulses = 0;
									
								EXTI->RTSR &=  ~EXTI_RTSR_RT11;//next time we want a falling edge..
								EXTI->FTSR |=  EXTI_FTSR_FT11;
								PulseHunting = 1;
							}
						}
						WaitingForFallingEdge = 1;
					}
				}	
			}
			else
			{
				if( Pulses < 24 )
				{					
					if( WaitingForFalling )        //this was a falling edge 
					{
						TIM17->CR1 &= ~TIM_CR1_CEN;      // stop Timer
									
						Interval = Timer1; 
						PArray[Pulses]=Interval;
						Pulses++;

						EXTI->RTSR |=  EXTI_RTSR_RT11;//next time we want a rising edge..
						EXTI->FTSR &=  ~EXTI_FTSR_FT11;
						WaitingForFalling = 0;
							 
						TIM17->CR1 |= TIM_CR1_CEN;      // start Timer
					}
					else //this was a rising edge
					{
						TIM17->CR1 &= ~TIM_CR1_CEN;      // stop Timer
						Period = Timer1;
							 
						if((Period>200)||(Period<5))
						{
							RFHunting=1;
							PulseHunting = 0;
							Pulses=30;
						}
						EXTI->RTSR &=  ~EXTI_RTSR_RT11;//next time we want a falling edge..
						EXTI->FTSR |=  EXTI_FTSR_FT11;
						Timer1 = 0;
						TIM17->CR1 |= TIM_CR1_CEN;      // start Timer
						WaitingForFalling = 1;
					}
						 
					if(Timer1 > 1000)
					{
						Pulses = 30;
						PulseHunting = 0;
						RFHunting = 1;
						TIM17->CR1 &= ~TIM_CR1_CEN;      // stop Timer
					}					 
				}
				else
				{
					PulseHunting = 0;
					RFHunting = 1;
					LED3_SET();
				}
			}
		}			
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

		if(Timer3) Timer3--;
		if(Timer2) Timer2--;
		
		if(Timer1 > 2000) TIM17->CR1 &= ~TIM_CR1_CEN;     // stop timer
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

		Timer1++;
		
		if(Timer1 > 2000)
		{
			TIM17->CR1 &= ~TIM_CR1_CEN;      // stop Timer
		}
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
		
		if(tempreg == 0)
		{
			tempreg = 1;
		  GPIOA->BSRR |= 1<<15;
			TIM14->CCER = 0x01;
		}
		else
		{
			tempreg = 0;
			GPIOA->BRR |= 1<<15;
			TIM14->CCER = 0x03;
		}			
	}
}


void TurnOnTempSensing(void)
{
	UI8 temp = 0;
	LORA_CS_CLR();
	BB_WriteData(0x06);
	//LORA_CS_SET();
	//LORA_CS_CLR();
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
	UI8 send = 0;
	UI8 temp = 44;
	UI16 i = 0;
	
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

	//UI8 j = 0;
  //i = 65000;
  ///PrintfP("\nTestNum = %d", SystemCoreClock);
	//PrintfP("\nSYSCLK = %d MHz\r\n", SystemCoreClock/1000000);
	//PrintOLED(0, 12, 0, "TEMP: ");
	//PrintOLED(0, 12, 0, "%dMHz ",SystemCoreClock/1000000);
	//LED0_SET();
	//Setup_ISR();
	
	LoRaSetup();
	
	while(1)
	{
		//getTemp();
		//LED1_TOG();
		//delaymms(100);
		
		
		//delaymms(1000);
		
		/*if(PB0_READ() == 0)
		{
			RFHunting=0;
			LED0_SET();
			//PrintOLED(0, 0, 3, "SEND PKT");
			//SetOpMode(TX_gc);
			Send_Packet();
			//ASK_SET();
			//Timer2=5000;
			//while(Timer2);
			delaymms(5000);
			//PrintOLED(0, 0, 3, "        ");
			//delayms(200);
			SetOpMode(RX_gc);
			ASK_CLR();
			LED0_CLR();
			RFHunting = 1;
			LED3_CLR();
		}
		
		if(RFHunting==0) 
		{
			RFHunting = 1;
			//FoundFlag = 1;
			
			LED3_SET();
			delaymms(500);
		}*/
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

