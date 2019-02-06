
/*

  Karadio IIC LCD 16x2.ino

  Edited by Alexander - yo2ldk - 2019.

  Arduino Board         IIC LCD
       A4                 sda   
       A5                 scl  

*/
// include the library code:
//#include <LiquidCrystal.h>

#include <Wire.h>
//#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN 3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

//int n = 1;

LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

#define version 3.0
#include <EEPROM.h>
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

// Character array pointers
char  msg[]       = {"Karadio LCD"}; //
char  msg1[]      = {"(c) KaraWin"}; //
char  blank[]     = {"                "};
char  msg2[]      = {" Internet Radio "};

// Karadio specific data
#define BUFLEN  200
#define LINES	2
char line[BUFLEN]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char nameset[BUFLEN];
char* lline[LINES] ; // array of ptr of n lines
int  iline[LINES] ; //array of index for scrolling
byte  tline[LINES] ;
char* ici;
char* ici2;
unsigned index = 0;
unsigned loopcount = 0;
unsigned scrl = 0;

int x = 16;    //Width
char temp;

int laststateUp = HIGH;
int laststateDown = HIGH;
int laststatePlay = HIGH;
int laststateStop = HIGH;
int buttonState = 0;

////////////////////////////////////////
// Clear all buffers and indexes

void clearAll()
{
  title[0] = 0;
  station[0] = 0;
  for (int i = 0; i < LINES; i++)
  {
    lline[i] = NULL; iline[i] = 0; tline[i] = 0;
    lcd.setCursor(0, i);
    lcd.print(blank);
  }
}
////////////////////////////////////////
void cleartitle()
{
  title[0] = 0;
  for (int i = 1 ; i < LINES; i++) // clear lines
  {
    lline[i] = NULL;
    iline[i] = 0;
    tline[i] = 0;
    lcd.setCursor(0, i);
    lcd.print(blank);
  }
  draw();
}

////////////////////////////////////////
void eepromReadStr(int addr, char* str)
{
  byte rd;
  do {
    rd = EEPROM.read(addr++);
    *str = rd;
    //    Serial.println(str[0],16);
    str++;
  } while (( rd != 0) && ( rd != 0xFF));
  *str = 0;
}

////////////////////////////////////////
void eepromWriteStr(int addr, char* str)
{
  byte rd;
  do {
    EEPROM.write( addr++, *str);
    rd = *str;
    str++;
  } while (( rd != 0) && ( rd != 0xFF));
  EEPROM.write( addr, 0);
}

////////////////////////////////////////
// parse the karadio received line and do the job

void parse(char* line)
{
  int mul;
  //     removeUtf8((byte*)line);
  //Serial.println(line);
  ////// Meta title
  if ((ici = strstr(line, "META#: ")) != NULL)
  {
    cleartitle();
    //remove station name if present
    if ((ici2 = strstr(line, station)) != NULL)
    {
      ici += strlen(station);
    }
    strcpy(title, ici + 7);
    lline[1] = title;
    draw();
  } else

    ////// ICY0 station name
    if ((ici = strstr(line, "ICY0#: ")) != NULL)
    {
      int len;
      clearAll();
      if (strlen(ici + 7) == 0) strcpy (station, nameset);
      else strcpy(station, ici + 7);
      lline[0] = station;
      draw();
    } else
      ////// STOPPED
      if ((ici = strstr(line, "STOPPED")) != NULL)
      {
        cleartitle();
        strcpy(title, "STOPPED");
        lline[1] = title;
        draw();
      }
  /////// Station Ip
      else if ((ici = strstr(line, "Station Ip: ")) != NULL)
      {
        char oip[20];
        //   Serial.println(line);
        eepromReadStr(EEaddrIp, oip);
        if ( strcmp(oip, ici + 12) != 0)
          eepromWriteStr(EEaddrIp, ici + 12 );

      } else
        // advertise web interface
        if ((ici = strstr(line, "IP: ")) != NULL)
        {
          clearAll();
          lline[0] = msg2;
          strcpy(title, ici + 4);
          lline[1] = title;
          draw();
        }

}

////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.

void setup(void) {
  char oip[20];
  Serial.begin(115200);
  while (!Serial) { ;}
  Serial.println("Arduino Ready");

  // set up the LCD's number of columns and rows:
  lcd.begin(16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.noAutoscroll() ;
  lcd.print(msg);
  lcd.setCursor(0,1);
  lcd.print(msg1);
  delay(3000); 
  clearAll();
  eepromReadStr(EEaddrIp, oip);
lline[0] = (char*)"IP:";
  lline[1] = oip;
  draw();

}

////////////////////////////////////////
// receive the esp8266 stream

void serial()
{
  char temp;
  while ((temp = Serial.read()) != -1)
  {
    switch (temp)
    {
      case '\n' : if (index == 0) break;
      case '\r' :
        line[index] = 0; // end of string
        index = 0;
        parse(line);
        break;
      default : // put the received char in line
        if (index > BUFLEN - 1) break; // small memory so small buffer
        line[index++] = temp;
    }
  }
}

////////////////////////////////////////
// draw all lines
void draw()
{
  lcd.clear();
  for (int i = LINES - 1; i >= 0; i--)
  {
    serial();
    lcd.setCursor(0,i);
    if ((lline[i] != NULL)) lcd.print(lline[i] + iline[i]);
    delay(1);
  }

}
////////////////////////////////////////
// scroll each line
void scroll()
{
  unsigned len;
  int dodraw = 0;

  for (int i = 0; i < LINES; i++)
  {
    if (tline[i] > 0)
    {
      if (tline[i] == 4) {
        iline[i] = 0;
        dodraw = 1;
      }
      tline[i]--;
    }
    else
    {
      len = strlen(lline[i] + (iline[i]));
      if (len > x) {
        iline[i]++;
        dodraw = 1;
      }
      else
      {
        tline[i] = 6;
      }
    }
  }
  if (dodraw == 1)	draw();

}

////////////////////////////////////////
void loop(void) {
  serial();
  if (loopcount++ == 0x3fff)
  {
    loopcount = 0;
    //if (++scrl%6 == 0) digitalWrite(13,HIGH);
    scroll();
    //digitalWrite(13,LOW);
  }

}

