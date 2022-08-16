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
#include <stdio.h>
#include <string.h>
#include "uartnaval.h"
#include "uartnaval.c"
#include "adcatmega16.h"
#include "adcatmega16.c"
#include "lcdgdheerajat8.h"
#include "lcdgdheerajat8.c"

#define ADC_RAW_DIV			512.0		// 512 for 10 bit adc, 128 for 8 bit adc
#define ADC_REAL_MUL		(5/(ADC_RAW_DIV*2))
//#define MV_PER_AMP		0.2			// for ACS712 5A module
//#define MV_PER_AMP		0.1			// for ACS712 20A module
#define MV_PER_AMP			0.066		// for ACS712 30A module
#define ADC_SAMPLE_CNT		50
#define VOLTAGE_MAX_READ	12.0
#define ADC_MAX_V_RAW		4.92
#define ADC_REAL_V_MUL		(VOLTAGE_MAX_READ/ADC_MAX_V_RAW)
#define DCWATTHR			(BatVdcVal*CurrentVal)

// below variables for task scheduling at given time

volatile int flag_10ms=0; /* Task Flag 1 for reading ADC values and Calculating real values */
volatile int flag_100ms=0; /* Task Flag 2 for printing values on LCD 16x2*/
volatile int flag_500ms=0; /* Task Flag 3 for sending data to terminal */

// Timer0 ISR for setting task flag

ISR(TIMER0_OVF_vect)
{
	static	int count_10ms=0;
	static	int count_100ms=0;
	static	int count_500ms=0;
	
	if(count_10ms == 10)
	{
		flag_10ms = 1;
		count_10ms = 0;
	}
	
	if(count_100ms == 100)
	{
		flag_100ms = 1;
		count_100ms = 0;
	}
	
	if(count_500ms == 500)
	{
		flag_500ms = 1;
		count_500ms = 0;
	}
	
	count_10ms++;
	count_100ms++;
	count_500ms++;
	
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
	
	char i;
	char str[30],str2[30];
	unsigned int AdcCurrentRead = 0,AdcBatVdcRead = 0;
	float AdcCurrentAvg, AdcbatVdcAvg, BatVdcVal = 0, CurrentVal = 0;
	
	/* Following are task status led pin */
	sbi(DDRB,0);
	sbi(DDRB,1);
	sbi(DDRB,2);
	//PORTB = 0x00;
	
	uart_init();			// UART INITIALIZE AT 9600 AS DEFAULT BAUD RATE
	
	timer0_init();
	
	lcd_init(LCD_DISP_ON);
	
	InitADC();

	sei();
	
	for(;;)
	
	if(flag_10ms == 1)
	{
		AdcCurrentRead = 0;
		AdcBatVdcRead = 0;
		
		/* Collecting samples of ADC value of Voltage and Current */
		for(i=0;i<ADC_SAMPLE_CNT;i++)
		{	
			AdcCurrentRead = AdcCurrentRead + ReadADC(0);   //reading ADC 10 bit value on channel 0
			AdcBatVdcRead = AdcBatVdcRead + ReadADC(1);
		}
		/* Samples of ADC reading for Voltage and Current are averaged */
		AdcCurrentAvg = (AdcCurrentRead/ADC_SAMPLE_CNT) - ADC_RAW_DIV;
		AdcbatVdcAvg = AdcBatVdcRead/ADC_SAMPLE_CNT;
		/* Calculating real Voltage and Current values  */
		CurrentVal = AdcCurrentAvg * ADC_REAL_MUL / MV_PER_AMP;
		BatVdcVal = AdcbatVdcAvg * ADC_REAL_MUL * ADC_REAL_V_MUL;
		
		//uart_txstr("inside 10 ms\r");
		sbi(PORTB,0);
		cbi(PORTB,1);
		cbi(PORTB,2);
		
		flag_10ms = 0;
		
	}
	
	else if(flag_100ms == 1)
	{
		lcd_clrscr();
		
		dtostrf(CurrentVal,2,2,str2);
		sprintf(str,"Current is %sA",str2);
		lcd_putsxy(0,0,str);
		
		dtostrf(BatVdcVal,2,2,str2);
		sprintf(str,"BVDC = %sV",str2);
		lcd_putsxy(0,1,str);
		
		//uart_txstr("inside 100 ms\r");
		sbi(PORTB,1);
		cbi(PORTB,0);
		cbi(PORTB,2);
		flag_100ms = 0;
	}
	
	else if(flag_500ms == 1)
	{	
		dtostrf(CurrentVal,2,2,str2);
		sprintf(str,"Current is %sA",str2);
		uart_txstr(str);
		
		dtostrf(BatVdcVal,2,2,str2);
		sprintf(str," BVDC = %sV",str2);
		uart_txstr(str);
		
		dtostrf(DCWATTHR,2,2,str2);
		sprintf(str," DC POWER = %sWATTS",str2);
		uart_txstr(str);
		
		uart_tx('\r');
		//uart_txstr("inside 500 ms\r");
		cbi(PORTB,1);
		cbi(PORTB,0);
		sbi(PORTB,2);
		flag_500ms = 0;
	}
}

