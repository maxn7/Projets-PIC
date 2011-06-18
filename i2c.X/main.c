#include <stdio.h>
#include <p18f2550.h>
#include <i2c.h>
#include <timers.h>
#include <delays.h>

#define ledv PORTCbits.RC0
#define ledr PORTCbits.RC1
#define longTram 10


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

char tab[longTram] ;
char *i = &tab[0] ;
char x;


//////*PROTOTYPES*////////
	void high_isr(void);



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
    if(PIE1bits.SSPIE && PIR1bits.SSPIF)
    {
        if(SSPSTATbits.R_W) // lecture
        {
            ledv = ledv^1;
            putsI2C("max");
        }
        else // ecriture
        {
            *i = SSPBUF ; // on stocke le byte
            if(!SSPSTATbits.D_A) // si c'était l'addresse
            {
                i=tab;
            }
            else // si c'était de la donnée
            {
                i++;
            }

           /* if(SSPSTATbits.P) ne sert à rien visiblement
            {
                i++;
                *i = 0;
                i=tab;
            }*/
        }
      
        PIR1bits.SSPIF = 0 ; // on réautorise l'interruption
    }

    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF)
    {
	INTCONbits.TMR0IF = 0 ;
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



//adc
//         OpenADC(ADC_FOSC_2 & ADC_LEFT_JUST & ADC_2_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS , 15);
//         SetChanADC(ADC_CH0);

//        OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_8BIT & T0_PS_1_256);

//i2c interrupt

    PIE1bits.SSPIE = 1;
    PIR1bits.SSPIF = 0; //Clear any pending interrupt
    OpenI2C(SLAVE_7, SLEW_OFF);
    SSPADD = 0b10000000 ; //addresse du pic
    SSPCON1bits.CKP = 1; //On relache l'horloge
    SSPBUF = 0 ;
    SSPCON1bits.SSPOV=0;

//autoriser les interruptions
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;


	while(1);
}

