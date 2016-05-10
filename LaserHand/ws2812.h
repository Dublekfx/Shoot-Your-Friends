/*
 * ws2812.h
 *
 * From https://github.com/mjmeli/MSP430-NeoPixel-WS2812-Library
 *
 */

#ifndef LASERHELPER_WS2812_H_
#define LASERHELPER_WS2812_H_

// Useful typedefs
typedef unsigned char u_char;		// 8 bit
typedef unsigned int u_int;			// 16 bit

// Transmit codes
#define HIGH_CODE	(0xF0)			// b11110000
#define LOW_CODE	(0xC0)			// b11000000

typedef struct ledStruct {
	u_char red;
	u_char green;
	u_char blue;
} LED;

// Configure processor to output to data strip
void initStrip(void);

// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip(void);

// Set the color of a certain LED
void setLEDColor(u_int p, u_char r, u_char g, u_char b);

int compareLEDColor(u_int p, u_char r, u_char g, u_char b);
int compareLEDColor1(u_int p, LED color);

// Clear the color of all LEDs (make them black/off)
void clearStrip(void);

// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b);

void gradualFill(u_int n, u_char r, u_char g, u_char b);

#endif /* LASERHELPER_WS2812_H_ */
