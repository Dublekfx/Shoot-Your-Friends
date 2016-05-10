#include <msp430g2553.h>
#include "Laserhelper.h"
#include "ws2812.h"

#define TEAMRED 0
#define TEAMBLUE 1

#define MAX_ENG		9
#define LOW_COST	1
#define MID_COST	4
#define HIGH_COST	9

#define ENG_REG_TIME	200	// * 5 = time to regenerate 1 bar of energy in milliseconds

#define HOLD_TIME_LOW	10		// min time in ms for button press to cause low damage
#define HOLD_TIME_MID	2000	// min time in ms for button press to cause mid damage
#define HOLD_TIME_HIGH	4000	// min time in ms for button press to cause high damage

#define	LOW_PERIOD		6			//
#define MID_PERIOD		12
#define HIGH_PERIOD		24

#define LOWUP			4
#define MIDUP			8
#define HIGHUP			16
#define SHOOT_TIME		400	// time in ms each shot lasts for

unsigned int team;
unsigned int i;
int startup;

int energy = MAX_ENG;	// player energy
int energy_after = MAX_ENG;		// energy player will have after firing shot

// timers and flags and all that jazz
long globaltime = 0;
int WDTcnt = 0;
int WDTflg = 0;
int _5msflg = 0;
int shoot_timer = 0;
int shootflag = 0;
int percnt = 0;
int up_time = 0;
int period = 1;
int regen_time = 400;
int regenflg = 0;
int regencnt = 0;
int ledflg = 0;
int failflag = 0;

int history[32] = {0};
int k = 0;

// button debouncing stuff
int rawKeyState = 0;
int debouncedKeyState = 0;
int keyCount = 0;
int keyHoldtime = 0;
#define PRESS_CNT	10  //stable cycles before registering a press
#define RELEASE_CNT 	14	// stable cycles before registering a release

void debounce();
void playSong();
void ratchetPWM();

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	if (CALBC1_12MHZ == 0xFF || CALDCO_12MHZ == 0xff)
		while(1); // Erased calibration data? Trap!

	BCSCTL1 = CALBC1_12MHZ; 			//Set the DCO to 1 MHz
	DCOCTL = CALDCO_12MHZ; 			//And load calibration data

	BCSCTL3 |= LFXT1S_2;			//Use VLO for ACLK

	P2DIR |= BIT1 | BIT4; 	//BIT1 is TA1.1 PWM output, BIT4 is TA1.2 PWM output
	P2SEL |= BIT1 | BIT4;

	// Port 1 - P1.4 is laser output
	P1DIR |= BIT4;
	P1OUT |= BIT6;
	P1REN |= BIT6;

	TACTL = TASSEL_2 + MC_1 + ID_0;	//SMCLK clock source, up mode, input divider 0.

	initLaserhelper();

	team = TEAMBLUE;
	startup = 1;
	currentTune = GAMEBOYSTARTTUNE;

	const LED ledOff = (LED) {0x00, 0x00, 0x00};
	const LED ledRed = (LED) {0xFF, 0x00, 0x00};
	const LED ledBlue = (LED) {0x00, 0x00, 0xFF};
	const LED ledWhite = (LED) {0xFF, 0xFF, 0xFF};
	const LED ledYellow = (LED) {0xFF, 0xFF, 0x00};

	WDTCTL = WDT_MDLY_0_5;
	IE1 |= WDTIE;
	time = 0;

	__enable_interrupt();		//global interrupt enable

	initStrip();
	fillStrip(255, 255, 255);

	showStrip();

	while(1) {
		__bis_SR_register(CPUOFF + GIE);
		if (_5msflg){				// every 5 ms,
			_5msflg = 0;
			debounce();				// debounce button
			if (shoot_timer > 0){	// decrease shoot timer if it's above 0
				shoot_timer -= 5;	// this timer is the amount of time left in each shot
			}
			else{
				shootflag = 0;		// when shoot timer hits 0, turn off laser
				regencnt++;
			}
		}

		if(regenflg && energy < MAX_ENG){
			regenflg = 0;
			energy += 1;
			energy_after = energy;
			if(energy == MAX_ENG) currentTune = RECHARGETUNE;
		}


		for(i = energy_after; i != 0; i--) {
			switch(team) {
			case TEAMRED:
				if(!compareLEDColor1(i-1, ledRed)) {
					ledflg = 1;
					setLEDColor(i-1, 0xFF, 0x00, 0x00);
				}
				break;
			case TEAMBLUE:
				if(!compareLEDColor1(i-1, ledBlue)) {
					ledflg = 1;
					setLEDColor(i-1, 0x00, 0x00, 0xFF);
				}
				break;
			}
		}


		if(energy != energy_after) {
			for(i = energy; i != energy_after; i--) {
				switch(team) {
				case TEAMRED:
					if(!compareLEDColor1(i-1, ledYellow)) {
						ledflg = 1;
						setLEDColor(i-1, 0xFF, 0xFF, 0x00);
					}
					break;
				case TEAMBLUE:
					if(!compareLEDColor1(i-1, ledWhite)) {
						ledflg = 1;
						setLEDColor(i-1, 0xFF, 0xFF, 0xFF);
					}
					break;
				}
			}
		}


		if(energy_after < MAX_ENG) {
			for(i = MAX_ENG; i != energy; i--) {
				if(!compareLEDColor1(i-1, ledOff)) {
					ledflg = 1;
					setLEDColor(i-1, 0x00, 0x00, 0x00);
				}
			}
		}
		if(currentTune != NOTUNE) playSong();
		if(ledflg) {
			ledflg = 0;
			showStrip();
		}
	}
}

#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void) {
	time++;							// units of 1/24 ms
	percnt = (percnt + 1);

	if (percnt == period){
		percnt = 0;
	}
	WDTcnt = (WDTcnt + 1);	// cycles every 5 ms

	if (WDTcnt == 120){
		WDTcnt = 0;
	}

	if (WDTcnt == 0){
		_5msflg = 1;
		regencnt = (regencnt + 1);		// regen every ENG_REG_TIME * 5 ms
		if (regencnt == ENG_REG_TIME){
			regencnt = 0;
		}
		if (regencnt == 0){
			regenflg = 1;
		}
	}

	ratchetPWM();

	//	WDTflg = 1;
	__bic_SR_register_on_exit(CPUOFF);	// Return to main with normal power mode on
}

void debounce(){				// takes care of button states, and game states associated w/ buttons
	// get raw state from input register
	int rawKeyState = (P1IN & BIT6)>>6;
	// debounce

	if (rawKeyState == debouncedKeyState){	// no change in button state
		if (debouncedKeyState){					// button up
			keyCount = PRESS_CNT;			// reset time before debounce complete
		}
		else{									// button down
			keyCount = RELEASE_CNT;
			keyHoldtime += 5;				// reset time before debounce complete
			regencnt = 0;				// can't replenish energy when button is held down


			// as hold time increases, increase the energy cost we'll use
			// and also increase the laser period
			// (but laser will only light up after button is released, and shootflag is set)
			if (keyHoldtime > HOLD_TIME_HIGH && energy >= HIGH_COST){
				playTone((Tone) {c4, 0}, 1);
				energy_after = energy - HIGH_COST;
				period = HIGH_PERIOD;
				up_time = HIGHUP;
			}
			else if (keyHoldtime > HOLD_TIME_MID && energy >= MID_COST){
				playTone((Tone) {g3, 0}, 1);
				energy_after = energy - MID_COST;
				period = MID_PERIOD;
				up_time = MIDUP;
			}
			else if (keyHoldtime > HOLD_TIME_LOW && energy >= LOW_COST){
				playTone((Tone) {c3, 0}, 1);
				energy_after = energy - LOW_COST;
				period = LOW_PERIOD;
				up_time = LOWUP;
			}
			else{
				energy_after = energy;
				period = 1;
				up_time = 0;
				mute();
				if (energy < LOW_COST){
					failflag = 1;
				}
			}
			percnt = 0;

		}
	}
	else{
		if (--keyCount == 0){			// button state changed
			debouncedKeyState = rawKeyState;
			if (debouncedKeyState){			// button now up

				keyHoldtime = 0;				// reset time button was held
				if (energy_after >= 0){			// if enough energy,
					if(period == LOW_PERIOD) {
						currentTune = SHOOTLOWTUNE;
					}
					else if(period == MID_PERIOD) {
						currentTune = SHOOTMIDTUNE;
					}
					else if(period == HIGH_PERIOD) {
						currentTune = SHOOTHIGHTUNE;
					}
					else {
						mute();
					}

					shootflag = 1;				// turn on laser
					shoot_timer = SHOOT_TIME;	// laser will only turn on for SHOOT_TIME ms
					energy = energy_after;		// update energy
					// HEREALICE
				}
				if (failflag) {
					mute();
					currentTune = FAILTUNE;
					failflag = 0;
				}
			}
			else{								// button now down
				keyCount = RELEASE_CNT;		// reset time needed before a release is registered
			}
		}

	}
}

void ratchetPWM(){
	if (shootflag && (percnt < up_time)){	// high duty cycle part of period and shootflag = 1
		P1OUT |= BIT4;			// blink laser on
		history[k] = percnt;
	}
	else{					// else, low duty cycle
		P1OUT &= ~BIT4;		// blink laser off
		history[k] = percnt;
	}

	if (shootflag){
		k = (k + 1);
		if (k == 32){
			k = 0;
		}
		if(k == 0){
			volatile int a = 0;
		}
	}
}

void playSong() {
	Tone note;
	switch(currentTune) {
	case NOTUNE:
		return;
	case GAMEBOYSTARTTUNE:
		note = gameboyStartTune[tuneIndex];
		break;
	case SHOOTLOWTUNE:
		note = shootLowTune[tuneIndex];
		break;
	case SHOOTMIDTUNE:
		note = shootMidTune[tuneIndex];
		break;
	case SHOOTHIGHTUNE:
		note = shootHighTune[tuneIndex];
		break;
	case RECHARGETUNE:
		note = rechargeTune[tuneIndex];
		break;
	case FAILTUNE:
		note = failTune[tuneIndex];
		break;
	default:
		return;
	}
	if(TA1CCR0 == 0) {
		playTone(note, 1);
	}
	if(micros() - toneStart > note.duration) {
		tuneIndex++;
		TA1CCR0 = 0;
	}
	if(note.pitch == 0 && note.duration == 0) {
		mute();
		tuneIndex = 0;
		currentTune = NOTUNE;
		if(startup) {
			startup = 0;
			if(team == TEAMRED)
				fillStrip(0xFF, 0x00, 0x00);
			else if(team == TEAMBLUE)
				fillStrip(0x00, 0x00, 0xFF);
			ledflg = 1;
		}
		return;
	}
}
