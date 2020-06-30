#include "preprocessor.h"

UI8         USB_RX_Buffer[USB_RX_BUFFSIZE];
UI8         USB_TX_Buffer[USB_TX_BUFFSIZE];

UI16        USB_RX_end, USB_TX_end, USB_RX_start, USB_TX_start=0;

/**
	* @brief 	This function clears the RX buffer
	* @param 	None
	*	@retval None
	*/
void ClrRX(void)
{
	UI16    k;

	USB_RX_start = USB_RX_end = 0;

	for(k=0; k < USB_RX_BUFFSIZE; k++)
	{
		USB_RX_Buffer[k]=0;
	}
}

/**
	* @brief 	This function clears the TX buffer
	* @param 	None
	*	@retval None
	*/
void ClrTX(void)
{
	UI16    k;    

	USB_TX_start = USB_TX_end = 0;

	for(k=0; k < USB_TX_BUFFSIZE; k++)
	{
		USB_TX_Buffer[k]=0;
	}
}

/**
	* @brief 	This function converts a decimel input to hex
	* @param 	input - input to convert
	*	@retval None
	*/
void int_hexasci_print(UI8 input)
{
    	UI8 ucsTempH, ucsTempL;

    	ucsTempH=input & 0xF0;
    	ucsTempH>>=4;
    	if(ucsTempH > 9)
    	{
        	ucsTempH-=10;
        	ucsTempH+='A';
    	}
    	else
        	ucsTempH+='0';

    	ucsTempL=input & 0x0F;
    	if(ucsTempL > 9)
    	{
        	ucsTempL-=10;
        	ucsTempL+='A';
    	}
    	else
        	ucsTempL+='0';

    	PutChar(ucsTempH);
    	PutChar(ucsTempL);
}

/**
	* @brief 	This function prints a char to the serial port
	* @param 	data - char to print
	*	@retval 1 - successful, 0 - not successful
	*/
UI8 PutChar(UI8 data)
{	
	UI16 Temp16;
	
	__disable_irq();
	Temp16=USB_TX_end;
	Temp16++;
	if(Temp16>=USB_TX_BUFFSIZE) Temp16=0;
		
	if(Temp16!=USB_TX_start)
	{        		
				USB_TX_Buffer[USB_TX_end] = data;   // dump character in buffer
				USB_TX_end=Temp16;
				USART1->CR1 |= USART_CR1_TXEIE;
				__enable_irq();
				return(1);                  // return success
	}
	
	return 0;
}

/**
	* @brief 	This function tests to see if there is a character in the receive buffer
	* @param 	None
	*	@retval 1 if there is something, 0 if there is nothing
	*/
UI8 is_Char(void)
{
		return(USB_RX_end != USB_RX_start);
}

/**
	* @brief 	This function gets A single UI8acter from the Serial port receive buffer this is normally 
  * 				called after checking the buffer using is_Char()
	* @param 	None
	*	@retval Character extracted from buffer
	*/
UI8 comms_getch(void)
{
    UI8     temp;
    
		__disable_irq();
		if(USB_RX_start == USB_RX_end)
		{
				__enable_irq();
				return(0);
		}
		else
		{
				temp = USB_RX_Buffer[USB_RX_start];
				USB_RX_start++;
				if(USB_RX_start>=USB_RX_BUFFSIZE )USB_RX_start=0;
				__enable_irq();
				return(temp);
		}
}

/**
	* @brief 	This is the USART1 interrupt handler (TX and RX interrupt) 
	* @param 	None
	*	@retval None
	*/
void USART1_IRQHandler( void )
{
	int temp = 0;
	UI16 k;
	if( ( USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		temp =  USART1->RDR;
		
		k=USB_RX_end;
		k++;
		k&=( USB_RX_BUFFSIZE - 1 ); // wrap

		if( k!=USB_RX_start )
		{
			USB_RX_Buffer[USB_RX_end] = temp;   // dump UI8acter in buffer
			USB_RX_end++;
			USB_RX_end&=( USB_RX_BUFFSIZE - 1 ); // wrap pointer...
		}
	}
	if( ( USART1->ISR & USART_ISR_TXE) == USART_ISR_TXE)
	{
		// character transferred to shift register so UDR is now empty
    	if(USB_TX_start != USB_TX_end)
    	{
        	USART1->TDR=USB_TX_Buffer[USB_TX_start];
	        USB_TX_start++;
	        if(USB_TX_start>=USB_TX_BUFFSIZE) USB_TX_start=0;
    	}
    	else
    	{
	        // there is nothing, so turn the damn interrupt off otherwise I never get time for
	        // anything else	        
					USART1->CR1 &= ~USART_CR1_TXEIE;				
    	}
	}
}

/**
	* @brief 	This function is a custom printf
	* @param 	input - string to print
	*	@retval returns 1 if successful, 0 if not
	*/
UI8 PrintfP(const UI8 *input, ...)    // works like printf
{
    UI16     count;

    UI16    tempt;
    UI8     success=0;
    va_list argptr;

    va_start(argptr, input);

    for (count = 0;count < strlen((const char *)input); count++)
    {
        if (input[count] == 0x0A)
        {
            success += PutChar(0x0D);
            success += PutChar(input[count]);
        }
        else if (input[count] == '%')
        {
            if (input[count + 1] == 'd')
            {
                tempt = va_arg(argptr, int);
                int_asci_print(tempt);
                count++;
            }
            else if (input[count + 1] == 'x')
            {
                tempt = va_arg(argptr, int);
                int_hexasci_print(tempt);
                count++;
            }
            else
            {
                success += PutChar(input[count]);
            }
        }
        else
            success += PutChar(input[count]);

        
    }

    va_end(argptr);

    if (success < count)
        return(0);
    else
        return(1);
}

/**
	* @brief 	This function converts the unsigned character data to ascii data
	* @param 	input - the unsigned characters
	*	@retval None
	*/
void int_asci_print(UI32 input)
{
    UI8 D1000000, D100000, D10000, D1000, D100, D10, D1;
    UI8 nosupp=0;

    // if(input & 0x8000) PutChar('-');
    D1000000=input / 1000000;
    input=input % 1000000;
    D1000000=D1000000 + 0x30;
    if(D1000000 == 0x30)
    {
        D1000000='$';   // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;
        
    D100000=input / 100000;
    input=input % 100000;
    D100000=D100000 + 0x30;
    if((D100000 == 0x30)&& (nosupp))
    {
        D100000='$';    // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;

    D10000=input / 10000;
    input=input % 10000;
    D10000=D10000 + 0x30;
    if((D10000 == 0x30)&& (nosupp))
    {
        D10000='$';     // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;

    D1000=input / 1000;
    input=input % 1000;
    D1000=D1000 + 0x30;
    if((D1000 == 0x30)&& (nosupp))
    {
        D1000='$';      // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;

    D100=input / 100;
    input=input % 100;
    D100=D100 + 0x30;

    if((D100 == 0x30) && (nosupp))
    {
        D100='$';
        nosupp=1;
    }
    else
        nosupp=0;

    D10=input / 10;
    input=input % 10;
    D10=D10 + 0x30;
    if((D10 == 0x30) && (nosupp))
    {
        D10='$';
        nosupp=1;
    }

    D1=input;
    D1=D1 + 0x30;

    if(D10000 != '$') PutChar(D10000);
    if(D1000 != '$') PutChar(D1000);
    if(D100 != '$') PutChar(D100);
    if(D10 != '$') PutChar(D10);
    if(D1 != '$') PutChar(D1);
}

/**
	* @brief 	This function is the same as above, but is dedicated to the OLED printf
	* @param 	charie[] - the character array to be sent to OLED
  *					count1 - how many characters
	*					input - the input array
	*	@retval None
	*/
UI16 int_asci_print2(UI8 charie[], UI8 count1, UI32 input)
{
    UI8 D1000000, D100000, D10000, D1000, D100, D10, D1;
    UI8 nosupp=0;
	  UI8 count = count1;
	
    // if(input & 0x8000) PutChar('-');
    D1000000=input / 1000000;
    input=input % 1000000;
    D1000000=D1000000 + 0x30;
    if(D1000000 == 0x30)
    {
        D1000000='$';   // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;
        
    D100000=input / 100000;
    input=input % 100000;
    D100000=D100000 + 0x30;
    if((D100000 == 0x30)&& (nosupp))
    {
        D100000='$';    // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;

    D10000=input / 10000;
    input=input % 10000;
    D10000=D10000 + 0x30;
    if((D10000 == 0x30)&& (nosupp))
    {
        D10000='$';     // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;

    D1000=input / 1000;
    input=input % 1000;
    D1000=D1000 + 0x30;
    if((D1000 == 0x30)&& (nosupp))
    {
        D1000='$';      // skip this one!!!
        nosupp=1;
    }
    else
        nosupp=0;

    D100=input / 100;
    input=input % 100;
    D100=D100 + 0x30;

    if((D100 == 0x30) && (nosupp))
    {
        D100='$';
        nosupp=1;
    }
    else
        nosupp=0;

    D10=input / 10;
    input=input % 10;
    D10=D10 + 0x30;
    if((D10 == 0x30) && (nosupp))
    {
        D10='$';
        nosupp=1;
    }

    D1=input;
    D1=D1 + 0x30;

    if(D10000 != '$') 
	{
	   charie[count] = D10000;
	  // count++;
	}
    if(D1000 != '$')  
 	{
	   charie[count] = D1000;
	//   count++;
	}
	if(D100 != '$')   
    {
	   charie[count] = D100; 
	   count++;  
	}
	if(D10 != '$')    
    {
	 
	   charie[count] = D10;
	   count++;  
	}
	if(D1 != '$')	  
	{
	   charie[count] = D1;  
	}
	return count;
}

/**
	* @brief 	This is custom printf to print data to OLED
	* @param 	x - x position
  *         y - y position
  *					input - data to be send to OLED
	*	@retval None
	*/
UI8 PrintOLED(UI8 x, UI8 y, const UI8 *input, ...)
{
	UI16     count;
	UI16    tempt;
	UI8     success=0;
	va_list argptr;
	UI8 charie[128];
	UI8 index;

	va_start(argptr, input);
	index = strlen((const char *)input); 

	for (count = 0; count < index; count++)
	{
		if (input[count] == '%')
		{
			if (input[count + 1] == 'd')
			{
				tempt = va_arg(argptr, int);
				count = int_asci_print2(charie, count, tempt);
			}
			else charie[count] = input[count]; 
		}
		else charie[count] = input[count];  
	}

	TransferBuffer(charie, count, x, y);

	va_end(argptr);
	if (success < count) return(0); 
	else return(1); 
}

