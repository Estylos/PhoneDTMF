#include <avr/io.h>
#include <util/setbaud.h>
#include "simpleUsartTx.h"

void initUSART(void) { 
                                   
    UBRR0H = UBRRH_VALUE;                   // Set the baudarte (macros defined in setbaud.h)
    UBRR0L = UBRRL_VALUE;
    #if USE_2X
        UCSR0A |= (1 << U2X0);
    #else
        UCSR0A &= ~(1 << U2X0);
    #endif

    UCSR0B = (1 << TXEN0);                  // Enable USART transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set character size in 8 bits
}

void sendByte(uint8_t data) {

    while ( !(UCSR0A & (1 << UDRE0)) );     // Wait until the transmission buffer is ready to receive new data
    UDR0 = data; 
}