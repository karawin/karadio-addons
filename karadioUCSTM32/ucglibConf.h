// ucglibConf.h
/*

>>> Before compiling: Please remove comment from the type of the
>>> connected graphics display (see below).

Use Universal 8bit Graphics Library, https://github.com/olikraus/ucglib/

*/

#if defined(__arm__)
#ifndef __NOP
#define __NOP __asm volatile ("nop")
#endif
#endif

#include <SPI.h>
#include <Ucglib.h>
#include <clib/ucg_karadio32_fonts.h>


//-----------------------------------------------------
//  >>> Please uncomment  one of the following . <<<  
//-----------------------------------------------------
//#define ILI9163_18x128x128
//#define SSD1351_18x128x128
//#define SSD1351_18x128x128_FT
//#define SEPS225_16x128x128_UNIVISION
//#define PCF8833_16x132x132
#define ST7735_18x128x160
//#define ILI9341_18x240x320
//#define HX8352C_18x240x400
//#define ILI9486_18x320x480

//-----------------------------------------------
// Do not modify after this line
//-----------------------------------------------
#if defined (ILI9163_18x128x128)
Ucglib_ILI9163_18x128x128_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W128
#elif defined (SSD1351_18x128x128)
Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W128
#elif defined (SSD1351_18x128x128_FT)
Ucglib_SSD1351_18x128x128_FT_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W128
#elif defined (SEPS225_16x128x128_UNIVISION)
Ucglib_SEPS225_16x128x128_UNIVISION_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W128
#elif defined (PCF8833_16x132x132)
Ucglib_PCF8833_16x132x132_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W132
#elif defined (ST7735_18x128x160)
Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W160
#elif defined (ILI9341_18x240x320)
Ucglib_ILI9341_18x240x320_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W320
#elif defined (HX8352C_18x240x400)
Ucglib_HX8352C_18x240x400_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W400
#elif defined (ILI9486_18x320x480)
Ucglib_ILI9486_18x320x480_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#define W480
#endif


// not supported
//#define W64
//Ucglib_SSD1331_18x96x64_UNIVISION_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Version 1.0: Initial release 05/2017 jp Cocatrix
Version 1.3:release 02/2019 jp Cocatrix
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**** Instruction for 1.8" Serial 128X160 SPI TFT LCD Module Display + t *************************
 *    + IR + Rotary encoder
     ----------------------------------------------------------------------------------------------
NOTES:
Confirmed vendor:
lcd:
https://www.aliexpress.com/item/J34-A96-Free-Shipping-1-8-Serial-128X160-SPI-TFT-LCD-Module-Display-PCB-Adapter-Power/32599685873.html?spm=2114.13010608.0.0.MvX4RI
https://www.aliexpress.com/item/1-8-inch-1-8-TFT-LCD-Display-module-ST7735S-128x160-51-AVR-STM32-ARM-8/32769506973.html?spm=2114.13010608.0.0.Dm20du

STM32:
https://www.aliexpress.com/item/1pcs-STM32F103C8T6-ARM-STM32-Minimum-System-Development-Board-Module-For-arduino/32720697607.html?spm=2114.13010608.0.0.Dm20du

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
  PA7     LCD SDA .... Pin 6
  PA5  	  LCD SCK .... Pin 7
  PB6  	  LCD TFT CS.. Pin 3
  PB5	    LCD A0	.... Pin 5
  PB4     LCD RST .... Pin 4
  Gnd			LCD Gnd .... Pin 2
  3.3			LCD Vcc .... Pin 1  3.3 VCC
				  LCD Led .... Pin 8  To Vcc thru a resistor around 100 ohms


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
  A         PB7
  B         PB8
  Switch    PB9
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

Libraries needed:
ucglib for stm32 must be installed in  arduino IDE library;
https://github.com/karawin/Ucglib_Arduino

IRMP library for stm32Duino:
https://github.com/karawin/irmp-master
See https://www.mikrocontroller.net/articles/IRMP_-_english

KY040 library:
https://github.com/karawin/encoder


--------
WARNING:
The webradio serial must be set at 115200 bauds
--------

In karadioUCSTM32.ino:
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
