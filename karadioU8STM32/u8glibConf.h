// u8glibConf.h

#ifndef _U8GLIBCONF_h
#define _U8GLIBCONF_h
#define logo_width 60
#define logo_height 51

#include "U8glib.h"
static unsigned char logo_bits[] PROGMEM = {
  0xFF, 0xFF, 0x3F, 0x00, 0xC0, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0x07, 0x00, 
  0x00, 0xFE, 0xFF, 0x0F, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0xF8, 0xFF, 0x0F, 
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0x0F, 0xFF, 0x3F, 0x00, 0x00, 
  0x00, 0xC0, 0xFF, 0x0F, 0xFF, 0x1D, 0x20, 0x00, 0x40, 0x80, 0xFF, 0x0F, 
  0x7F, 0x0E, 0x90, 0x00, 0x80, 0x00, 0xFF, 0x0F, 0x3F, 0x07, 0x40, 0x00, 
  0x20, 0x00, 0xFE, 0x0F, 0xBF, 0x03, 0x48, 0x00, 0x20, 0x01, 0xFC, 0x0F, 
  0xDF, 0x03, 0x08, 0x61, 0x48, 0x00, 0xF8, 0x0F, 0xCF, 0x01, 0x20, 0xF9, 
  0x49, 0x02, 0xF8, 0x0F, 0xE7, 0x00, 0xA4, 0xF8, 0x41, 0x02, 0xF0, 0x0F, 
  0xF7, 0x00, 0x24, 0xFC, 0x53, 0x00, 0xE0, 0x0F, 0x73, 0x00, 0x84, 0xFC, 
  0x03, 0x02, 0xE0, 0x0F, 0x7B, 0x00, 0x20, 0xF8, 0x51, 0x02, 0xC0, 0x0F, 
  0x39, 0x00, 0x24, 0xF9, 0x48, 0x00, 0xC0, 0x0F, 0x3D, 0x00, 0x28, 0x60, 
  0x00, 0x01, 0xC0, 0x0F, 0x1D, 0x00, 0x40, 0x72, 0x24, 0x01, 0x80, 0x0F, 
  0x1C, 0x00, 0x48, 0xF0, 0x90, 0x00, 0x80, 0x0F, 0x1E, 0x00, 0x90, 0xF0, 
  0x80, 0x00, 0x80, 0x0F, 0x1E, 0x00, 0x20, 0xF0, 0x40, 0x00, 0x80, 0x0F, 
  0x1E, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0x0E, 0x00, 0x00, 0xF0, 
  0xC0, 0x1F, 0x00, 0x0F, 0x0E, 0x60, 0x30, 0xB8, 0xC1, 0x3F, 0x00, 0x0F, 
  0x0E, 0x60, 0x18, 0xD8, 0xC1, 0x70, 0x00, 0x0F, 0x0E, 0x70, 0x1C, 0xF8, 
  0xC1, 0x60, 0x00, 0x0F, 0x0F, 0x60, 0x0C, 0x98, 0xE1, 0x60, 0x00, 0x0F, 
  0x0E, 0x30, 0x06, 0x98, 0xC1, 0x60, 0x00, 0x0F, 0x0E, 0x70, 0x03, 0xBC, 
  0xC1, 0x60, 0x00, 0x0F, 0x0F, 0xA0, 0x03, 0x9C, 0xC3, 0x70, 0x00, 0x0F, 
  0x1F, 0xF0, 0x01, 0x0C, 0xC3, 0x3F, 0x80, 0x0F, 0x1F, 0xE0, 0x01, 0xDC, 
  0xC3, 0x0F, 0x80, 0x0F, 0x1F, 0x60, 0x03, 0x0C, 0xE3, 0x0C, 0x80, 0x0F, 
  0x1F, 0x70, 0x07, 0x0C, 0xC3, 0x1C, 0x80, 0x0F, 0x3F, 0x60, 0x06, 0x0C, 
  0xC7, 0x18, 0xC0, 0x0F, 0x3F, 0x70, 0x0C, 0xDE, 0xC7, 0x30, 0xC0, 0x0F, 
  0x3F, 0x60, 0x18, 0x06, 0xC6, 0x70, 0xC0, 0x0F, 0x7F, 0x70, 0x38, 0x06, 
  0xC6, 0x60, 0xE0, 0x0F, 0x7F, 0x20, 0x30, 0xFE, 0x47, 0x20, 0xF0, 0x0F, 
  0xFF, 0x00, 0x00, 0x07, 0x06, 0x00, 0xF0, 0x0F, 0xFF, 0x01, 0x00, 0x03, 
  0x0C, 0x00, 0xF8, 0x0F, 0xFF, 0x01, 0x00, 0xDF, 0x0F, 0x00, 0xF8, 0x0F, 
  0xFF, 0x03, 0x00, 0x03, 0x0C, 0x00, 0xFC, 0x0F, 0xFF, 0x07, 0x00, 0x03, 
  0x0C, 0x00, 0xFE, 0x0F, 0xFF, 0x0F, 0x00, 0x03, 0x1C, 0x00, 0xFF, 0x0F, 
  0xFF, 0x1F, 0x80, 0x01, 0x08, 0x80, 0xFF, 0x0F, 0xFF, 0x3F, 0x00, 0x00, 
  0x00, 0xC0, 0xFF, 0x0F, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0x0F, 
  0xFF, 0xFF, 0x03, 0x00, 0x00, 0xFC, 0xFF, 0x0F, 0xFF, 0xFF, 0x0F, 0x00, 
  0x00, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0x3F, 0x00, 0xC0, 0xFF, 0xFF, 0x0F, 
  }; 


#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Version 1.0  first release
*/


/**** Instruction for 128X64 OLED LCD LED Display Module white For Arduino 0.96" I2C IIC SPI Serial *************************
 *    + IR + Rotary encoder
     ----------------------------------------------------------------------------------------------
NOTES:
Confirmed vendor:
lcd:
https://www.aliexpress.com/item/Free-shipping-1Pcs-128X64-OLED-LCD-LED-Display-Module-white-For-Arduino-0-96-I2C-IIC/32671702306.html?spm=2114.13010608.0.0.9K7DWb

STM32:
https://www.aliexpress.com/item/1pcs-STM32F103C8T6-ARM-STM32-Minimum-System-Development-Board-Module-For-arduino/32720697607.html?spm=2114.13010608.0.0.Dm20du
https://www.aliexpress.com/item/1pcs-STM32F103C8T6-ARM-STM32-Minimum-System-Development-Board-Module-For-arduino/32720697607.html?spm=2114.13010608.0.0.9K7DWb

Rotary Encoder Module KY-040:
https://www.aliexpress.com/item/5pcs-lot-Rotary-Encoder-Module-Free-Shipping-Dropshipping-KY-040-for-Arduino/32218652643.html?spm=2114.13010608.0.0.Dm20du

Infrared IR wireless:
https://www.aliexpress.com/item/Hot-Selling-1pcs-New-Infrared-IR-Wireless-Remote-Control-Module-Kits-For-Arduino-Wholesale/32334118062.html?spm=2114.13010608.0.0.j3B4M6 

--------
 Pins:
--------
See http://karadio.karawin.fr/images/KaradioUCSTM32l.jpg

Cable wiring between Graphic LCD and STM32:
-------------------------------------------
___STM32___LCD_______________
  PB6      LCD SDA .... PIN 4
  PB7      LCD SCK .... Pin 3
  Gnd      LCD Gnd .... Pin 1 or 2 (see the board)
  3.3      LCD Vcc .... Pin 2 or 1 (for your model) 


Cable wiring between nodeMcu and stm32
--------------------------------------
--NodeMcu--   --stm32
  Tx            Rx   PA10
  Rx            Tx   PA9
  Gnd           Gnd
  VU            VB

Cable wiring between STM32 and IR:
----------------------------------
The IR receiver pins:
From left to right (pin at bottom, sensor in front of you)
  GND  3.3  SIGNAL
to STM32:
  GND  3.3  PC15

Cable wiring between STM32 and KY040:
-------------------------------------
--KY040--   --STM32--
  A         PB12
  B         PB13
  Switch    PB14
  Vcc
  Gnd

Software:
---------
STM32DUINO  (to add support of the stm32 with arduino IDE:
https://github.com/rogerclarkmelbourne/Arduino_STM32
Bootloader (see http://wiki.stm32duino.com/index.php?title=Bootloader)
https://github.com/rogerclarkmelbourne/STM32duino-bootloader (the one we need is generic_boot20_pc13.bin)
Instruction to load the bootloader on stm32 (one time)
http://www.stm32duino.com/viewtopic.php?f=32&t=413
http://wiki.stm32duino.com/index.php?title=Uploading_a_sketch

Libraries needed:
u8glib for stm32 must be installed in  arduino IDE library;
https://github.com/karawin/U8glib_Arduino

IRMP library for stm32Duino:
https://github.com/karawin/irmp-master
See https://www.mikrocontroller.net/articles/IRMP_-_english

KY040 library:
https://github.com/karawin/encoder

IMPORTANT:
In C:\Users\jp\Documents\Arduino\hardware\Arduino_STM32\STM32F1\libraries\Wire
In wirebase.h change
In line 47
#define WIRE_BUFSIZ 32
to
#define WIRE_BUFSIZ 256  



--------
WARNING:
The webradio serial must be set at 115200 bauds
--------

In karadioU8STM32.ino:
//  Uncomment the following line to see the code of your remote control and report to the case the value
//    Serial.println(irmp_protocol_names[irmp_data.protocol]);
//    Serial.println(irmp_data.address,HEX);
//    Serial.println(irmp_data.command,HEX);
//    Serial.println(irmp_data.flags,HEX);
//    Serial.println(code,HEX);
//    Serial.println();
If you want to use another remote, you can see the code of the keys and modify the switch case for the needed function.
See IRMP.h  to add the protocol you need for the remote.

Enjoy.

jpc 05/2017
*/















////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  *** Instruction for NOKIA 5110*********************************************
        ---------------------------
NOTES:
Confirmed vendor:
lcd: http://www.ebay.fr/itm/262558872574?_trksid=p2057872.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT
cpu: http://www.ebay.fr/itm/252409929753?_trksid=p2057872.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT

MiniPRO 3.3 v 8MHz Pins:
#__      Function_________________________________

RESET    Reset (not used)
Tx  0    n/a (dedicated Serial Input)
Rx  1    to the tx of the nodeMcu
PIN 2    n/a
PIN 3-7  Nokia Display  (specifics below)
PIN 8    Activate changes to NOKIA contrast B0 --> BF GND and press RESET

Raw: 3.3 to 16 VDC Max

Nokia 5110 Graphic LCD Pinout:
_______ Mini Pro____   _______ Nokia GLCD___
#define PIN_SCE   7    LCD CE ....  Pin 2
#define PIN_RESET 6    LCD RST .... Pin 1
#define PIN_DC    5    LCD Dat/Com. Pin 3  (DC)
#define PIN_SDIN  4    LCD SPIDat . Pin 4  (DIN)
#define PIN_SCLK  3    LCD SPIClk . Pin 5

//                     LCD Gnd .... Pin 8
//                     LCD Vcc .... Pin 6   3.3 volts  from the minipro VCC
//                     LCD Vled ... Pin 7   (100 to 300 Ohms to Gnd)

Cable wiring between nodeMcu and Mini Pro
-----------------------------------------
--NodeMcu--   --Mini Pro--
Tx            Rx
Gnd           Gnd
VU            Raw

Extra features:
---------------
A Jumper between Nokia GLCD pin 7 and  100 ohm + Gnd to switch on or off the lcd light
A7 ---| |--- 100 ohms --- GND
A push button between PIN8 of mini pro and GND to ajust the contrast after a reset.
- Press the switch, reset and wait for the right contrast (32 steps) .
- Release the switch. Done.
The contrast is saved in eprom.

Software: 
u8glib must be installed in library;
In .../Arduino\libraries\U8glib\utility/u8g.h
 uncomment the following line 
#define U8G_16BIT 1

In file u8glibConf.h
comment the current u8g
uncomment:
U8GLIB_PCD8544 u8g(PIN_SCLK, PIN_SDIN, PIN_SCE, PIN_DC, PIN_RESET); // SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9, Reset = 8

Warning:
The webradio serial must be set at 28800 b/s
with sys.uart("28800") command



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
**** Instruction for 0.96" I2C IIC SPI Serial 128X64 White OLED LCD LED Display Module for Arduino *************************
     ----------------------------------------------------------------------------------------------
NOTES:
Confirmed vendor:
lcd: http://www.ebay.fr/itm/191909116703?_trksid=p2057872.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT
cpu: http://www.ebay.fr/itm/Pro-Mini-Atmega328-5V-16M-Micro-controller-Board-for-Arduino-Compatible-Nano-new-/331809989705?hash=item4d416aa449

MiniPRO 5v 16MHz
 Pins:
#__      Function_________________________________

RESET    Reset (not used))
Rx  1    to the tx of the nodeMcu
Gnd      ground
jp2: A4,A5	I2C lines
Raw: 5 to 16 VDC Max

Graphic LCD Pinout:
 Mini Pro____   _______  GLCD___
JP2 A4             LCD SDA .... Pin 3
P2  A5             LCD SCK .... Pin 4
//                 LCD Gnd .... Pin 1
//                 LCD Vcc .... Pin 2  5 volts  from the minipro VCC

Depending of the pcb, A4 & A5 are on the opposite pins of the board, or on the pcb near A2 & A3 

Cable wiring between nodeMcu and Mini Pro
-----------------------------------------
--NodeMcu--   --Mini Pro--
Tx            Rx
Gnd           Gnd
VU            Raw

Software:
u8glib must be installed in library;
In .../Arduino\libraries\U8glib\utility/u8g.h
uncomment the following line
#define U8G_16BIT 1

In file u8glibConf.h
comment the current u8g
uncomment:
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI

--------
WARNING:
The webradio serial must be set at 28800 b/s
--------
WARNING:
---------
The nodeMcu is a 3.3v device.
DO NOT connect the Rx pin of the lcd to the Tx pin of the mini Pro


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
**** Instruction for the IR module *****************************************************************************************
------------------------------------
The model used here is from:
https://www.aliexpress.com/item/Hot-Selling-1pcs-New-Infrared-IR-Wireless-Remote-Control-Module-Kits-For-Arduino-Wholesale/32334118062.html?spm=2114.13010608.0.0.XzmgYk
or equivalent. Cost less than 1$

- Based on NEC protocol; Built-in 1 x AG10 battery;
- Remote control range: above 8m;
- Wavelength: 940Nm;
- Frequency: crystal oscillator: 455KHz; IR carrier frequency: 38KHz

// obsolete-----------
See Library at: https://github.com/shirriff/Arduino-IRremote
Install the library found at  https://github.com/z3t0/Arduino-IRremote/releases/download/2.1.0/Arduino-IRremote-dev.zip
Uncomment the #define IR at the beginning of the karadioU8glib.ino file.
----------------------
// new library
Install the library from https://github.com/cyborg5/IRLib2
Uncomment the #define IR  and the #ifdef IRLib2 at the beginning of the karadioU8glib.ino file.
----------------------

The IR receiver pins:
From left to right (pin at bottom)
Gnd VCC Signal
The signal must be connected to
#define PIN_IRRECV	11  for the old library or
#define PIN_IRRECV  2  or 3 for the new library IRLib2
It is the default In karadioU8glig.ino, you can change it

In karadioU8glig.ino:
//  Uncomment the following line to see the code of your remote control and report to the case the value
//	    Serial.print("Protocol:");Serial.print(results.decode_type);Serial.print("  value:");Serial.println(results.value,HEX);
If you want to use another remote, you can see the code of the keys and modify the switch case for the needed function.
See IRremote.h  to add the protocol you need for the remote.

The tx of the pro mini must be connected to the rx of the esp12 but
Warning; the esp rx pin is not 5V tolerant, so if your pro mini is a 5v version we nned to adapt the level.
-------
See
http://www.instructables.com/id/Pi-Cubed-How-to-connect-a-33V-Raspberry-Pi-to-a-5V/

The value of the resistors is not critical. Only th ration 1/3 2/3 must be respected 1k and 2k for example)

I found that the ir is not very accurate, but it works.
Enjoy.

jpc 01/2017
*/

