/*
 * adcatmega16.h
 *
 * Created: 2/25/2012 9:17:12 AM
 *  Author: Avinash Gupta
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>


//Threshold Values For Sensor Triggering


unsigned int adval;   // for storing 12 bit ADCH+ADCL after bit shifting on below variable st
unsigned char st[1];  // for storing ADCH and ADCL and to perform bit shifting on it

//Function To Initialize the ADC Module
void InitADC();

/* 

Function To Read ADC Channel

Argument: Channel Number between 0-7
Return Value : Between 0-1023

*/
unsigned int ReadADC(char m);



#endif /* ADC_H_ */
