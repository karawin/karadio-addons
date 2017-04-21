// Nokia 5110 LCD support functions and font mapping
static const byte ASCII[][5] =
{{0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c £¤
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x08, 0x08, 0x2A, 0x1C, 0x08} // 7e ->
,{0x08, 0x1C, 0x2A, 0x08, 0x08} // 7f <-
};

char disp_tab[]={'0','1','2','3','4','5','6','7','8','9'};


void LcdInitialise(void)
  { pinMode(PIN_SCE,    OUTPUT);
    pinMode(PIN_RESET,  OUTPUT);
    pinMode(PIN_DC,     OUTPUT);
    pinMode(PIN_SDIN,   OUTPUT);
    pinMode(PIN_SCLK,   OUTPUT);
    digitalWrite(PIN_RESET, LOW);
    digitalWrite(PIN_RESET, HIGH);
    LcdWrite(LCD_C, 0x21 );  // LCD Extended Commands.
//    LcdWrite(LCD_C, 0xB3 );  // Set LCD Vop (Contrast). Change for individual display variance, B1, BA, BB, etc. - BF
    LcdWrite(LCD_C, NOKIAcontrast );
    LcdWrite(LCD_C, 0x04 );  // Set Temp coefficent. //0x04
    LcdWrite(LCD_C, 0x14 );  // LCD bias mode 1:48. //0x13
    LcdWrite(LCD_C, 0x0C );  // LCD in normal mode.
    LcdWrite(LCD_C, 0x20 );
    LcdWrite(LCD_C, 0x0C );  }


void LcdCharacter(char character)
  {
    if ((character > 0x7F)||(character < 0x20)) character = 0x20;
    LcdWrite(LCD_D, 0x00);
     for (int index = 0; index < 5; index++)  {
      LcdWrite(LCD_D, ASCII[character - 0x20][index]);  }
  LcdWrite(LCD_D, 0x00);  }


void LcdClear(void)  //20130504 Added nColumn/nRow reset and goto(0, 0)
  { for (int index = 0; index < LCD_X * LCD_Y / 8; index++)
  { LcdWrite(LCD_D, 0x00); }
    nColumn = 0; nRow = 0;
//    LcdCurrentLine(2);
    gotoXY(0, 0);}

void removeUtf8(byte *characters)
{
  int index = 0;
  while (characters[index])
  {
    if ((characters[index]) == 195) 
    {
      characters[index] = 'x';
//      Serial.println((characters[index]));
      characters[index+1] +=64;
      int sind = index+1;
      switch(characters[sind])
      {
      case 0xFF: case 0xFD: 
        characters[sind] = 'y'; break;     
      case 0xDF: 
        characters[sind] = 'Y'; break;  
      case 0xE7:  
        characters[sind] = 'c'; break;     
      case 0xC7: 
        characters[sind] = 'C'; break;  
      case 0xF2: case 0XF3: case 0xF4: case 0xF5: case 0xF6:
        characters[sind] = 'o'; break;     
      case 0xD2: case 0XD3: case 0xD4: case 0xD5: case 0xD6:
        characters[sind] = 'O'; break;             
      case 0xEC: case 0XED: case 0xEE: case 0xEF:
        characters[sind] = 'i'; break;       
      case 0xCC: case 0XCD: case 0xCE: case 0xCF:
        characters[sind] = 'I'; break;   
      case 0xF9: case 0XFA: case 0xFB: case 0xFC:
        characters[sind] = 'u'; break;       
      case 0xD9: case 0XDA: case 0xDB: case 0xDC:
        characters[sind] = 'U'; break;       
      case 0xE8: case 0XE9: case 0xEA: case 0xEB:
        characters[sind] = 'e'; break;
      case 0xC8: case 0XC9: case 0xCA: case 0xCB:
        characters[sind] = 'E'; break;
      case 0xE3: case 0XE0: case 0xE1: case 0xE2: case 0xE4: case 0XE5: case 0xE6: 
        characters[sind] = 'a'; break;
      case 0xC3: case 0XC0: case 0xC1: case 0xC2: case 0xC4: case 0XC5: case 0xC6: 
        characters[sind] = 'A'; break;
      default: ;
      }  
      while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
      characters[sind-1] = 0;     

    }
    index++;
  }
}

void LcdString(char *characters)
  {  while (*characters)
    {LcdCharacter(*characters++); }
  }
void LcdnString(char *characters, byte len)
  {  
    int i = 0;
    while ((*characters)&& (i++<len))
    {
      LcdCharacter(*characters++);
    }
  }


void LcdWrite(byte dc, byte data)
  { digitalWrite(PIN_DC, dc);
    digitalWrite(PIN_SCE, LOW);
    shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
    digitalWrite(PIN_SCE, HIGH);  }


void gotoXY(int x, int y)
  { LcdWrite( 0, 0x80 | x);  // Column.
    LcdWrite( 0, 0x40 | y); } // Row.  


void dispcountt(int count)
{
  //LcdCharacter(disp_tab[count/10000]);
  //LcdCharacter(disp_tab[count/1000%10]);
  //LcdCharacter(disp_tab[count/100%10]);
  LcdCharacter(disp_tab[count%100/10]);
  //LcdString(".");
  LcdCharacter(disp_tab[count%10]);
}

// Routine to create a black current line to overtype
void LcdCurrentLine(int line)
{
  unsigned char  j;  
  for(j=0; j<84; j+=2) //Bottom
	{ gotoXY (j, line);
//          LcdWrite (1,0x3c);
    LcdWrite (1,0x10);
    gotoXY (j+1, line);
          LcdWrite (1,0x18);         
  }
}


// Utility Functions for Magic Morse


void SendCharLCD( char temp ) {
  gotoXY(nColumn * 7, nRow); // Nokia LCD function to place character 6 lines of 12 characters in font
  LcdCharacter ( temp ); 
  ++nColumn;
  if (nColumn >= 12) {  // lines fill to 12 characters and increase through line 6 then line 1 is cleared
    nColumn = 0;
    nRow = ++nRow % 6;
    gotoXY(nColumn, nRow);
  }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


  
