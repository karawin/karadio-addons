
/*

  Karadiolcd16x2.ino

  Edited by Paul Sijben May 2018.

*/
// include the library code:
#include <LiquidCrystal.h>
/*
 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 2
 * LCD D5 pin to digital pin 3
 * LCD D6 pin to digital pin 4
 * LCD D7 pin to digital pin 5
 * LCD R/W pin to ground
 * 10K potentiometer:
 *      ends to +5V and ground
 *      wiper to LCD VO pin (pin 3)
*/

#define version 2.0

#define ChUpPin 10
#define ChDownPin 9
#define PlayPin 8
#define StopPin 7

// constants
const int  BAUD            = 28800;  // use the telnet interface on the esp32 to set the baudrate there sys.uart("28800")

const byte ContrastPin     = 8;     // D8 low activates the Contrast adjustment

// Character array pointers
   char  msg[]       = {"Karadio lcd"}; //
   char  msg1[]      = {"(c) KaraWin"}; //
   char  blank[]     = {"                "};
// Karadio specific data
#define BUFLEN  200
#define LINES    2
char line[BUFLEN]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];    // received title
char nameset[BUFLEN];
char* lline[LINES] ; // array of ptr of n lines
int  iline[LINES] ; //array of index for scrolling
byte  tline[LINES] ;
char* ici;
char* ici2;
unsigned index = 0;
unsigned loopcount = 0;
unsigned scrl = 0;

int laststateUp=LOW;
int laststateDown=LOW;
int laststatePlay=LOW;
int laststateStop=LOW;
int buttonState = 0;
int x = 16;        //Width
char temp;

 // initialize the library with the numbers of the interface pins
  LiquidCrystal lcd(12, 11, 2, 3, 4, 5);

////////////////////////////////////////
// Clear all buffers and indexes
void clearAll()
{
      title[0] = 0;
      station[0]=0;
      for (int i=0;i<LINES;i++)
      {
      lline[i] = NULL;iline[i] = 0;tline[i] = 0;
       lcd.setCursor(0, i);
       lcd.print(blank);
      }
}
////////////////////////////////////////
void cleartitle()
{
     title[0] = 0;
     for (int i = 1 ;i<LINES;i++)  // clear lines
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
// parse the karadio received line and do the job
void parse(char* line)
{
  int mul;
     removeUtf8((byte*)line);
   //Serial.println(line);
 ////// Meta title
   if ((ici=strstr(line,"META#: ")) != NULL)
   {
     cleartitle();
     //remove station name if present
     if ((ici2=strstr(line,station)) != NULL)
     {
      ici+=strlen(station);
     }
     strcpy(title,ici+7);
     lline[1] = title;
     draw();
   } else

 ////// ICY0 station name
   if ((ici=strstr(line,"ICY0#: ")) != NULL)
   {
      int len;
      clearAll();
      if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
      lline[0] = station;
      draw();
   } else
 ////// STOPPED
   if ((ici=strstr(line,"STOPPED")) != NULL)
   {
       cleartitle();
       strcpy(title,"STOPPED");
       lline[1] = title;
           draw();
   }

}


////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.
void setup(void) {
    char oip[20];
   Serial.begin(BAUD);
   while (!Serial) { ;}
Serial.println("Arduino Ready");
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  pinMode(13, OUTPUT); // led

   //Serial.println(F("Free RAM available:")) ;
   //Serial.print(freeRam());
    lcd.clear();
  lcd.noAutoscroll() ;

  lcd.print(msg);
  lcd.setCursor(0, 1);
  lcd.print(msg1);
// initialize the pushbutton pins as an input:
  pinMode(ChUpPin, INPUT);
  pinMode(ChDownPin, INPUT);
  pinMode(PlayPin, INPUT);
  pinMode(StopPin, INPUT);

  delay(3000);
  clearAll();

}

////////////////////////////////////////
// receive the esp8266 stream
void serial()
{
    char temp;
    while ((temp=Serial.read()) != -1)
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
                if (index>BUFLEN-1) break; // small memory so small buffer
                line[index++] = temp;
        }
    }
}

////////////////////////////////////////
// draw all lines
void draw()
{
    lcd.clear();
        for (int i = 0;i < LINES;i++)
        {
            serial();
      lcd.setCursor(0, i);
            if ((lline[i] != NULL)) lcd.print(lline[i]+iline[i]);
      delay(1);
        }
}
////////////////////////////////////////
// scroll each line
void scroll()
{
unsigned len;
    for (int i = 0;i < LINES;i++)
    {
       if (tline[i]>0)
       {
         if (tline[i] == 4) iline[i]= 0;
         tline[i]--;
       }
       else
       {
           len = strlen(lline[i]+(iline[i]));
           if (len > x) iline[i]++;
          else
            {tline[i] = 6;}
       }
    }
    draw();
}

////////////////////////////////////////
void loop(void) {
    serial();
    if (loopcount++ == 0x3fff)
    {
        loopcount = 0;
        //if (++scrl%6 == 0) digitalWrite(13, HIGH);
        scroll();
        //digitalWrite(13, LOW);
    }
    // yes this can be done in a nicer way, but for 4 buttons I am not going to bother
    buttonState = digitalRead(ChUpPin);
    if (buttonState!= laststateUp) {
      laststateUp=buttonState;
      if (buttonState == HIGH) {
        Serial.println("cli.next");
      }
    }
    buttonState = digitalRead(ChDownPin);
    if (buttonState!= laststateDown) {
      laststateDown=buttonState;
      if (buttonState == HIGH) {
        Serial.println("cli.prev");
      }
    }
    buttonState = digitalRead(PlayPin);
    if (buttonState!= laststatePlay) {
      laststatePlay=buttonState;
      if (buttonState == HIGH) {
        Serial.println("cli.start");
      }
    }
    buttonState = digitalRead(StopPin);
    if (buttonState!= laststateStop) {
      laststateStop=buttonState;
      if (buttonState == HIGH) {
        Serial.println("cli.stop");
      }
    }
}
