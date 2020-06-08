#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <math.h>
#include "simpleUsartTx.h"

#define PHONE_PIN           PC0
#define BLOCK_SIZE          100
#define DTMF_ROWS           4
#define DTMF_COLUMNS        3
#define DTMF_FREQUENCIES    DTMF_COLUMNS + DTMF_ROWS
#define THRESHOLD           700


const float dtmfCoef[DTMF_FREQUENCIES] = {
    1.809654104932039,  	// 697 Hz
    1.7526133600877272,	    // 770 Hz
    1.6886558510040302,	    // 852 Hz
    1.618033988749895,	    // 941 Hz
    1.3690942118573772,	    // 1209 Hz
    1.2748479794973793,	    // 1336 Hz
    1.175570504584946	    // 1477 Hz
};

const uint8_t phoneKeys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
}; 

volatile uint8_t count = 0;
volatile int16_t adcSamples[BLOCK_SIZE];

uint8_t previousKey; 


ISR(ADC_vect) { // When conversion finished

    adcSamples[count++] = ADC - 468;
}

void initADC(void) {
	
	ADMUX |= (1 << REFS0);                  // Reference voltage on AVCC
    ADMUX |= PHONE_PIN;                     // Set the analog input
    ADCSRA |= (1 << ADPS1) | (1 << ADPS2);  // ADC clock prescaler /64 (125 kHz with F_CPU = 8 MHz and ADC need 13 cycles so sample rate = F/125/13 = 9615 kHz)
	ADCSRA |= (1 << ADEN);                  // Enable ADC
    ADCSRA |= (1 << ADIE);                  // ADC Interrupt Enable
	ADCSRA |= (1 << ADATE);                 // ADC auto-trigger enable
	ADCSRA |= (1 << ADSC);                  // ADC start conversion
}

void getDtmfMagnitudes(volatile int16_t *samples, uint16_t *magnitudes) {

    /*  Here we use the goertzel algorithm to detect the magnitudes of the DTMF signals present in the samples that we collected
     *  For more information on the algorithm and its application check :
     *      - https://en.wikipedia.org/wiki/Goertzel_algorithm
     *      - https://www.embedded.com/the-goertzel-algorithm/
     */

    for (uint8_t toneIndex = 0 ; toneIndex < DTMF_FREQUENCIES ; toneIndex++) { // For all DTMF
         
        float coef = dtmfCoef[toneIndex];
        float q0 = 0, q1 = 0, q2 = 0;

        for (uint8_t sampleIndex = 0 ; sampleIndex < BLOCK_SIZE ; sampleIndex++) { // For all samples

            q0 = (float)(samples[sampleIndex]) + coef * q1 - q2;
            q2 = q1;
            q1 = q0;
        }

        float magnitude = sqrt( q1 * q1 + q2 * q2 - q1 * q2 * coef );
        magnitudes[toneIndex] = magnitude;
    }
}

uint8_t getLargestMagnitudeIndex(uint16_t *magnitudes, uint8_t len) {

    uint16_t largestMagnitude = 0;
    uint8_t largestMagnitudeIndex = 0;

    for (uint8_t magnitudeIndex = 0 ; magnitudeIndex < len ; magnitudeIndex++) {

        uint16_t magnitude = magnitudes[magnitudeIndex];
        if (largestMagnitude < magnitude) {
            
            largestMagnitude = magnitude;
            largestMagnitudeIndex = magnitudeIndex;
        }
    }

    return largestMagnitudeIndex;
}

uint8_t detectFalsePositive(uint16_t *magnitudes) { // To avoid returning false data. Can be improved...
    
    for (uint8_t magnitudeIndex = 0 ; magnitudeIndex < DTMF_FREQUENCIES ; magnitudeIndex++) { // To detect only high magnitudes
        if (magnitudes[magnitudeIndex] > THRESHOLD)
            return 1;
    }

    return 0;
}


int main(void) {

    clock_prescale_set(clock_div_1); // CPU clock to 8 MHz

	initUSART();
	initADC(); 

    sei(); // Set the global interrupt flag (interrupts ON)

	while (1) {

        if (count >= BLOCK_SIZE) {

            cli(); // Clear the global interrupt flag (interrupts OFF)

            uint16_t magnitudes[DTMF_FREQUENCIES]; // Magnitudes for all frequencies (from 697 Hz to 1477 Hz)
            /*              +--------+--------+--------+--------+---------+---------+---------+
             *  magnitudes: | m697Hz | m770Hz | m852Hz | m941Hz | m1209Hz | m1336Hz | m1477Hz |   uint16_t[7]
             *              +--------+--------+--------+--------+---------+---------+---------+
             *
             *              |----------------------------------| |----------------------------| 
             *                              rows                            column
             */


            getDtmfMagnitudes(adcSamples, magnitudes);
            
            uint8_t dtmfRowIndex = getLargestMagnitudeIndex(magnitudes, DTMF_ROWS); // From 697 Hz to 941 Hz (rows)
            uint8_t dtmfColIndex = getLargestMagnitudeIndex(magnitudes + DTMF_ROWS, DTMF_COLUMNS); // From 1209 Hz to 1477 Hz (cols)

            uint8_t key = phoneKeys[dtmfRowIndex][dtmfColIndex];

            if (key != previousKey) {
                if (detectFalsePositive(magnitudes)) {
                    sendByte(key); // Print the corresponding phone key
                }
            }

            previousKey = key;

            count = 0;
            sei();
        }
	}
	
	return 0;
}