/* 
 * File: ADC.h
 * Author: Francisco Javier López Turcios
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SPI_H
#define	SPI_H

#include <xc.h> // include processor files - each processor file is guarded.  
#define _XTAL_FREQ 4000000

void spi_Master(void);
void spi_Slave(void);
unsigned char spi_Read(void);


#endif	