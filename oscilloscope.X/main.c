#include <stdio.h>
#include <p18f2550.h>
#include <math.h>
#include <adc.h>
#include <timers.h>
#include <delays.h>
#include <usart.h>

#define ledv PORTCbits.RC0
#define ledr PORTCbits.RC1
#define buffSize 256


#pragma config FOSC = HS
#pragma config FCMEN = ON
#pragma config IESO = OFF
#pragma config PWRT = OFF
#pragma config BOR = ON
#pragma config BORV = 2
#pragma config VREGEN = ON
#pragma config WDT = OFF
#pragma config MCLRE = ON
#pragma config LPT1OSC = OFF
#pragma config PBADEN = OFF
#pragma config CCP2MX = OFF
#pragma config LVP = OFF
#pragma config DEBUG = ON


/////VARIABLES GLOBALES ////



//////*PROTOTYPES*////////
	void high_isr(void);
        void base_temp(char t);

unsigned int x = 0, unsync = 0, sendFlag = 0 ;
unsigned int tinit = 0;

unsigned char buff[buffSize] ;
unsigned char * pos = &buff[0] ; // pointeur sur chaine
unsigned char trig = 0, eps = 2;

/////*INTERRUPTIONS*/////
#pragma code high_vector=0x08
void high_interrupt(void)
{
     _asm GOTO high_isr _endasm
}
#pragma code

#pragma interrupt high_isr
void high_isr(void)
{
  

    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF)
	{
       // ledv = ledv^1; // signal
        WriteTimer0(tinit);    // reinitialisation timer
        ledv = 0 ;
        ConvertADC();       // on lance la conversion
        while(BusyADC());
        x = ADRESH ;        // on place l'échantillon dans x

        if(pos < &buff[buffSize] ) // si on est pas en bout de chaine
        {
            pos++;
            *pos = x;
        }
        else // si on est en bout de chaine
        {
            if(sendFlag) // si envoi demandé
            {
                putsUSART(buff);
                sendFlag = 0;
            }
            if( fabs(x - trig) < eps)
            {
                ledv = 1 ;
                pos = &buff[0];
                ledr = 0;
                *pos = x ;
                pos++;
            }
            else // si on n'est pas dans la gamme du tigger
            {
                unsync ++ ;
                if(unsync > 10) // si on n'arrive pas à resyncrhoniser
                {
                    trig = x ; // on fixe un nouveau tigger
                    unsync = 0;
                    ledr = 1;
                }
            }
        }




        
	INTCONbits.TMR0IF = 0 ; //on réautorise l'interruption
	}
    if(PIR1bits.RCIF && PIE1bits.RCIE) // réception usart
    {
        sendFlag = 1 ;
        ReadUSART(); // vider le buffer
        PIR1bits.RCIF = 0;
    }

}


////////*PROGRAMME PRINCIPAL*////////

void main (void)
{
//initialisations
	CMCON=  0b00000111; // turn off comparators
	ADCON0= 0b00000000;
	ADCON1= 0x0F ;//0b00001111;
	WDTCON = 0 ;
	OSCCON = 0b01111100;

	UCON = 0 ;            // turn off usb
	UCFG = 0b00001000 ;

	TRISA = 0b11000011 ;
	TRISB = 0b00000011 ; // SCL et SDA en entrées
	TRISC = 0b11111100;
	PORTA = 0b11000000 ;
	PORTB = 0b11000011 ;
	PORTC = 0b00000000 ;







//timer interrupt
        base_temp(4);


//adc
         OpenADC(ADC_FOSC_2 & ADC_LEFT_JUST & ADC_2_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS , 15);
         SetChanADC(ADC_CH0);
//usart     
        OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_LOW , 25 );

//autoriser les interruptions
	INTCONbits.GIE = 1;
 	INTCONbits.PEIE = 1;
/* Enable interrupt priority */
  RCONbits.IPEN = 1;

  /* Make receive interrupt high priority */
  IPR1bits.RCIP = 1;

  /* Enable all high priority interrupts */
  INTCONbits.GIEH = 1;




	while(1);


        

}

void base_temp(char t)
{
    switch(t)
    {
    case 1 : // 0.01s
        tinit = 99;
	OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_8BIT & T0_PS_1_256);
	WriteTimer0(tinit);
        break;

    case 2 : // 0.1s
        tinit = 40536;
	OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_16BIT & T0_PS_1_16);
	WriteTimer0(tinit);
        break;

    case 3 : //
        tinit = 0;
	OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_16BIT & T0_PS_1_1);
	WriteTimer0(tinit);
        break;
    case 4 : //
        tinit = 99;
	OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_8BIT & T0_PS_1_64);
	WriteTimer0(tinit);
        break;


    case 0 : // arreter le timer0
        INTCONbits.TMR0IE = 0;
        break;

    }
}

