/*
 * Schedular_DC_PWR.c
 *
 * Created: 06-Aug-22 9:35:45 PM
 * Author : admin
 */ 

#undef	F_CPU
#define F_CPU		16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uartnaval.h"
#include "uartnaval.c"

// below variables for task scheduling at given time

volatile int flag_10ms=0;
volatile int flag_100ms=0;
volatile int flag_500ms=0;

// Timer0 ISR for setting task flag

ISR(TIMER0_OVF_vect)
{
	static	int count_100ms=0;
	static	int count_10ms=0;
	static	int count_500ms=0;
	
	if(count_100ms == 100)
	{
		flag_100ms = 1;
		count_100ms = 0;
	}
	
	if(count_10ms == 10)
	{
		flag_10ms = 1;
		count_10ms = 0;
	}
	
	if(count_500ms == 500)
	{
		flag_500ms = 1;
		count_500ms = 0;
	}
	
	count_500ms++;
	count_100ms++;
	count_10ms++;
	TCNT0 = 240;		// COUNT FOR 1 MS
}

void timer0_init(void)
{
	TCCR0 = 0X05;			//	PRESCALAR AS 1024
	TCNT0 = 240;			//	COUNT FOR 1 MS
	TIMSK |= (1 << TOIE0);  //	Unmask Timer 0 overflow interrupt.

}

int main(void)
{
	/* Replace with your application code */
	
	DDRC = 0XFF;
	PORTC = 0X00;
	DDRB = 0XFF;
	PORTB = 0X00;
	
	uart_init();			// UART INITIALIZE AT 9600 AS DEFAULT BAUD RATE
	
	timer0_init();

	sei();
	
	for(;;)
	
	if(flag_10ms == 1)
	{
		PORTC=PORTC ^0XFF;
		uart_txstr("inside 10 ms\r");
		flag_10ms = 0;
		
	}
	
	else if(flag_100ms == 1)
	{
		PORTB=PORTB ^0XFF;
		uart_txstr("inside 100 ms\r");
		flag_100ms = 0;
	}
	
	else if(flag_500ms == 1)
	{
		uart_txstr("inside 500 ms\r");
		flag_500ms = 0;
	}
	
	
}

