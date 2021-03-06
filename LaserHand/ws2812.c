#include <msp430g2553.h>
#include "ws2812.h"

#define OUTPUT_PIN	BIT2			// Set to whatever UCB0SIMO is on your processor (Px.7 here)

#define NUM_LEDS	(9)			// NUMBER OF LEDS IN YOUR STRIP

LED leds[NUM_LEDS] = {{0,0,0}};

// Initializes everything needed to use this library. This clears the strip.
void initStrip(){
	P1SEL |= OUTPUT_PIN;			// configure output pin as SPI output
	P1SEL2 |= OUTPUT_PIN;
	UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; 	// 3-pin, MSB, 8-bit SPI master
	UCA0CTL1 |= UCSSEL_2;			// SMCLK source (16 MHz)
	UCA0BR0 = 2;					// 16 MHz / 3 = .1875 us per bit
	UCA0BR1 = 0;
	UCA0CTL1 &= ~UCSWRST;			// Initialize USCI state machine

	clearStrip();					// clear the strip
}

// Sets the color of a certain LED (0 indexed)
void setLEDColor(u_int p, u_char r, u_char g, u_char b){
	leds[p].red = r;
	leds[p].green = g;
	leds[p].blue = b;
}

int compareLEDColor(u_int p, u_char r, u_char g, u_char b) {
	int ret = 1;
	if(leds[p].red != r) ret =  0;
	if(leds[p].green != g) ret = 0;
	if(leds[p].blue != b) ret = 0;
	return ret;
}

int compareLEDColor1(u_int p, LED color) {
	int ret = 1;
	if(leds[p].red != color.red) ret =  0;
	if(leds[p].green != color.green) ret = 0;
	if(leds[p].blue != color.blue) ret = 0;
	return ret;
}

// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip(){
	__bic_SR_register(GIE);       	// disable interrupts

	// send RGB color for every LED
	int i, j;
	for (i = 0; i < NUM_LEDS; i++){
		u_char rgb[3] = {leds[i].green, leds[i].red, leds[i].blue};	// get RGB color for this LED

		// send green, then red, then blue
		for (j = 0; j < 3; j++){
			u_char mask = 0x80;					// b1000000

			// check each of the 8 bits
			while(mask != 0){
				while (!(IFG2 & UCA0TXIFG));	// wait to transmit

				if (rgb[j] & mask){			// most significant bit first
					UCA0TXBUF = HIGH_CODE;		// send 1
				}
				else {
					UCA0TXBUF = LOW_CODE;		// send 0
				}

				mask >>= 1;						// check next bit
			}
		}
	}

	// send RES code for at least 50 us (800 cycles at 16 MHz)
	_delay_cycles(800);

	__bis_SR_register(GIE);       	// enable interrupts
}

// Clear the color of all LEDs (make them black/off)
void clearStrip(){
	fillStrip(0x00, 0x00, 0x00);	// black
}

// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b){
	int i;
	for (i = 0; i < NUM_LEDS; i++){
		setLEDColor(i, r, g, b);		// set all LEDs to specified color
	}
	showStrip();						// refresh strip
}

void gradualFill(u_int n, u_char r, u_char g, u_char b) {
	int i;
	for (i = 0; i < n; i++){			// n is number of LEDs
		setLEDColor(i, r, g, b);
		showStrip();
		_delay_cycles(50000);			// lazy delay
	}
}
