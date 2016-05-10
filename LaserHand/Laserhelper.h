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
#define a2 110
#define b2 123
#define c3 131
#define d3 147
#define e3 165
#define f3 175
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
#define d6 1175
#define e6 1319
#define f6 1397
#define g6 1568
#define c7 2093

#define UINT_MAX  0xFFFF
#define ULONG_MAX 0xFFFFFFFF

#define NOTUNE -1
#define GAMEBOYSTARTTUNE 0
#define SHOOTLOWTUNE 1
#define SHOOTMIDTUNE 2
#define SHOOTHIGHTUNE 3
#define RECHARGETUNE 4
#define FAILTUNE 5

typedef struct toneStruct {
	unsigned int pitch;
	unsigned long duration;
} Tone;

unsigned long time;

unsigned long toneStart;
int currentTune;
unsigned int tuneIndex;

Tone gameboyStartTune[3];
Tone shootLowTune[4];
Tone shootMidTune[4];
Tone shootHighTune[4];
Tone rechargeTune[3];
Tone failTune[3];

void initLaserhelper();

unsigned long micros();

Tone initTone(unsigned int pitch, unsigned long duration);

void playTone(Tone tone, int volume);
void mute();

#endif /* LASERHELPER_H_ */
