/*
 * Project: Monster Joysticks DE9 to USB Joystick Adapter V1
 * Author: Monster Joysticks Ltd. <info@monsterjoysticks.com>
 * Copyright: (C) 2017 - 2019 Monster Joysticks Ltd. <info@monsterjoysticks.com>
 * License: GPLv2
 * Comments: Based on Nes/Snes/Genesis/SMS/Atari to USB by Raphaël Assenat raph@raphnet.net
 */

#include "devdesc.h"
#include "usbdrv.h"
#include "usbconfig.h"

#define USBDESCR_DEVICE         1

const char usbDescrDevice[] PROGMEM = {    /* USB device descriptor */
    18,         /* sizeof(usbDescrDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,    /* descriptor type */
    0x01, 0x10, /* USB version supported */
    USB_CFG_DEVICE_CLASS,
    USB_CFG_DEVICE_SUBCLASS,
    0,          /* protocol */
    8,          /* max packet size */
    USB_CFG_VENDOR_ID,  /* 2 bytes */
    USB_CFG_DEVICE_ID,  /* 2 bytes */
    USB_CFG_DEVICE_VERSION, /* 2 bytes */
#if USB_CFG_VENDOR_NAME_LEN
    1,          /* manufacturer string index */
#else
    0,          /* manufacturer string index */
#endif
    2,          /* product string index */
    0,          /* serial number string index */
	1,          /* number of configurations */
};

int getUsbDescrDevice_size(void) { return sizeof(usbDescrDevice); }