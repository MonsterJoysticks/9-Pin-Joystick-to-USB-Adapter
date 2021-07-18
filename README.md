# 9 Pin Joystick to USB Adapter
Firmware for the 9 Pin Joystick to USB Adapter https://monsterjoysticks.com/9-pin-joystick-to-usb-adapter

You can use the adapter to connect any of the following devices via USB
* Mini Monster Retro Gaming Joystick
* Atari 2600 Joysticks
* Atari ST Compatible Joysticks
* Amiga Compatible Joysticks
* Commodore 64 Compatible Joysticks
* Amstrad CPC Compatible Joysticks (without fire button 2 support - see note)
* SEGA Master System Controllers
* SEGA MegaDrive/Genesis Controllers (3 & 6 Button variants)
* Amiga CD32 (Hold up when connecting USB to enable CD32 Compatibility)

NOTE: For full Amstrad CPC Compatibility (including fire button 2 support) see the Amstrad fork - https://github.com/MonsterJoysticks/9-Pin-Joystick-to-USB-Adapter-Amstrad

The adapter is detected as a USB HID joystick and is compatible with the following:
* Windows
* MacOS
* Most Linux Distros

# Updating the Firmware
The latest pre-compiled firmware hex file ready for flashing can be found at https://github.com/MonsterJoysticks/9-Pin-Joystick-to-USB-Adapter/releases/download/1.03/main.hex. Alternatively, you can compile your own.

Once you have downloaded or compiled the firmware hex file, you can use the BootloadHID <https://www.obdev.at/products/vusb/bootloadhid.html> command line utility or HIDBootFlash GUI and command line utility (Windows only) to flash the file to the device via USB. 

You will just need to change the position of the small switch on the device so that it is closer to the 9 pin connector and then connect the device to your computer via USB. Once connected, the BootloadHID or HIDBootFlash software should be able to flash the device. You can then return the switch to its original position and reconnect the USB cable.

# Compiling the Firmware
To compile your own firmware, you will need to have the AVR gcc compiler and AVR C library installed. On Debian based systems (e.g. Ubuntu), you can install these packages as follows:
* sudo apt install gcc-avr
* sudo apt install avr-libc

Once you have done the above, you should then just be able to compile the firmware using the usual "make" command. If successful, you should then find the file "main.hex" in the "bin" folder, which you can then flash to your USB adaptor as per the above instructions.