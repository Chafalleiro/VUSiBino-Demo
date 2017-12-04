/**
 * Project: VUSiBino test http://chafalladas.com
 * Author: Alfonso Abelenda Escudero alfonso@abelenda.es
 * Inspired by V-USB example code by Christian Starkjohann and v-usb tutorials from http://codeandlife.com
 * Hardware based on tinyUSBboard http://matrixstorm.com/avr/tinyusbboard/ and paperduino perfboard from http://txapuzas.blogspot.com.es
 * Copyright: (C) 2017 Alfonso Abelenda Escudero
 * License: GNU GPL v3 (see License.txt)
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usbdrv.h"

//#define F_CPU 16000000L // uncomment if not defined yet in the IDE or usbconfig.h

#define SERIAL_NUMBER_LENGTH 6 // the number of characters required for your serial number

static int  serialNumberDescriptor[SERIAL_NUMBER_LENGTH + 1];

#define USB_LED_ON 0
#define USB_LED_BLINK 1
#define USB_LED_OFF 2
#define USB_SEND_MESSAGE  3
#define USB_READ_MESSAGE  4
#define USB_SET_SERIAL 5

static char replyBuf[16] = "Hello, USB!";
static uchar dataReceived = 0, dataLength = 0; // for USB_DATA_IN
static char serialNum[16] = "DEM001";

int lapse_01 = 0;
volatile int countA = 0;
volatile int sw_led = 0b00000000;
volatile int operation = 0;

uchar usbFunctionDescriptor(usbRequest_t *rq)
{
   uchar len = 0;
   usbMsgPtr = 0;
   if (rq->wValue.bytes[1] == USBDESCR_STRING && rq->wValue.bytes[0] == 3) // 3 is the type of string descriptor, in this case the device serial number
   {
      usbMsgPtr = (uchar*)serialNumberDescriptor;
      len = sizeof(serialNumberDescriptor);
   }
   return len;
}

static void SetSerial(char *data)
{
   serialNumberDescriptor[0] = USB_STRING_DESCRIPTOR_HEADER(SERIAL_NUMBER_LENGTH);
   serialNumberDescriptor[1] = data[0];
   serialNumberDescriptor[2] = data[1];
   serialNumberDescriptor[3] = data[2];
   serialNumberDescriptor[4] = data[3];
   serialNumberDescriptor[5] = data[4];
   serialNumberDescriptor[6] = data[5];
}

// this gets called when custom control message is received
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data; // cast data to correct type
	operation=0;	
	switch(rq->bRequest) { // custom command is in the bRequest field
    case USB_LED_ON: //
		sw_led = sw_led & 0b11111101;
		DDRB = DDRB | 0b00100000; // PB1 as output
		PORTB = PORTB | 0b00100000; //turn LED on	
		return 0;
	case USB_LED_BLINK: //
		sw_led = sw_led | 0b00000010;
		return 0;	
	case USB_LED_OFF: // Turn off the led
		sw_led = sw_led & 0b11111101;
		PORTB &= ~(1<<PB5); //  Turn off led
		return 0;
	case USB_READ_MESSAGE: //Read message from buffer to PC
        usbMsgPtr = replyBuf;
        return sizeof(replyBuf);
	case USB_SET_SERIAL:
		operation = 9;
	case USB_SEND_MESSAGE://Write from PC to buffer
		dataLength  = (uchar)rq->wLength.word;
        dataReceived = 0;	
		if(dataLength  > sizeof(replyBuf)) // limit to buffer size
			dataLength  = sizeof(replyBuf);
		return USB_NO_MSG; // usbFunctionWrite will be called now
    }

    return 0; // should not get here
}

// This gets called when data is sent from PC to the device
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar i;
			
	for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
	{
		if (operation==9)
		{
			serialNum[dataReceived] = data[i];
		}
		else
		{
			replyBuf[dataReceived] = data[i];
		}
	}
    return (dataReceived == dataLength); // 1 if we received it all, 0 if not	
}

int main()
{
	SetSerial(serialNum);
	uchar i;

	DDRB = DDRB | 0b00101011; // PB0 PB1 PB3 PB5 as output Rest as input
	DDRD = DDRD | 0b01101000; // PD3 PD5 PD6 as output Rest as input

	TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
	TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
	
    wdt_enable(WDTO_1S); // enable 1s watchdog timer

    usbInit();
	
    usbDeviceDisconnect(); // enforce re-enumeration
    for(i = 0; i<250; i++)
		{ // wait 500 ms
        wdt_reset(); // keep the watchdog happy
        _delay_ms(2);
    	}
    usbDeviceConnect();


    sei(); // Enable interrupts after re-enumeration
//	OCR1B    = 6;  // 1 tic 0.000096 secs
	OCR1A    = 62;  // 1 Tic = 0.000992 Secs
//	OCR1B    = 625;  // 1 tic 0.01 secs

//	TCCR1B |= (1 << CS11);					// Start timer at Fcpu/8 16MHz/256=2.000.000 ticks per second.
//	TCCR1B |= (1 << CS11) | (1 << CS10);	// Start timer at Fcpu/64 16MHz/256=250.000 ticks per second.
	TCCR1B |= (1 << CS12);					// Start timer at Fcpu/256 16MHz/256=62500 ticks per second.
//	TCCR1B |= (1 << CS12) | (1 << CS10);	// Start timer at Fcpu/1024 16MHz/1024=15625 ticks per second.
	
    while(1)
	{
        wdt_reset(); // keep the watchdog happy
        usbPoll();
		if (operation == 9)
		{
			SetSerial(serialNum);
		}		
	}
    return 0;
}

ISR(TIMER1_COMPA_vect) //Timer 1 Compare A triggered.  We use Timer 1 for B port pins, no special reason for that.
{
countA++;
lapse_01++;
if(sw_led & 0b00000010) // Check if blink flag is active
	{
	if (countA > 100)
		{
		PORTB = PORTB ^ 0b00100000; // Toggle  the  LED
		sw_led = sw_led ^ 0b00000001; // Toggle  the  Switch
		countA = 0;
		}
	}			
}


