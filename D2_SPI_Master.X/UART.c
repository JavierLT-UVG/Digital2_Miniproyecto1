#include "UART.h"


void config_tx(void)
{
    SPBRGH = 0;
    SPBRG = 103;                // Selector de Baud Rate
    TXSTAbits.SYNC = 0;         // Modo asíncrono
    TXSTAbits.BRGH = 1;         // High Baud Rate alta velocidad
    BAUDCTLbits.BRG16 = 1;      // Generador de Baud Rate de 8 bits
    
    RCSTAbits.SPEN = 1;         // Habilitar puertos seriales
    TXSTAbits.TX9 = 0;          // Envío de 8 bits
    TXSTAbits.TXEN = 1;         // Habilitar envío de datos
    return;
}

void enviar_char(char dato)
{
    TXREG = dato;       // Enviar dato
    __delay_ms(1);     // Esperar a que se termine de enviar
    return;
}

void enviar_string(char dato[])
{
    for(int i = 0; i<strlen(dato); i++) // Ir de 0 hasta la longitud del string
    {
        TXREG = dato[i];    // Mostrar cada char del string de 1 en 1
        __delay_ms(1);     // Delay entre cada char mostrado
    }
    return;
}