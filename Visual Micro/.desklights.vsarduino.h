/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Yún, Platform=avr, Package=arduino
*/

#define __AVR_ATmega32u4__
#define __AVR_ATmega32U4__
#define USB_VID 0x2341
#define USB_PID 0x8041
#define USB_MANUFACTURER 
#define USB_PRODUCT "\"Arduino Yun\""
#define ARDUINO 101
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define __cplusplus
extern "C" void __cxa_pure_virtual() {;}

//
void ledrun();
//
void process(YunClient client);
void powerCommand(YunClient client);
void deskCommand(YunClient client);
void globalCommand(YunClient client);
void setDeskPalette(uint8_t deskNum, uint8_t palette);
const CRGBPalette16 getPalette(uint8_t paletteId);
void DimmAll(byte value, int desk);
void DimmAllGlobal(byte value);
int getTimeMins();

#include "E:\ArduinoDev\Arduino\hardware\arduino\avr\variants\yun\pins_arduino.h" 
#include "E:\ArduinoDev\Arduino\hardware\arduino\avr\cores\arduino\arduino.h"
#include "E:\Dropbox\Arduino\desklights\desklights.ino"
