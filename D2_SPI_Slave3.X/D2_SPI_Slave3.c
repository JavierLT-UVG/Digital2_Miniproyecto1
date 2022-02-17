/*
 * File:   D2_SPI_Slave3.c
 * Author: fjltu
 *
 * Created on 15 de febrero de 2022, 06:53 PM
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
uint16_t adres_aux;             // Almacena el último valor de la conversión ADC
uint16_t adres_map;             // Almacena el valor mapeado del adres

uint8_t lectura;                // Variable no utilizada, recibe datos entrantes

//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void config_int(void);
void config_adc(void);
uint32_t map(uint16_t num);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    if(PIR1bits.ADIF)                   // Interrupción del ADC
    {
        adres_aux = ADRESH;             // Colocar ADRESL en el auxiliar (2 bits más significativos)
        adres_aux = adres_aux << 8;     // Left shift 8 posiciones para que abrir espacio a ADRESL
        adres_aux = adres_aux | ADRESL; // Llenar los bits menos significativos con ADRESL
        PIR1bits.ADIF = 0;              // Limpiar bandera
    }
    
    if(PIR1bits.SSPIF)                  // Interrupción de comunicación SPI
    {
        lectura = spi_Read();           // Recibir y almacenar datos entrates
        SSPBUF = adres_map;             // Enviar mapeo del ADC del pot
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
    spi_Slave();
    config_adc();
    
    while(1)
    {
        if(!ADCON0bits.GO)              // Si la conversión ya terminó, entrar
        {
            __delay_us(50);             // Delay para no interrumpir conversión
            ADCON0bits.GO = 1;          // Iniciar nueva conversión
        }
        
        adres_map = map(adres_aux);
        
        if(adres_map <= 23){
            PORTBbits.RB7 = 0;      // LED verde encendida
            PORTBbits.RB5 = 0;
            PORTBbits.RB3 = 1;
        }else if (adres_map < 28){
            PORTBbits.RB7 = 0;      // LED amarilla encendida
            PORTBbits.RB5 = 1;
            PORTBbits.RB3 = 0;
        }else{
            PORTBbits.RB7 = 1;      // LED roja encendida
            PORTBbits.RB5 = 0;
            PORTBbits.RB3 = 0;
        }
    }
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)
{
    ANSEL = 0;                  // Pines digitales (AN7 - AN0)
    ANSELH = 0b00010000;        // Pin analógico (AN12 = RB0)
    
    TRISBbits.TRISB0 = 1;       // Entrada LM35         (ADC)
    TRISBbits.TRISB3 = 0;       // LED semáforo (Rojo)
    TRISBbits.TRISB5 = 0;       // LED semáforo (Amarillo)
    TRISBbits.TRISB7 = 0;       // LED semáforo (Verde)
    
    TRISAbits.TRISA5 = 1;       // Entrada Slave Select (SPI - Slave)
    
    TRISCbits.TRISC3 = 1;       // Entrada SCK          (SPI - Slave)
    TRISCbits.TRISC4 = 1;       // Entrada SDI          (SPI)
    TRISCbits.TRISC5 = 0;       // Salida SDO           (SPI)
    TRISD = 0;
    
    PORTA = 0;                  // Limpiar condiciones iniciales
    PORTB = 0;                  //
    PORTC = 0;                  //
    PORTD = 0;
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
    PIE1bits.ADIE = 1;          // Activar interrupción de ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera de ADC
    return;
}

void config_adc(void)           // Configuración del ADC
{
    ADCON1bits.ADFM = 1;        // Justificación a la derecha
    ADCON1bits.VCFG0 = 0;       // Vss como referencia
    ADCON1bits.VCFG1 = 0;       // Vdd como referencia
    
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON0bits.CHS = 12;        // Selección del canal 12
    ADCON0bits.ADON = 1;        // ADC encendido
    __delay_us(50);             // Delay de 50us
    
    ADCON0bits.GO = 1;          // Activar primer ciclo de conversión
    return;
}

uint32_t map(uint16_t num)      // Mapear valores de rango 1023/0 a 500/0 500/1024
{
    uint32_t res;               // Definir variable de salida
    res = num*125/256;          // Factor de mapeo
    return res;                 // Devolver variable mapeada
}