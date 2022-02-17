/*
 * File:   D2_SPI_Master.c
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
#include "LCD.h"
#include "UART.h"
#define _XTAL_FREQ 4000000

//============================================================================
//============================ VARIABLES GLOBALES ============================
//============================================================================
uint8_t enviar_nada;
uint8_t bandera_UART;

char txt_pot[] = "S1: ";
char txt_cont[] = "    S2: ";
char txt_temp[] = "    S3: ";

struct datos_recibidos
{
    uint8_t var8bits;
    uint16_t mapeado;
    uint8_t centenas;
    uint8_t decenas;
    uint8_t unidades;
}pot, cont, temp;

//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void config_timer1(void);
uint16_t map(uint8_t num);
void divisor(uint16_t num, uint8_t *centena, uint8_t *decena, uint8_t *unidad);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    if(PIR1bits.TMR1IF)         // Si la bandera está encendida, entrar
    {
        bandera_UART++;       // Activar bandera que permite el TX de UART dentro del main
        TMR1H = 11;             // Reset de los valores del timer
        TMR1L = 220;            // 
        PIR1bits.TMR1IF = 0;    // Limpiar bandera de overflow
    }
}

//============================================================================
//=================================== MAIN ===================================
//============================================================================
void main(void) 
{
    config_reloj();
    config_io();
    spi_Master();
    config_tx();
    config_timer1();
 
    ini_LCD();          // Inicializar LCD
    w_Titulos();        // Escribir layout en LCD
    
    while(1)
    {
        PORTBbits.RB0 = 0;          // Slave 1 seleccionado
        __delay_ms(5);
        SSPBUF = enviar_nada;       // Enviarle variable vacia por protocolo
        pot.var8bits = spi_Read();  // Recibir valor de ADC del potenciometro
        __delay_ms(5);
        PORTBbits.RB0 = 1;          // Slave 1 deseleccionado
        __delay_ms(5);
        
        PORTBbits.RB1 = 0;          // Slave 2 seleccionado
        __delay_ms(5);
        SSPBUF = enviar_nada;       // Enviarle variable vacia por protocolo
        cont.var8bits = spi_Read(); // Recibir valor de contador de 8 bits
        __delay_ms(5);
        PORTBbits.RB1 = 1;          // Slave 2 deseleccionado
        __delay_ms(5);

        PORTBbits.RB2 = 0;          // Slave 3 seleccionado
        __delay_ms(5);
        SSPBUF = enviar_nada;       // Enviarle variable vacia por protocolo
        temp.var8bits = spi_Read(); // Recibir valor de ADC del sensor LM35
        __delay_ms(5);
        PORTBbits.RB2 = 1;          // Slave 3 deseleccionado 
        __delay_ms(5);
        
        // Mapear de 255(8b) a 500(9b)
        pot.mapeado = map(pot.var8bits);
        // Obtener los dígitos de cada valor que aparecerá en el LCD
        divisor(pot.mapeado, &pot.centenas, &pot.decenas, &pot.unidades);
        divisor(cont.var8bits, &cont.centenas, &cont.decenas, &cont.unidades);
        divisor(temp.var8bits, &temp.centenas, &temp.decenas, &temp.unidades);
        
        // Escribir valor de Pot en LCD
        set_Cursor(1,0);
        w_Char(pot.centenas + '0');
        set_Cursor(1,2);
        w_Char(pot.decenas + '0');
        set_Cursor(1,3);
        w_Char(pot.unidades + '0');
        
        // Escribir valor de Cont en LCD
        set_Cursor(1,6);
        w_Char(cont.centenas + '0');
        set_Cursor(1,7);
        w_Char(cont.decenas + '0');
        set_Cursor(1,8);
        w_Char(cont.unidades + '0');
        
        // Escribir valor de Temp en LCD
        set_Cursor(1,11);
        w_Char(temp.centenas + '0');
        set_Cursor(1,12);
        w_Char(temp.decenas + '0');
        set_Cursor(1,13);
        w_Char(temp.unidades + '0');
        
        // Escribir los digitos de cada valor de la LCD en un array de chars
        char pot_string[] = {pot.centenas+'0', '.', pot.decenas+'0', pot.unidades+'0', '\0'};
        char cont_string[] = {cont.centenas+'0', cont.decenas+'0', cont.unidades+'0', '\0'};
        char temp_string[] = {temp.centenas+'0', temp.decenas+'0', temp.unidades+'0', '\0'};
        
        // Ciclo de escritura UART ejecutable cada 1 segundo
        if(bandera_UART>1)
        {
            enviar_string(txt_pot);     // Enviar titulo de pot
            enviar_string(pot_string);  // Enviar valores del pot
            
            enviar_string(txt_cont);    // Enviar titulo de contador
            enviar_string(cont_string); // Enviar valores de contador
            
            enviar_string(txt_temp);    // Enviar titulo de lm35
            enviar_string(temp_string); // Enviar valores de temperatura
            enviar_char(13);            // Salto de línea al terminar de escribir
            
            bandera_UART = 0;   // Apagar y esperar al siguiente ciclo de Tmr1
        }
    }
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)
{
    ANSEL = 0;                  // Pines digitales
    ANSELH = 0;                 //
    
    TRISA = 0;                  // Salida PuertoD       (LCD)
    TRISB = 0b000;              // Salida Slave Selects (SPI - Master)
    TRISC = 0b00;               // Salida de RS y E     (LCD)
    TRISCbits.TRISC3 = 0;       // Salida SCK           (SPI - Master)
    TRISCbits.TRISC4 = 1;       // Entrada SDI          (SPI)
    TRISCbits.TRISC5 = 0;       // Salida SDO           (SPI)
    TRISCbits.TRISC6 = 0;       // Salida TX            (UART)
    TRISD = 0;
    
    PORTA = 0;                  // Limpiar condiciones iniciales
    PORTB = 0;                  //
    PORTC = 0;                  //
    PORTD = 0;
    enviar_nada = 0;            //
    return;
}

void config_reloj(void)
{
    OSCCONbits.IRCF2 = 1;       // 4MHz
    OSCCONbits.IRCF1 = 1;       // 
    OSCCONbits.IRCF0 = 0;       // 
    OSCCONbits.SCS = 1;         // Reloj interno
    return;
}

void config_timer1(void)        // Timer1 a 0.5 segundos
{
    T1CONbits.T1CKPS1 = 1;      // bits 5-4  Prescaler Rate Select bits
    T1CONbits.T1CKPS0 = 1;      // bit 4
    T1CONbits.T1OSCEN = 0;      // bit 3 Timer1 Oscillator Enable Control bit 1 = on
    T1CONbits.T1SYNC = 1;       // bit 2 Timer1 External Clock Input Synchronization Control bit...1 = Do not synchronize external clock input
    T1CONbits.TMR1CS = 0;       // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
    T1CONbits.TMR1ON = 1;       // bit 0 enables timer
    TMR1H = 11;                 // preset for timer1 MSB register
    TMR1L = 220;                // preset for timer1 LSB register

    INTCONbits.PEIE = 1;
    PIE1bits.TMR1IE = 1;
    INTCONbits.GIE = 1;
}

uint16_t map(uint8_t num)       // Mapeado de pot de ADC
{
    uint16_t res;               // Definir variable de salida
    res = num*100/51;           // Factor de mapeo 500:255 = 100:51
    return res;
}

void divisor(uint16_t num, uint8_t *centena, uint8_t *decena, uint8_t *unidad)  // Divisor que saca los dígitos de un número
{
    *centena = num / 100;       // Obtener centenas
    uint8_t aux = num % 100;    // Auxliar que almacena decenas y unidades
    *decena = aux / 10;         // Obtener decenas
    *unidad = aux % 10;         // Obtener unidades
    return;
}