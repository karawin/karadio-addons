// ucglibConf.h
/*

>>> Before compiling: Please remove comment from the constructor of the
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

/*
  Hardware SPI Pins:
    Arduino Uno		sclk=13, data=11
    Arduino Due		sclk=76, data=75
    Arduino Mega	sclk=52, data=51
	Arduino Mini	sclk=13, data=11 (mosi)
	Arduino Micro	sclk=15, data=16
    
  >>> Please uncomment (and update) one of the following constructors. <<<  
*/
//Ucglib8BitPortD ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, /* wr= */ 18 , /* cd= */ 19 , /* cs= */ 17, /* reset= */ 16 );
//Ucglib8Bit ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, 0, 1, 2, 3, 4, 5, 6, 7, /* wr= */ 18 , /* cd= */ 19 , /* cs= */ 17, /* reset= */ 16 );

//Ucglib4WireSWSPI ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, /*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);	// not working
//Ucglib4WireSWSPI ucg(ucg_dev_ili9325_spi_18x240x320, ucg_ext_ili9325_spi_18, /*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);	// not working
//Ucglib3WireILI9325SWSPI ucg(ucg_dev_ili9325_spi_18x240x320, ucg_ext_ili9325_spi_18, /*sclk=*/ 13, /*data=*/ 11, /*cs=*/ 10, /*reset=*/ 8);	// not working
//Ucglib3WireILI9325SWSPI ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, /*sclk=*/ 13, /*data=*/ 11, /*cs=*/ 10, /*reset=*/ 8);	// not working

//Ucglib_ST7735_18x128x160_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

#if defined(__STM32F1__)
// STM32
//Ucglib_ST7735_18x128x160_SWSPI ucg(/*sclk=*/ PA5, /*data=*/ PA7, /*cd=*/ PB9 , /*cs=*/ PB5, /*reset=*/ PB3);
Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ PB5 , /*cs=*/ PB6, /*reset=*/ PB4);
#else
//Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
#endif
//Ucglib_ILI9163_18x128x128_SWSPI ucg(/*sclk=*/ 7, /*data=*/ 6, /*cd=*/ 5 , /*cs=*/ 3, /*reset=*/ 4);
//Ucglib_ILI9163_18x128x128_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);	/* HW SPI Adapter */

//Ucglib_ILI9341_18x240x320_SWSPI ucg(/*sclk=*/ 7, /*data=*/ 6, /*cd=*/ 5 , /*cs=*/ 3, /*reset=*/ 4);
//Ucglib_ILI9341_18x240x320_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_ILI9341_18x240x320_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_ILI9341_18x240x320_SWSPI ucg(/*sclk=*/ 4, /*data=*/ 3, /*cd=*/ 6 , /*cs=*/ 7, /*reset=*/ 5);	/* Elec Freaks Shield */

//Ucglib_SSD1351_18x128x128_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1351_18x128x128_FT_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1351_18x128x128_FT_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

//Ucglib_PCF8833_16x132x132_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cs=*/ 9, /*reset=*/ 8);	/* linksprite board */
//Ucglib_PCF8833_16x132x132_HWSPI ucg(/*cs=*/ 9, /*reset=*/ 8);	/* linksprite board */

//Ucglib_LD50T6160_18x160x128_6Bit ucg( /*d0 =*/ d0, /*d1 =*/ d1, /*d2 =*/ d2, /*d3 =*/ d3, /*d4 =*/ d4, /*d5 =*/ d5, /*wr=*/ wr, /*cd=*/ cd, /*cs=*/ cs, /*reset=*/ reset);
//Ucglib_LD50T6160_18x160x128_6Bit ucg( /*d0 =*/ 16, /*d1 =*/ 17, /*d2 =*/ 18, /*d3 =*/ 19, /*d4 =*/ 20, /*d5 =*/ 21, /*wr=*/ 14, /*cd=*/ 15); /* Samsung 160x128 OLED with 6Bit minimal interface with Due */ 
//Ucglib_LD50T6160_18x160x128_6Bit ucg( /*d0 =*/ 5, /*d1 =*/ 4, /*d2 =*/ 3, /*d3 =*/ 2, /*d4 =*/ 1, /*d5 =*/ 0, /*wr=*/ 7, /*cd=*/ 6); /* Samsung 160x128 OLED with 6Bit minimal interface with Uno */ 

//Ucglib_SSD1331_18x96x64_UNIVISION_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1331_18x96x64_UNIVISION_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

//Ucglib_SEPS225_16x128x128_UNIVISION_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SEPS225_16x128x128_UNIVISION_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Version 1.0: Initial release 05/2017 jp Cocatrix
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
