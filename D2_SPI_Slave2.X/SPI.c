#include "SPI.h"

void spi_Master(void)
{
    SSPSTATbits.SMP = 1;        // Input data sampled at end of data output time
    SSPSTATbits.CKE = 0;        // Data transmitted on rising edge of SCK
    
    SSPCONbits.SSPEN = 1;       // Enables serial port and configures SCK, SDO, SDI and SS as the source of the serial port pins
    SSPCONbits.CKP = 0;         // Idle state for clock is a low level
    SSPCONbits.SSPM = 0b0000;   // SPI Master mode, clock = FOSC/4
    return;
}
void spi_Slave(void)
{
    SSPSTATbits.SMP = 0;        // SMP must be cleared when SPI is used in Slave mode
    SSPSTATbits.CKE = 0;        // Data transmitted on rising edge of SCK
    
    SSPCONbits.SSPEN = 1;       // Enables serial port and configures SCK, SDO, SDI and SS as the source of the serial port pins
    SSPCONbits.CKP = 0;         // Idle state for clock is a low level
    SSPCONbits.SSPM = 0b0100;   // SPI Slave mode, clock = SCK pin, SS pin control enabled
    
    PIE1bits.SSPIE = 1;         // Activar interrupciones MSSP
    PIR1bits.SSPIF = 0;         // Limpiar bandera de interrupciones MSSP
    return;
}

unsigned char spi_Read(void)
{
    while (!SSPSTATbits.BF);    // Mientras que no haya terminado de recibir, no avanzar
    return SSPBUF;              // Al terminar de recibir, enviar registro
}