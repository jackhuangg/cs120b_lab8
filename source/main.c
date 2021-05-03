/*	Author: Jack Huang
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #8  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *      https://drive.google.com/drive/folders/1JBIqqJb-m900203LVLXI8yLaMciH493w?usp=sharing
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

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
enum states {initial,start,waitincrease,waitdecrease,waitoff} state;

double freq[8] = {261.63, 293.66, 329.64, 349.23, 392, 440, 493.88, 523.25};
unsigned char index=0;

void Tick(){
	switch(state){
		case initial:
			if((~PINA & 0x04) == 0x04){
				state = start;
			}
			else{
				state = initial;
			}
			break;
		case start:
			if((~PINA & 0x01) == 0x01){
				state = waitincrease;
			}
			else if((~PINA & 0x02) == 0x02){
				state = waitdecrease;
			}
			else if((~PINA & 0x04) == 0x04){
				state = waitoff;
			}
			else{
				state = start;
			}
			break;
		case waitincrease:
			if((~PINA & 0x01) == 0x01){
				state = waitincrease;
			}
			else{
				state = start;	
			}
			break;
		case waitdecrease:
			if((~PINA & 0x02) == 0x02){
				state = waitdecrease;
			}
			else{
				state = start;
			}
			break;
		case waitoff:
			if((~PINA & 0x04) == 0x04){
				state = waitoff;
			}
			else{
				state = initial;
			}
			break;
	}
	switch(state){
		case initial:
			index=4;
			break;
		case start:
			PWM_on();
			set_PWM(freq[index]);
			break;
		case waitincrease:
			if(index<7){
				index++;
			}
			set_PWM(freq[index]);
			break;
		case waitdecrease:
			if(index>0){
				index--;
			}
			set_PWM(freq[index]);
			break;
		case waitoff:
			PWM_off();
			break;
	}
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
    PWM_on();
    state = initial;
    while (1) {
	Tick();
    }
    PWM_off();
    return 1;
}
