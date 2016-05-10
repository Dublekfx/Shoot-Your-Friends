#include <msp430g2553.h>
#include "Laserhelper.h"
#include "ws2812.h"
#include <stdlib.h>

/*
 * Laserhelper.c
 *
 *  Created on: Apr 29, 2016
 *      Author: Keiko
 */

void initLaserhelper() {
	currentTune = NOTUNE;
	tuneIndex = 0;

	gameboyStartTune[0] = (Tone) {c6, 2400};
	gameboyStartTune[1] = (Tone) {c7, 10800};
	gameboyStartTune[2] = (Tone) {rest, 0};

	shootLowTune[0] = (Tone) {d3, 1800};
	shootLowTune[1] = (Tone) {e3, 1800};
	shootLowTune[2] = (Tone) {f3, 1800};
	shootLowTune[3] = (Tone) {rest, 0};

	shootMidTune[0] = (Tone) {g3, 1800};
	shootMidTune[1] = (Tone) {a3, 1800};
	shootMidTune[2] = (Tone) {b3, 1800};
	shootMidTune[3] = (Tone) {rest, 0};

	shootHighTune[0] = (Tone) {d4, 1800};
	shootHighTune[1] = (Tone) {e4, 1800};
	shootHighTune[2] = (Tone) {f4, 1800};
	shootHighTune[3] = (Tone) {rest, 0};

	rechargeTune[0] = (Tone) {b4, 1200};
	rechargeTune[1] = (Tone) {e5, 3600};
	rechargeTune[2] = (Tone) {rest, 0};

	failTune[0] = (Tone) {g4, 2400};
	failTune[1] = (Tone) {c4, 6000};
	failTune[2] = (Tone) {rest, 0};
}

unsigned long micros() {
	//	return time * 16.0 / 3.0;
	return time;
}

Tone initTone(unsigned int pitch, unsigned long duration) {
	Tone *t = malloc(sizeof(Tone));

	t->pitch = pitch;
	t->duration = duration * 3000 >> 4;

	return *t;
}

void playTone(Tone tone, int volume) {
	if(tone.pitch) {
		unsigned int pitch = (1500000/tone.pitch);
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
	TA1CCR0 = 0;
	TA1CCR1 = 0;
	TA1CCR2 = 0;
	TA1CTL |= MC_0;
	currentTune = NOTUNE;
}
