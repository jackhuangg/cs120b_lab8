/*	Author: Jack Huang
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #8  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *      https://drive.google.com/drive/folders/1JBIqqJb-m900203LVLXI8yLaMciH493w?usp=sharing
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if(!frequency)
            TCCR3B &= 0x08;
        else
            TCCR3B |= 0x03;

        if(frequency < 0.954)
            OCR3A = 0xFFFF;
        else if (frequency > 31250)
            OCR3A = 0x0000;
        else
            OCR3A = (short) (8000000 / (128 * frequency)) - 1;
        
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}
enum states {smstart,initial,start,waitincrease,waitdecrease,waitoff,waitdepress} state;

double freq[] = {261.63, 293.66, 329.64, 349.23, 392, 440, 493.88, 523.25};
unsigned char i=0;

void Tick(){
	switch(state){
		case smstart:
			state = initial;
		case initial:
			if((~PINA & 0x07) == 0x04){
				state = start;
			}
			else{
				state = initial;
			}
			break;
		case start:
			if((~PINA & 0x07) == 0x01){
				state = waitincrease;
			}
			else if((~PINA & 0x07) == 0x02){
				state = waitdecrease;
			}
			else if((~PINA & 0x07) == 0x04){
				state = waitoff;
			}
			else{
				state = start;
			}
			break;
		case waitincrease:
			if((~PINA & 0x07) == 0x01){
				state = waitdepress;
			}
			else{
				state = start;	
			}
			break;
		case waitdecrease:
			if((~PINA & 0x07) == 0x02){
				state = waitdepress;
			}
			else{
				state = start;
			}
			break;
		case waitoff:
			if((~PINA & 0x07) == 0x04){
				state = waitoff;
			}
			else if((~PINA & 0x07) == 0x00){
				state = initial;
			}
			break;
		case waitdepress:
			if((~PINA & 0x07) == 0x00){
				state = start;
			}
			else{
				state = waitdepress;
			}
	}
	switch(state){
		case smstart:
			break;
		case initial:
			i=1;
			break;
		case start:
			set_PWM(freq[i]);
			break;
		case waitincrease:
			if(i<7){
				i++;
			}
			set_PWM(freq[i]);
			break;
		case waitdecrease:
			if(i>0){
				i--;
			}
			set_PWM(freq[i]);
			break;
		case waitoff:
			set_PWM(0);
			break;
		case waitdepress:
			break;
	}
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
    PWM_on();
    TimerSet(300);
    TimerOn();
    state = smstart;
    while (1) {
	Tick();
	while(!TimerFlag);
	TimerFlag=0;
    }
    PWM_off();
    return 1;
}
