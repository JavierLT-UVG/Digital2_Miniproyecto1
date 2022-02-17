/*
 * File:   D2_SPI_Slave2.c
 * Author: fjltu
 *
 * Created on 15 de febrero de 2022, 06:52 PM
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "SPI.h"
#define _XTAL_FREQ 4000000

//============================================================================
//============================ VARIABLES GLOBALES ============================
//============================================================================
uint8_t lectura;                // Variable no utilizada, recibe datos entrantes

//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void config_int(void);
void config_iocb(void);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    if(INTCONbits.RBIF)         // Interrupción de botones, antirrebotes
    {
        if(!PORTBbits.RB4)      // Si se presiona B4, incrementar Puerto D
        {
            PORTD++;
        }
        if(!PORTBbits.RB2)      // Si se presiona B2, incrementar Puerto D
        {
            PORTD--;
        }
        INTCONbits.RBIF = 0;    // Limpiar bandera de overflow
    }
    
    if(PIR1bits.SSPIF)                  // Interrupción de comunicación SPI
    {
        lectura = spi_Read();           // Recibir y almacenar datos entrates
        SSPBUF = PORTD;                 // Enviar valor de contador
        PIR1bits.SSPIF = 0;             // Limpiar bandera
    }
}

//============================================================================
//=================================== MAIN ===================================
//============================================================================
void main(void) 
{
    config_reloj();
    config_io();
    config_int();
    config_iocb();
    spi_Slave();
    
    while(1)
    {
        
    }
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)
{
    ANSEL = 0;
    ANSELH = 0;
    
    TRISAbits.TRISA5 = 1;       // Entrada Slave Select (SPI - Slave)
    
    TRISCbits.TRISC3 = 1;       // Entrada SCK          (SPI - Slave)
    TRISCbits.TRISC4 = 1;       // Entrada SDI          (SPI)
    TRISCbits.TRISC5 = 0;       // Salida SDO           (SPI)
    
    TRISBbits.TRISB2 = 1;       // Entrada Botones de Contador
    TRISBbits.TRISB4 = 1;       // 
    
    TRISD = 0;                  // Salida LEDs de Contador
    
    PORTA = 0;                  // Limpiar condiciones iniciales
    PORTB = 0;                  //
    PORTC = 0;                  //
    PORTD = 0;                  //
}

void config_reloj(void)
{
    OSCCONbits.IRCF2 = 1;       // 4MHz
    OSCCONbits.IRCF1 = 1;       // 
    OSCCONbits.IRCF0 = 0;       // 
    OSCCONbits.SCS = 1;         // Reloj interno
    return;
}

void config_int(void)
{
    INTCONbits.GIE  = 1;        // Activar interrupciones
    INTCONbits.PEIE = 1;        // Activar interrupciones periféricas
    INTCONbits.RBIE = 1;        // Activar interrupciones de Puerto B
    INTCONbits.RBIF = 0;        // Apagar bandera de overflow de Puerto B
    return;
}

void config_iocb(void)
{
    OPTION_REGbits.nRBPU = 0;   // Encender configuración de weak pullups
    WPUB    =   0b00010100;     // Encender 2 pullups del puerto b (botones)
    IOCB    =   0b00010100;     // Encender interrupt on change de los pullups
    return;
}
