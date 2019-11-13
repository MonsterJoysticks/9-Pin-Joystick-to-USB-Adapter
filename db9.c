/*
 * Project: Monster Joysticks DE9 to USB Joystick Adapter V1
 * Author: Monster Joysticks Ltd. <info@monsterjoysticks.com>
 * Copyright: (C) 2017 - 2019 Monster Joysticks Ltd. <info@monsterjoysticks.com>
 * License: GPLv2
 * Comments: Based on Nes/Snes/Genesis/SMS/Atari to USB by Raphaël Assenat raph@raphnet.net
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>
#include <string.h>

#include "gamepad.h"
#include "db9.h"

#define REPORT_SIZE		3
#define GAMEPAD_BYTES	3

#define INPUT_TYPE_ATARI	0x00 // up/dn/lf/rt/btn_b
#define INPUT_TYPE_SMS		0x01 // up/dn/lf/rt/btn_b/btn_c
#define INPUT_TYPE_MD		0x02 // up/dn/lf/rt/btn_a/btn_b/btn_c/start
#define INPUT_TYPE_MD6 		0x03 // all bits

#define isMD(a) ( (a) >= INPUT_TYPE_MD)

static unsigned char cur_id = INPUT_TYPE_MD;

static inline unsigned char SAMPLE()
{
	unsigned char res;

	/* Re-order the sampled input */

	res = (PINC & 0x01); // Up/Up/Z
	res |= (PINC & 0x02); // Down/Down/Y
	res |= (PINC & 0x04); // Left/0/X
	res |= (PINC & 0x08); // Right/0
	res |= (PINC & 0x10); // BtnB/BtnA
	res |= (PIND & 0x40) >> 1; // BtnC/BtnStart

	return res;
}

#define SET_SELECT()	PORTC |= (1<<PC5);
#define CLR_SELECT()	PORTC &= ~(1<<PC5);


static char db9Init(void);
static void db9Update(void);

// the most recent bytes we fetched from the controller
static unsigned char last_read_controller_bytes[REPORT_SIZE];
// the most recently reported bytes
static unsigned char last_reported_controller_bytes[REPORT_SIZE];

#define READ_CONTROLLER_SIZE 5

static void readController(unsigned char bits[READ_CONTROLLER_SIZE])
{
	unsigned char a,b,c,d,e;

	// total delays: 160 microseconds.

	/* |   1 |  2  |  3  |  4  | 5 ...
	 * ___    __    __    __    __
	 *    |__|  |__|  |__|  |__|
	 *  ^  ^     ^     ^   ^
	 *  A  B     D     E   C
	 *
	 *  ABC are used when reading controllers.
	 *  D and E are used for auto-detecting the genesis 6 btn controller.
	 *
	 */

	/* 1 */
	SET_SELECT();
	_delay_us(20.0);
	a = SAMPLE();

	if (cur_id == INPUT_TYPE_ATARI ||
		cur_id == INPUT_TYPE_SMS) {

		bits[0] = a;
		bits[1] = 0xff;
		bits[2] = 0xff;

		return;
	}
	CLR_SELECT();
	_delay_us(20.0);
	b = SAMPLE();

	/* 2 */
	SET_SELECT();
	_delay_us(20.0);
	CLR_SELECT();
	_delay_us(20.0);
	d = SAMPLE();

	/* 3 */
	SET_SELECT();
	_delay_us(20.0);
	CLR_SELECT();
	_delay_us(20.0);
	e = SAMPLE();

	/* 4 */
	SET_SELECT();
	_delay_us(20.0);
	c = SAMPLE();

	CLR_SELECT();
	_delay_us(20.0);

	/* 5 */
	SET_SELECT();

	bits[0] = a;
	bits[1] = b;
	bits[2] = c;
	bits[3] = d;
	bits[4] = e;
}

static char db9Init(void)
{
	unsigned char sreg;
	unsigned char bits[READ_CONTROLLER_SIZE];
	
	sreg = SREG;
	cli();

	DDRD &= ~0x40; //BtnC/BtnStart 
	PORTD |= 0x40;
	
	DDRC |= 0x20;	// SELECT
	PORTC |= 0x20;

	DDRC &= ~0x1F; // direction and buttons
	PORTC |= 0x1F;

	// set the output voltages and ground for controller
	DDRB |= 0x01;
	PORTB |= 0x01; // PB0 goes high
	DDRD |= 0x80;
	PORTD &= ~0x80; // PD7 goes low

	readController(bits);

	cur_id = INPUT_TYPE_SMS;

	// Megadrive 3 & 6 Button detection
	if ((bits[0]&0xf) == 0xf) {
		if ((bits[1]&0xf) == 0x3)
		{
			if (	((bits[3] & 0xf) != 0x3)  ||
					((bits[4] & 0xf) != 0x3) ) {
				cur_id = INPUT_TYPE_MD6;
			}
			else {
				cur_id = INPUT_TYPE_MD;
			}
		}
	}

	if (!(bits[1] & 0x20)) { // if start button initially held down
		cur_id = INPUT_TYPE_MD6;
	}

	db9Update();

	SREG = sreg;

	return 0;
}

static void db9Update(void)
{
	unsigned char data[READ_CONTROLLER_SIZE];
	int x=128,y=128;
	
	/* 0: Up//Z
	 * 1: Down//Y
	 * 2: Left//X
	 * 3: Right//Mode
	 * 4: Btn B/A
	 * 5: Btn C/Start/
	 */
	readController(data);

	/* Buttons are active low. Invert the bits
	 * here to simplify subsequent 'if' statements... */
	data[0] = data[0] ^ 0xff;
	data[1] = data[1] ^ 0xff;
	data[2] = data[2] ^ 0xff;

	if (data[0] & 1) { y = 0; } // up
	if (data[0] & 2) { y = 255; } //down
	if (data[0] & 4) { x = 0; }  // left
	if (data[0] & 8) { x = 255; } // right

	last_read_controller_bytes[0]=x;
	last_read_controller_bytes[1]=y;

 	last_read_controller_bytes[2]=0;

	if ( isMD(cur_id) ) {

		if (data[1]&0x10) { last_read_controller_bytes[2] |= 0x01; } // A
		if (data[0]&0x10) { last_read_controller_bytes[2] |= 0x02; } // B
		if (data[0]&0x20) { last_read_controller_bytes[2] |= 0x04; } // C
		if (data[1]&0x20) { last_read_controller_bytes[2] |= 0x08; } // Start
		if (cur_id == INPUT_TYPE_MD6) {
			if (data[2]&0x04) { last_read_controller_bytes[2] |= 0x10; } // X
			if (data[2]&0x02) { last_read_controller_bytes[2] |= 0x20; } // Y
			if (data[2]&0x01) { last_read_controller_bytes[2] |= 0x40; } // Z

			if (data[2]&0x08) { last_read_controller_bytes[2] |= 0x80; } // Mode
		}
	} else {
		if (data[0]&0x10) { last_read_controller_bytes[2] |= 0x01; } // Button 1
		if (data[0]&0x20) { last_read_controller_bytes[2] |= 0x02; } // Button 2
	}
}	

static char db9Changed(char id)
{
	static int first = 1;
	if (first) { first = 0;  return 1; }
	
	return memcmp(last_read_controller_bytes, 
					last_reported_controller_bytes, GAMEPAD_BYTES);
}

static char db9BuildReport(unsigned char *reportBuffer, char id)
{
	if (reportBuffer != NULL)
	{
		memcpy(reportBuffer, last_read_controller_bytes, GAMEPAD_BYTES);
	}
	memcpy(last_reported_controller_bytes, 
			last_read_controller_bytes, 
			GAMEPAD_BYTES);	

	return REPORT_SIZE;
}

static const char usbHidReportDescriptor[] PROGMEM = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    // USAGE (Pointer)
    0xa1, 0x00,                    // COLLECTION (Physical)
    0x09, 0x30,                    // USAGE (X)
    0x09, 0x31,                    // USAGE (Y)
    0x15, 0x00,                    // LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              // LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    // REPORT_SIZE (8)
    0x95, 0x02,                    // REPORT_COUNT (2)
    0x81, 0x02,                    // INPUT (Data,Var,Abs)
    0xc0,                          // END_COLLECTION
    0x05, 0x09,                    // USAGE_PAGE (Button)
    0x19, 0x01,                    // USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    // USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    // LOGICAL_MINIMUM (0)
    0x25, 0x01,                    // LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    // REPORT_SIZE (1)
    0x95, 0x08,                    // REPORT_COUNT (8)
    0x81, 0x02,                    // INPUT (Data,Var,Abs)
    0xc0                           // END_COLLECTION
};

Gamepad db9Gamepad = {
	.num_reports			=	1,
	.reportDescriptorSize	=	sizeof(usbHidReportDescriptor),
	.init					=	db9Init,
	.update					=	db9Update,
	.changed				=	db9Changed,
	.buildReport			=	db9BuildReport
};

Gamepad *db9GetGamepad(void)
{
	db9Gamepad.reportDescriptor = (void*)usbHidReportDescriptor;

	return &db9Gamepad;
}

