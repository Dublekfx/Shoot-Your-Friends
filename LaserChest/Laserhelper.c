#include <msp430g2553.h>
#include "Laserhelper.h"

/*
 * Laserhelper.c
 *
 *  Created on: Apr 29, 2016
 *      Author: Keiko
 */

void initLaserhelper() {
	white = (SpiLed) {0xE1, 255, 255, 255};
	ledOff = (SpiLed) {0xE1, 0, 0, 0};

	currentTune = NOTUNE;
	tuneIndex = 0;
	//Mario Lose Tune Array
	marioLoseTune[0] = (Tone) {b4, oneFiftyNote};
	marioLoseTune[1] = (Tone) {f4, oneFiftyNote};
	marioLoseTune[2] = (Tone) {rest, oneFiftyNote};
	marioLoseTune[3] = (Tone) {f4, oneFiftyNote};

	marioLoseTune[4] = (Tone) {f4, twoHundredNote};
	marioLoseTune[5] = (Tone) {e4, twoHundredNote};
	marioLoseTune[6] = (Tone) {d4, twoHundredNote};

	marioLoseTune[7] = (Tone) {c4, oneFiftyNote};
	marioLoseTune[8] = (Tone) {e3, oneFiftyNote};
	marioLoseTune[9] = (Tone) {rest, oneFiftyNote};
	marioLoseTune[10] = (Tone) {e3, oneFiftyNote};

	marioLoseTune[11] = (Tone) {c3, oneFiftyNote};
	marioLoseTune[12] = (Tone) {rest, 0};

	damageTune[0] = (Tone) {c3, 12000};
	damageTune[1] = (Tone) {rest, 0};
}

unsigned long micros() {
	return time;
}

void spiLedSend(SpiLed *array, int numLeds) {
	unsigned int spiLedMessageLength = 4 + (numLeds << 2) + 4;

	unsigned char *ledAddr = ((unsigned char *) array);
	ledAddr += spiLedIndex;

	if(IFG2 & UCA0TXIFG) {
		UCA0TXBUF = *((unsigned char *) ledAddr);
	}

	spiLedIndex = spiLedIndex < spiLedMessageLength-1 ? spiLedIndex+1 : 0;
}

void spiLedFlush(SpiLed *array, int numLeds) {
	unsigned int spiLedMessageLength = 4 + (numLeds << 2) + 4;

	int count = spiLedMessageLength*2;

	while(count--) {
		unsigned char *ledAddr = ((unsigned char *) array);
		ledAddr += spiLedIndex;

		if(IFG2 & UCA0TXIFG) {
			UCA0TXBUF = *((unsigned char *) ledAddr);
		}

		spiLedIndex = spiLedIndex < spiLedMessageLength-1 ? spiLedIndex+1 : 0;
	}
}

void playTone(Tone tone, int volume) {
	if(tone.pitch) {
		unsigned int pitch = (1500000/tone.pitch);
		P2DIR |= BIT1 | BIT4; 	//BIT1 is TA1.1 PWM output, BIT4 is TA1.2 PWM output
		P2SEL |= BIT1 | BIT4;
		TA1CCR0 = pitch - 1;
		TA1CCR1 = pitch >> (volume+1); 	// divde by 2
		TA1CCR2 = pitch >> (volume+1);
		TA1CCTL1 = OUTMOD_7;
		TA1CCTL2 = OUTMOD_3;
		TA1CTL |= TASSEL_2 + ID_3 + MC_1; 	// SMCLK, upmode
	}
	else {							//Rest
		mute();
	}
	toneStart = tone.duration ? micros() : ULONG_MAX;
}

void mute() {
	P2DIR &= ~(BIT1 | BIT4); 	//BIT1 is TA1.1 PWM output, BIT4 is TA1.2 PWM output
	P2SEL &= ~(BIT1 | BIT4);
	TA1CCR0 = 0;
	TA1CCR1 = 0;
	TA1CCR2 = 0;
	TA1CTL |= MC_0;
}
