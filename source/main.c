/*	Author: Jack Huang
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #8  Exercise 2
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
enum states {smstart,initial,start,wait} state;

double freq[] = {261.63,0,261.63,0,293.66,293.66,0,261.63,261.63,0,349.23,349.23,0,329.63,329.63,329.63,0,261.63,0,261.63,0,293.66,293.66,0,261.63,261.63,0,392,392,0,349.23,349.23,349.23,0,261.63,0,261.63,0,524,524,0,440,440,0,349.23,349.23,0,329.63,329.63,0,293.66,293.66,0,466.16,0,466.16,0,440,440,0,349.23,349.23,0,392,392,0,349.23,349.23,349.23,349.23,0};
unsigned char i=0;

void Tick(){
	switch(state){
		case smstart:
			state = initial;
			break;
		case initial:
			if((~PINA & 0x07) == 0x04){
				state = start;
			}
			else{
				state = initial;
			}
			break;
		case start:
			if(i<70){
				state = start;
			}
			else{
				state = wait;
			}
			break;
		case wait:
			if((~PINA & 0x07) == 0x04){
				state = wait;
			}
			else{
				state = initial;
			}
			break;
	}
	switch(state){
		case smstart:
			break;
		case initial:
			set_PWM(0);
			i=0;
			break;
		case start:
			set_PWM(freq[i]);
			i++;
			break;
		case wait:
			set_PWM(0);
			break;
	}
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
    PWM_on();
    TimerSet(170);
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
