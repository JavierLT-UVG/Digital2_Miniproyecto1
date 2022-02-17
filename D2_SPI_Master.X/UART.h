/* 
 * File: ADC.h
 * Author: Francisco Javier López Turcios
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef USART_H
#define	USART_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include <string.h>
#define _XTAL_FREQ 4000000

void config_tx(void);               // Configuración del USART
void enviar_char(char dato);        // Enviar chars
void enviar_string(char dato[]);    // Enviar strings

#endif	

