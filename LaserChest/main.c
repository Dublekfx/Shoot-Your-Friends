#include <msp430g2553.h>
#include "Laserhelper.h"

#define NUMSPILEDS 24

#define MAX_HP		24
#define LOW_DMG		1
#define MID_DMG		8
#define HIGH_DMG	9
#define IMM_TIME	1000			// time in ms of immunity. Immunity happens immediately after getting shot.
#define PER_LOW		64		// time in us of period of modulation needed for low damage
#define PER_MID		400		// mid damage period
#define	PER_HIGH	650		// high damage period
#define TOLERANCE	75

int startup;
SpiLed spiLeds[NUMSPILEDS+2];

// Variables for finding modulation period
int sample_cnt = 0;
int sample_sum = 0;

int hp = MAX_HP;
int dmg = 0;

//int _125usflag = 0;
//int _1msflag = 0;
int immunity = 0;
int game_over = 0;

int readADCflag = 0;
int adc_reading = 0;

int k = 0;

// Timing and flag variables
int i = 0;
int WDTsubcnt = 0;		// Increases every 1/24 ms, reset every 1
int WDTcnt = 0;			// Increases every 1 ms, reset every 5
int LCDupdateflag = 0;	// flags every 5 ms
int _1msflag = 0;		// flags ever 1 ms
long globaltime = 0;	// global time in ms

int check_period_flag = 0;
int times[16] = {0};
int immunity_timer = 0;
int laser_on = 0;
int laser_flag = 0;
int base = 30;
int time_since = 0;
int laser_state = 0;
int j = 0;

int history[32] = {0};

// Function define
void readADC();				// read ADC value and update data to send to LCD
void calcPeriod();			// find period of modulation
void initialize();			// initialize base, no laser hitting adc value

void playSong();
void ledsSet(SpiLed color);
void ledsOff();

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	if (CALBC1_12MHZ == 0xFF || CALDCO_12MHZ == 0xff)
		while(1); 					//Erased calibration data? Trap!

	BCSCTL1 = CALBC1_12MHZ; 			//Set the DCO to 1 MHz
	DCOCTL = CALDCO_12MHZ; 			//And load calibration data

	BCSCTL3 |= LFXT1S_2;			//Use VLO for ACLK

	TACTL = TASSEL_2 + MC_1 + ID_0;	//SMCLK clock source, up mode, input divider 1

	P1SEL |= BIT2 | BIT4;			//Set pins for USCI A0 use
	P1SEL2 |= BIT2 | BIT4;			//UCA0TX | UCA0CLK

	UCA0CTL1 = UCSWRST;							//Disable USCI
	UCA0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC;	//Set some settings
	UCA0CTL1 |= UCSSEL_2;						//Source off SMCLK
	UCA0BR0 = 1;								//Divide SMCLK by 1, to 1 MHz
	UCA0BR1 = 0;
	UCA0CTL1 &= ~UCSWRST;						//Enable USCI

	// ADC Configuration
	// ADC10 on, turn on reference voltage, ADC sample & hold time of 16, interrupt not enabled
	ADC10CTL0 |= ADC10ON + REFON + SREF_0 + ADC10SHT_3;
	// ADC input channel P1.5, clock source SMCLK
	ADC10CTL1 |= INCH_5 + ADC10SSEL_3;
	ADC10AE0 |= BIT5;
	ADC10CTL0 |= ENC + ADC10SC;

	initLaserhelper();

	spiLeds[0] = (SpiLed) {0, 0, 0, 0};
	for(i = NUMSPILEDS; i != 0; i--) {
		spiLeds[i] = ledOff;
	}
	spiLeds[NUMSPILEDS+1] = (SpiLed) {255, 255, 255, 255};

	TA1CCR0 = UINT_MAX;


	WDTCTL = WDT_MDLY_0_5;
	IE1 |= WDTIE;
	time = 0;

	__enable_interrupt();		//global interrupt enable

	startup = 1;
	currentTune = SAMUSSTARTTUNE;
//	startup = 0;
//	currentTune = NOTUNE;

	ledsOff();
	spiLedFlush(spiLeds, NUMSPILEDS);

	initialize();

	while(1){
		__bis_SR_register(CPUOFF + GIE);
		if(_1msflag){
			if (immunity_timer == 0){
				immunity = 0;
			}
			else{
				immunity_timer -= 1;
			}
		}

		if (time_since > 24 && adc_reading == 0){
					laser_on = 0;
					k = 0;
					j = 0;
					time_since = 0;
		}

		if (!laser_on && adc_reading){	// laser is off but adc reading is a 1
			laser_on = 1;	// laser state is on
			time_since = 0;
		}


		else{
			if (check_period_flag){			// if 16 L/H or H/L swaps occured
				check_period_flag = 0;
				volatile int sum = 0;
				for (i = 0; i < 16; i ++){
					sum = sum + times[i];
				}

				// ---------------
				if (sum < (PER_LOW + TOLERANCE)){	// low dmg
					dmg = LOW_DMG;
				}

				else if (sum > (PER_HIGH - (TOLERANCE >>1))){	// high dmg
					dmg = HIGH_DMG;
				}

				else{				// mid dmg
					dmg = MID_DMG;
				}

				// ---------------

				if(game_over) {
					playSong();
					continue;
				}

				// Calibration - put stop point here, read "sum" for high, mid, low periods
				// Adjust PER_LOW, PER_MID, PER_HIGH, TOLERANCE accordingly
				if (!immunity){			// if not dmg immune
					hp -= dmg;			// decrease hp
					if (hp <= 0){
						game_over = 1;
						currentTune = MARIOLOSETUNE;
					}
					if (dmg > 0 && !game_over){
						currentTune = DAMAGETUNE;
						immunity = 1;		// become immune
						immunity_timer = IMM_TIME;	// for IMM_TIME ms
					}
				}

			}	////////////////// if(check_period_flag)

			else{						//	if there is no laser
				dmg = 0;				// then no dmg
			}

		}
		if(!startup) {
			ledsOff();
			switch(hp) {
			case 24:
				spiLeds[11] = white;
			case 23:
				spiLeds[12] = white;
			case 22:
				spiLeds[13] = white;
			case 21:
				spiLeds[14] = white;
			case 20:
				spiLeds[15] = white;
			case 19:
				spiLeds[16] = white;
			case 18:
				spiLeds[17] = white;
			case 17:
				spiLeds[18] = white;
			case 16:
				spiLeds[19] = white;
			case 15:
				spiLeds[20] = white;
			case 14:
				spiLeds[21] = white;
			case 13:
				spiLeds[22] = white;
			case 12:
				spiLeds[23] = white;
			case 11:
				spiLeds[24] = white;
			case 10:
				spiLeds[1] = white;
			case 9:
				spiLeds[2] = white;
			case 8:
				spiLeds[3] = white;
			case 7:
				spiLeds[4] = white;
			case 6:
				spiLeds[5] = white;
			case 5:
				spiLeds[6] = white;
			case 4:
				spiLeds[7] = white;
			case 3:
				spiLeds[8] = white;
			case 2:
				spiLeds[9] = white;
			case 1:
				spiLeds[10] = white;
			}
			spiLedFlush(spiLeds, NUMSPILEDS);
		}
		if(currentTune != NOTUNE) playSong();
	}
}

#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void) {
	readADC();
	calcPeriod();

	time++;		// units of 1/24 ms

	// subcount
	//	WDTsubcnt = (WDTsubcnt + 1) % 192;	// cycles every 1.024 ms
	WDTsubcnt = (WDTsubcnt + 1);	// cycles every 1.024 ms
	if(WDTsubcnt == 24){
		WDTsubcnt = 0;
	}

	// 1 ms flag
	if (WDTsubcnt == 0){	//every 1.024 ms
		_1msflag = 1;
	}
	__bic_SR_register_on_exit(CPUOFF);	// Return to main with normal power mode on
}

void readADC(){
	adc_reading = ADC10MEM;				// read ADC value
	ADC10CTL0 |= ENC + ADC10SC;			// enable conversions and start conversion
}

void calcPeriod(){
	// looks for changes in light input, finds time between those changes
	// turn laser off if it's been too long since changes detected
	k = (k+ 1);
	if(k == 32){
		k = 0;
	}
	history[k] = adc_reading;

//	volatile int temp = adc_reading;

	// Hysteresis
	if (laser_state){					// If laser is already on
		adc_reading = (adc_reading > base + 15) ? 1:0;
	}
	else {								// If laser is not on already, requires a higher adc value to activate
		adc_reading = (adc_reading > base + 25) ? 1:0;
	}
//	adc_reading = (adc_reading > base + 25) ? 1 : 0;		// convert from analog to 1/0

	if(laser_on){									// only care if being hit by laser
		time_since += 1;
		if (laser_state != adc_reading){			// if pulse switched
			times[j] = time_since + 1;					// insert half period time into an array
			time_since = 0;							// reset time since last swap

			j = (j + 1);						// increment number of switches. every 16 switches,
			if (j == 16){
				j = 0;
				check_period_flag = 1;
			}

		}
	}

	laser_state = adc_reading;					// update laser_state to current digital reading

}

void initialize(){
	int sum = 0;
	for (i = 0; i < 32 ; i++){
		__delay_cycles(100000);
		readADCflag = 0;
		readADC();
		sum = sum + adc_reading;
	}

	base = sum >> 5;
}

void ledsSet(SpiLed color) {
	for(i = NUMSPILEDS; i != 0; i--) {
		spiLeds[i] = color;
	}
	spiLedFlush(spiLeds, NUMSPILEDS);
}

void ledsOff() {
	for(i = NUMSPILEDS; i != 0; i--) {
		spiLeds[i] = ledOff;
	}
}

void playSong() {
	Tone note;
	switch(currentTune) {
	case NOTUNE:
		return;
	case MARIOLOSETUNE:
		note = marioLoseTune[tuneIndex];
		break;
	case DAMAGETUNE:
		note = damageTune[tuneIndex];
		break;
	case SAMUSSTARTTUNE:
		if(tuneIndex == 0) {
			note = (Tone) {d5, sixFiftyNote};
		}
		else if(tuneIndex == 1) {
			note = (Tone) {f5, sixFiftyNote};
		}
		else if(tuneIndex == 2) {
			note = (Tone) {d5, sixFiftyNote};
		}
		else if(tuneIndex == 3) {
			note = (Tone) {c5, sixFiftyNote};
		}
		else if(tuneIndex == 4) {
			note = (Tone) {a4, 24000};
		}
		else if(tuneIndex == 5) {
			note = (Tone) {rest, 0};
		}
		break;
	default:
		return;
	}
	if(TA1CCR0 == UINT_MAX || (TA1CCR0 == UINT_MAX && tuneIndex == 0)) {
		playTone(note, 1);
	}
	if(micros() - toneStart > note.duration) {
		tuneIndex++;
		TA1CCR0 = UINT_MAX;
	}
	if(note.pitch == 0 && note.duration == 0) {
		mute();
		tuneIndex = 0;
		currentTune = NOTUNE;
		TA1CCR0 = UINT_MAX;
		return;
	}
	if(startup) {
		switch(tuneIndex) {
		case 0:
			spiLeds[21] = white;
			spiLeds[22] = white;
			spiLeds[23] = white;
			break;
		case 1:
			spiLeds[18] = white;
			spiLeds[19] = white;
			spiLeds[20] = white;
			spiLeds[24] = white;
			spiLeds[1] = white;
			spiLeds[2] = white;
			break;
		case 2:
			spiLeds[15] = white;
			spiLeds[16] = white;
			spiLeds[17] = white;
			spiLeds[3] = white;
			spiLeds[4] = white;
			spiLeds[5] = white;
			break;
		case 3:
			spiLeds[12] = white;
			spiLeds[13] = white;
			spiLeds[14] = white;
			spiLeds[6] = white;
			spiLeds[7] = white;
			spiLeds[8] = white;
			break;
		case 4:
			spiLeds[9] = white;
			spiLeds[10] = white;
			spiLeds[11] = white;
			startup = 0;
			break;
		}
		spiLedFlush(spiLeds, NUMSPILEDS);
	}
}
