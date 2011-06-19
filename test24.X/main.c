#include<stdio.h>
#include <p24Fxxxx.h>
#define FCY 8000000UL
#include"libpic30.h"
#include"timer.h"
#include"uart.h"

#define ledj PORTAbits.RA0
#define ledr PORTAbits.RA1

//////// CONFIGURATION ////////

_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & BKBUG_OFF & COE_OFF & ICS_PGx1
                    & FWDTEN_OFF & WINDIS_OFF & FWPSA_PR32 & WDTPS_PS1);

_CONFIG2(IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRI & FCKSM_CSDCMD
                 & OSCIOFNC_OFF & IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_HS);

//////// PROTOTYPES ////////
void __attribute__((interrupt, auto_psv)) _T1Interrupt (void);
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt  (void);

//////// INTERRUPTIONS ////////
void __attribute__((interrupt, auto_psv)) _T1Interrupt (void)
{
    ledj = ledj^1;
    putsUART1("Hello World!\n");

    IFS0bits.T1IF = 0 ;
    return;
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt  (void)
{
    ledr = ledr^1;
    ReadUART1();

    IFS0bits.U1RXIF = 0;
    return;
}

//////// PROGRAMME PRINCIPAL ////////
int main ( void )
{
    /* Initialisations. */
    AD1PCFG = 0x1fff;    
    AD1CON2 = 0x0000;   
    CLKDIV = 0;

    /* Configuration I/O. */
   TRISA = 0b1100; // Quartz sur RA2 et 3
   PORTA = 0;
   TRISB=0xFFFF;

   /* Configuration Timer 1. */
   T1CON = T1_ON & T1_IDLE_STOP & T1_GATE_OFF & T1_PS_1_256 & T1_SOURCE_INT ;
   IEC0bits.T1IE = 1 ; 
   IFS0bits.T1IF = 0 ;
   IPC0bits.T1IP = 4 ; /* Priorité par défaut : 4. */

   /* Configuration du module UART1. */
   __C30_UART = 1;

   U1MODE = UART_EN & UART_IDLE_CON & UART_IrDA_DISABLE   & UART_MODE_SIMPLEX
          & UART_UEN_00             & UART_EN_WAKE        & UART_DIS_LOOPBACK
          & UART_DIS_ABAUD           & UART_UXRX_IDLE_ZERO & UART_BRGH_SIXTEEN
          & UART_NO_PAR_8BIT        & UART_1STOPBIT ;

   U1STA = UART_INT_TX_LAST_CH & UART_IrDA_POL_INV_ZERO & UART_SYNC_BREAK_DISABLED
         & UART_TX_ENABLE      & UART_INT_RX_CHAR       & UART_ADR_DETECT_DIS
         & UART_RX_OVERRUN_CLEAR ;

   U1BRG = 51 ;

   RPINR18bits.U1RXR = 2;   /* RX1 sur RP2    */
   RPINR18bits.U1CTSR = 14; /* CTS1 sur RP14 */
   RPOR1bits.RP3R = 3 ;     /* RP3 pour TX1  */
   RPOR7bits.RP15R = 4 ;    /* RP15 pour RTS */

   IEC0bits.U1RXIE = 1; /* Interruption de réception. */
   IPC2bits.U1RXIP = 4; /* Priorité par défaut. */

   /* On autorise les interuptions. */
   INTCON1bits.NSTDIS = 0;
   INTCON2bits.ALTIVT = 0;

   PORTA = 0b01;

   while(1);

    return 0;
}