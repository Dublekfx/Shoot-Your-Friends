/*
 * Laserhelper.h
 *
 *  Created on: Apr 29, 2016
 *      Author: Keiko
 */

//#include <float.h>

#ifndef LASERHELPER_H_
#define LASERHELPER_H_

#define rest 0
#define c3 131
#define e3 165
#define g3 196
#define a3 220
#define b3 247
#define c4 261
#define d4 294
#define dS4 311
#define e4 329
#define f4 349
#define fS4 370
#define g4 392
#define gS4 415
#define a4 440
#define aS4 466
#define b4 494
#define c5 523
#define cS5 554
#define d5 587
#define dS5 622
#define e5 659
#define f5 698
#define fS5 740
#define g5 784
#define gS5 831
#define a5 880
#define b5 988
#define c6 1047
#define g6 1568
#define c7 2093

#define oneFiftyNote 3600		//150 ms
#define twoHundredNote 4800		//200 ms
#define sixFiftyNote 15600		//650 ms

#define UINT_MAX  0xFFFF
#define ULONG_MAX 0xFFFFFFFF

#define NOTUNE -1
#define MARIOLOSETUNE 0
#define SAMUSSTARTTUNE 1
#define DAMAGETUNE 2

typedef struct ledStruct {
	unsigned char brightness;
	unsigned char blue;
	unsigned char green;
	unsigned char red;
} SpiLed;

typedef struct toneStruct {
	unsigned int pitch;
	unsigned long duration;
} Tone;

unsigned long time;

SpiLed white;
SpiLed ledOff;

unsigned long toneStart;
int currentTune;
unsigned int tuneIndex;

unsigned int spiLedIndex;

Tone marioLoseTune[13];
Tone samusStartTune[6];
Tone damageTune[2];

void initLaserhelper();

unsigned long micros();

void spiLedSend(SpiLed *array, int numLeds);
void spiLedFlush(SpiLed *array, int numLeds);

void playTone(Tone tone, int volume);
void mute();

#endif /* LASERHELPER_H_ */
