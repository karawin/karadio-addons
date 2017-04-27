/*

  Karadioucglib.pde
  
*/

//-------------------------------------------------------
// UnComment the following lines if you want the IR remote
#define IR
// Uncomment the following line to use the IRLib2 library
#define IRLib2
//-------------------------------------------------------

// your timezone offset
#define TZO 1

#undef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 256
#include "ucglibConf.h"
#include <EEPROM.h>
#include <time.h>
#include <avr/pgmspace.h>
#ifdef IR
#ifdef IRLib2
#include <IRLibDecodeBase.h>
// uncomment the needed protocols and check in the library that the receive is enabled
#include <IRLib_P01_NEC.h>
//#include <IRLib_P02_Sony.h>
//#include <IRLib_P03_RC5.h>
//#include <IRLib_P04_RC6.h>
//#include <IRLib_P05_Panasonic_Old.h>
//#include <IRLib_P06_JVC.h>
#include <IRLib_P07_NECx.h>
//#include <IRLib_P08_Samsung36.h>
//#include <IRLib_P09_GICable.h>
//#include <IRLib_P10_DirecTV.h>
//#include <IRLib_P11_RCMM.h>
//include additional protocols here
#include <IRLibCombo.h>
#include <IRLibRecvPCI.h>
#else
#include "IRremote.h"
#endif
#endif

#define SERIALX Serial
// the pin_playing is high when playing
#define PIN_PLAYING 5
#define PIN_LED 6
#define  BAUD       28800   // any standard serial value: 300 - 115200

// nams <--> num of line
#define STATIONNAME 0
#define STATION1  1
#define STATION2  2
#define IP        3
#define GENRE     2
#define TITLE1    3
#define TITLE11   4
#define TITLE2    5
#define TITLE21   6
#define VOLUME    7


// constants
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

#ifdef IR
char irStr[4];
// IR define and objects
#ifdef IRLib2
#define PIN_IRRECV  2
IRrecvPCI irrecv(PIN_IRRECV);//create instance of receiver using pin PIN_IRRECV
IRdecode irDecoder;   //create decoder
IRdecode results; //create decoder
#else
#define PIN_IRRECV	11
IRrecv irrecv(PIN_IRRECV); // The IR 
decode_results results; 
#endif
#endif
bool state = false; // start stop on Ok ir key
//-----------
#ifdef IR
// Character array pointers
   char  msg2[]      = {"IR+clcd V1.1"}; //
#else
	 char  msg2[]      = {"clcd V1.1"}; //
#endif
   char  msg1[]    = {"(c) KaraWin"}; //
   char  msg[]    = {"Karadio"};
      
// Karadio specific data
#define BUFLEN  160
#define BUFREC  254
#define LINES	7
char line[BUFREC]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char nameset[BUFLEN/2]; // the local name of the station
char nameNum[5]; // the number of the station
char genre[BUFLEN/2]; // the local name of the station

char* lline[LINES] ; // array of ptr of n lines 
uint8_t  iline[LINES] ; //array of index for scrolling
uint8_t  tline[LINES] ; // tempo at end or begin
uint8_t  mline[LINES+1] ; // mark to display
byte volume;
volatile unsigned  Rbindex = 0; // receive buffer Rbindex
unsigned loopScroll = 0;
unsigned loopDate = 0;
char oip[16];

int16_t y ;		//Height between line
int16_t yy;		//Height of screen
int16_t x ;		//Width of the screen
int16_t z ;		// an internal offset for y

struct tm *dt;
time_t timestamp = 0;
volatile bool syncTime = false;
volatile bool askDraw  = false;
volatile bool itAskTime = true;
volatile unsigned loopTime = 0;
volatile bool itSerial = false;
////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.
// setup on esp reset
void setup2(void)
{
  cli();//stop interrupts
  lline[0] = msg;
  lline[1] = msg1;
  lline[2] = msg2;
  nameNum[0]=0;
  eepromReadStr(EEaddrIp, oip);
  lline[3] = (char*)"IP:";
  lline[4] = oip;
  drawFrame();
  digitalWrite(PIN_PLAYING, LOW);
  sei();//allow interrupts
}

void setTimer2()
{
  cli();//stop interrupts
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR2A = F_CPU/2500/64 -1;// (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 64 prescaler
  TCCR2B |= (1 << CS22);//| (1 << CS21) ;   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();//allow interrupts
}

void setTimer1()
{
  //set timer1 interrupt at 4Hz
  cli();//stop interrupts
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 4hz increments
//  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  OCR1A = F_CPU/4096 -1;// = (16*10^6) / (2*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}

ISR(TIMER2_COMPA_vect)
{
  sei();//allow interrupts
  if (!itSerial)
  {
  itSerial = true;
  serial();
  itSerial = false;
  }
}

ISR(TIMER1_COMPA_vect)
{//timer1 interrupt 4Hz 
  if (loopTime%4 ==0) // 1hz
  {
    timestamp++;  // time update 
    loopDate++;
    if (!syncTime) itAskTime=true; // first synchro if not done 
    askDraw = true;  
  }
  loopScroll++; 
  if ((++loopTime%7200) == 0) itAskTime=true; // refresh ntp time every 30Mn
}




//Setup all things, check for contrast adjust and show initial page.
void setup(void) {
   char  msg3[] = {"Karadio"};

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PLAYING, OUTPUT);
  digitalWrite(PIN_PLAYING, LOW);
   SERIALX.begin(BAUD);

/*  while (!(SERIALX.isConnected() && (SERIALX.getDTR() || SERIALX.getRTS())))
  {
    digitalWrite(PIN_LED,!digitalRead(PIN_LED));// Turn the LED from off to on, or on to off
    delay(100);         // fast blink
  }

*/
#ifdef IR
  irrecv.enableIRIn(); // Start the IR receiver
 irStr[0] = 0;
#endif

#if defined(__AVR_ATmega32U4__)
// switch off the leds
  TXLED1;
  RXLED1;
#endif
// start the graphic library  
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
  ucg.setRotate90();
  if (ucg.getWidth() == 84)
  ucg.setFont(ucg_font_5x8_tf);
  else 
  ucg.setFont(ucg_font_6x10_tf);
  ucg.setFontPosTop();
  ucg.setColor(1,0,0,0);
// some constant data
  y = - ucg.getFontDescent()+ ucg.getFontAscent() +4; //interline
  yy = ucg.getHeight(); // screen height
  x = ucg.getWidth();   // screen width
  z = 0; 
  //banner
  ucg.setColor(0, 255, 0);
  for (int i = 0;i<3;i++)
  {
    if (i%2) ucg.setColor(0, 255, 0);
    else ucg.setColor(255, 255, 0);
    ucg.drawString(ucg.getWidth()/2 - (ucg.getStrWidth(msg3)/2), ucg.getHeight()/3,0, msg3);  
    delay(400);
    if (i%2) ucg.setScale2x2(); 
    else ucg.undoScale(); 
    ucg.clearScreen();
  }
  ucg.undoScale(); 
  lline[0] = msg;
  lline[1] = msg1;
  lline[2] = msg2;
  nameNum[0]=0;
  eepromReadStr(EEaddrIp, oip);
  lline[3] = (char*)"IP:";
  lline[4] = oip;
  drawFrame();

//  setTimer2();
  setTimer1();
}



////////////////////////////////////////
// Clear all buffers and Rbindexes
void clearAll()
{
      title[0] = 0;
      station[0]=0;
	  for (int i=1;i<LINES;i++) {lline[i] = NULL;iline[i] = 0;tline[i] = 0;mline[i]=1;}
}
////////////////////////////////////////
void cleartitle()
{
     title[0] = 0;
     for (int i = 3;i<LINES;i++)  // clear lines
     {
       lline[i] = NULL;
	     iline[i] = 0;
	     tline[i] = 0; 
       mline[i] = 1;
     }  
}

////////////////////////////////////////
void removeUtf8(byte *characters)
{
  int Rindex = 0;
  while (characters[Rindex])
  {
    if ((characters[Rindex] >= 0xc2)&&(characters[Rindex] <= 0xc3)) // only 0 to FF ascii char
    {
      //      SERIALX.println((characters[Rindex]));
      characters[Rindex+1] = ((characters[Rindex]<<6)&0xFF) | (characters[Rindex+1] & 0x3F);
      int sind = Rindex+1;
      while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
      characters[sind-1] = 0;
    }
    Rindex++;
  }
}

////////////////////////////////////////
void eepromReadStr(int addr, char* str)
{
  byte rd;
  do {
    rd = EEPROM.read(addr++);
    *str = rd;
//    SERIALX.println(str[0],16);
    str++;
  } while (( rd != 0)&&( rd != 0xFF)); 
  *str = 0;
}

////////////////////////////////////////
void eepromWriteStr(int addr, char* str)
{
  byte rd;
  do {
    EEPROM.write( addr++,*str);
    rd = *str;
    str++;
  } while (( rd != 0)&&( rd != 0xFF)); 
  EEPROM.write( addr,0);
}

////////////////////////////////////////
void separator(char* from)
{
    char* interp;
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr_PF(from,PSTR(" - "));
	if (from == nameset) {lline[0] = nameset;lline[1] = NULL;lline[2] = NULL;return;}
	if (interp != NULL)
	{
	  from[interp-from]= 0;
	  lline[(from==station)?STATION1:TITLE1] = from;
	  lline[(from==station)?STATION2:TITLE2] = interp+3;
    mline[(from==station)?STATION1:TITLE1]=1;
    mline[(from==station)?STATION2:TITLE2]=1;
	} else
	{
	  lline[(from==station)?STATION1:TITLE1] = from;
    mline[(from==station)?STATION1:TITLE1]=1;
	}

// 2 lines for Title
 if ((from == title)&&(ucg.getStrWidth(lline[TITLE1]) > x))
 {
    int idx = strlen(lline[TITLE1]);
    *(lline[TITLE1]+idx) = ' ';
    *(lline[TITLE1]+idx+1) = 0;
    while ((ucg.getStrWidth(lline[TITLE1]) > x)&&(idx !=0))
    {
      *(lline[TITLE1]+idx--)= ' ';
      while ((*(lline[TITLE1]+idx)!= ' ')&&(idx !=0)) idx--;
      if (idx != 0) *(lline[TITLE1]+idx)= 0;
    }
    lline[TITLE11] = lline[TITLE1]+idx+1;
    mline[TITLE11]=1; 
 }
 
 if ((from == title)&&(ucg.getStrWidth(lline[TITLE2]) > x))
 {
    int idx = strlen(lline[TITLE2]);
    *(lline[TITLE2]+idx) = ' ';
    *(lline[TITLE2]+idx+1) = 0;
    while ((ucg.getStrWidth(lline[TITLE2]) > x)&&(idx !=0))
    {
      *(lline[TITLE2]+idx--)= ' ';
      while ((*(lline[TITLE2]+idx)!= ' ')&&(idx !=0)) idx--;
      if (idx != 0) *(lline[TITLE2]+idx)= 0;
    }
    lline[TITLE21] = lline[TITLE2]+idx+1;
    mline[TITLE21]=1; 
 }

}
////////////////////////////////////////
// parse the karadio received line and do the job
void parse(char* line)
{
  char *ici,*pline;
  pline = line;

//unsigned cwr = SERIALX.availableForWrite() ;
//SERIALX.print("sys.log: ");SERIALX.print(pline); SERIALX.print("\r");
//while (SERIALX.availableForWrite() != cwr) ;

   removeUtf8((byte*)pline);  

// ##CLI. command
if ((ici=strstr(pline,"##CLI.")) != NULL)
{
   pline +=6;
////// ICY7 audio info
   if ((ici=strstr_PF(pline,PSTR("ICY7#: "))) != NULL)
   {
      digitalWrite(PIN_PLAYING, HIGH);
      return;  
   } else   
////// ICY6 Description
   if ((ici=strstr_PF(pline,PSTR("ICY6#: "))) != NULL)
   {
      digitalWrite(PIN_PLAYING, HIGH);
      return;  
   } else
////// ICY3 icy-url
   if ((ici=strstr_PF(pline,PSTR("ICY3#: "))) != NULL)
   {
      return;  
   } else
////// ICY1 Notice1
   if ((ici=strstr_PF(pline,PSTR("ICY1#: "))) != NULL)
   {
      return;  
   } else
////// ICY2 Notice2
   if ((ici=strstr_PF(pline,PSTR("ICY2#: "))) != NULL)
   {
       return;  
   } else      
////// ICY5 br
   if ((ici=strstr_PF(pline,PSTR("ICY5#: "))) != NULL)
   {
       return;  
   } else
////// ICY4 genre
    if ((ici=strstr_PF(pline,PSTR("ICY4#: "))) != NULL)
    {
	    strcpy(genre,ici+7);
	    if (lline[GENRE] == NULL)
	    {
	      lline[GENRE] = genre;
        markDraw(GENRE);  
      }
    } else 
////// ICY0 station name
   if ((ici=strstr_PF(pline,PSTR("ICY0#: "))) != NULL)
   {
	    if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
	    separator(station);
   } else
////// Meta title   
   if ((ici=strstr(pline,"META#: ")) != NULL)
   {     
      digitalWrite(PIN_PLAYING, HIGH);
     cleartitle(); 
     strcpy(title,ici+7);    
     separator(title); 
   } else 
////// STOPPED  
   if ((ici=strstr_PF(pline,PSTR("STOPPED"))) != NULL)
   {
       digitalWrite(PIN_PLAYING, LOW);
	     cleartitle();
       strcpy(title,"STOPPED");
       lline[TITLE1] = title;
       markDraw(TITLE1); 
   } else   
//////Nameset
   if ((ici=strstr_PF(pline,PSTR("MESET#: "))) != NULL)  
   {
     clearAll();
     strcpy(nameset,ici+8);
	   ici = strstr(nameset," ");
	   strncpy(nameNum,nameset,ici-nameset+1);
	   nameNum[ici - nameset+1] = 0;
	   strcpy(nameset,nameset+strlen(nameNum));
     lline[STATIONNAME] = nameset;
     markDraw(STATIONNAME);         
   } else
 //////Playing
   if ((ici=strstr(pline,"YING#")) != NULL)  
   {
	   digitalWrite(PIN_PLAYING, HIGH);
     if (strcmp(title,"STOPPED") == 0)
     {
		   title[0] = 0;
		   separator(title);
     }
   } else
 //////Volume
   if ((ici=strstr(pline,"VOL#:")) != NULL)  
   {
      volume = atoi(ici+6);
      markDraw(VOLUME);  
   } else
////// URLSET
   if ((ici=strstr_PF(pline,PSTR("URL"))) != NULL)
   {
       return;  
   } else  
////// PATHSET
   if ((ici=strstr_PF(pline,PSTR("PATH"))) != NULL)
   {
       return;  
   } else   
////// PORTSET
   if ((ici=strstr_PF(pline,PSTR("PORT"))) != NULL)
   {
       return;  
   } else        
////// OVOLSET
   if ((ici=strstr_PF(pline,PSTR("OVOL"))) != NULL)
   {
       return;  
   }  
}   else  
if ((ici=strstr(pline,"##SYS.")) != NULL)
{
// ##SYS. command
   pline += 6;
 //////Date Time  ##SYS.DATE#: 2017-04-12T21:07:59+01:00
   if ((ici=strstr(pline,"DATE#: ")) != NULL)  
   {
      char lstr[30];
      if (*(ici+7) != '2')//// invalid date. try again later
      {
//        askDraw = true;
          return;
      }
      strcpy(lstr,ici+7);
      dt = gmtime(&timestamp);
      int year,month,day,hour,minute,second;
      sscanf(lstr,"%04d-%02d-%02dT%02d:%02d:%02d",&(year),&(month),&(day),&(hour),&(minute),&(second));
      dt->tm_year = year; dt->tm_mon = month; dt->tm_mday = day;
      dt->tm_hour = hour; dt->tm_min = minute;dt->tm_sec =second;
      dt->tm_year -= 1900;
      timestamp = mktime(dt); 
      syncTime = true;
   }
}   else // other non tagged
{
 //////  reset of the esp
   if ((ici=strstr_PF(pline,PSTR("VS Version"))) != NULL) 
   {
      clearAll();
      setup2();
      syncTime = false;
   }  else
 /////// Station Ip       
   if ((ici=strstr_PF(pline,PSTR("Station Ip: "))) != NULL) 
   {
       eepromReadStr(EEaddrIp, oip);
       if ( strcmp(oip,ici+12) != 0)
         eepromWriteStr(EEaddrIp,ici+12 );   
   } 
}      
}

////////////////////////////////////////
// receive the esp8266 stream

void serial()
{
    byte temp;
    while (Serial.available() > 0) 
    {
      temp = Serial.read();
      if (temp == '\r')
      {
        line[Rbindex] = 0; // end of string
        parse(line);
        Rbindex = 0;
        serial();
//        break;
      } else
      if ((temp != '\n') && (temp != '\\'))
      {
        line[Rbindex++] = temp;  
        if (Rbindex>=BUFREC-1) 
        {
//          SERIALX.println(F("overflow"));
//          line[Rbindex] = 0;
 //         parse(line);
          Rbindex = 0;
          serial();
        }
      }
    }
/*    while ((temp=SERIALX.read()) != -1)
    {
	    switch (temp)
	    {
        case '\\':break;
		    case '\n' : break;
		    case '\r' : 
				line[Rbindex] = 0; // end of string
				parse(line);
        Rbindex = 0;
				break;
		    default : // put the received char in line
				line[Rbindex++] = temp;
        if (Rbindex>BUFREC-1) 
        {
          SERIALX.println(F("overflow"));
          line[Rbindex] = 0;
          parse(line);
          Rbindex = 0;
        }       
	    }
    }
    */
}

// Mark the lines to draw
void markDraw(int i)
{
  mline[i] = 1;
}
////////////////////////////////////////
// draw the full screen
void drawLines()
{
    for (int i=0;i<LINES;i++)
    {
        if (mline[i]) draw(i); 
        serial();
    }
    if (mline[VOLUME]) draw(VOLUME); 
//time
//    
    serial();
    drawTime();
}
void drawTime()
{
     char strsec[10]; 
static uint16_t oldmin = 0xff;;
   char strdte[12];
  dt=gmtime(&timestamp);
  if (x==84)
  {
    sprintf(strsec,"%02d:%02d:%02d", dt->tm_hour, dt->tm_min,dt->tm_sec);
    sprintf(strdte,"%02d-%02d",dt->tm_mon,dt->tm_mday);
  }
  else
  {
    sprintf(strsec,"%02d:%02d:%02d", dt->tm_hour, dt->tm_min,dt->tm_sec);
    sprintf(strdte,"%02d-%02d-%04d",dt->tm_mon,dt->tm_mday,dt->tm_year+1900);
  }
     
   if (dt->tm_min == oldmin) // only time
    {
      ucg.setColor(240,240,255);
      ucg.setFontMode(UCG_FONT_MODE_SOLID);
//      cli();//stop interrupts
      ucg.drawString(3*x/4-(ucg.getStrWidth(strsec)/2),yy-2*y,0,strsec); 
//      sei(); //enable interrupts 
      ucg.setFontMode(UCG_FONT_MODE_TRANSPARENT);
    } else // all  date+time
    { 
      ucg.setColor(0,0,0,0);
      ucg.drawBox(0,yy-2*y, x ,y); // full line
      ucg.setColor(240,240,255);
//      cli();//stop interrupts
      ucg.drawString(3*x/4-(ucg.getStrWidth(strsec)/2),yy-2*y,0,strsec); 
      ucg.drawString(x/4-(ucg.getStrWidth(strdte)/2),yy-2*y,0,strdte);
//      sei(); //enable interrupts 
    }
    oldmin = dt->tm_min;
}
////////////////////
void drawFrame()
{
    ucg.clearScreen();
    ucg.setColor(0,255,255,0);  
    ucg.setColor(1,0,255,255);  
    ucg.drawGradientLine(0,(4*y) - (y/2)-5,x,0);
    ucg.setColor(0,255,255,255);  
    ucg.drawBox(0,0,x,y);  
    ucg.setColor(1,0,0,0);  
    for (int i=0;i<LINES;i++) draw(i);
    draw(VOLUME); 
    drawTime();
}
//////////////////////////
// set color of font per line
void setColor(int i)
{
        switch(i){
          case STATIONNAME: ucg.setColor(0,0,0); break;
          case STATION1: ucg.setColor(255,255,255); break;
          case STATION2: ucg.setColor(255,200,200);  break;
          case TITLE1:
          case TITLE11: ucg.setColor(255,255,0);  break;
          case TITLE2:
          case TITLE21: ucg.setColor(0,255,255); break; 
          default:ucg.setColor(100,255,100);  
        }  
}
////////////////////
// draw one line
void draw(int i)
{   
//  SERIALX.print("Draw ");SERIALX.print(i);SERIALX.print("  ");SERIALX.println(lline[i]);
      if (i >=3) z = y/2 ; else z = 0;
			if (i == STATIONNAME) 
			{	
        ucg.setColor(255,255,255); 	
        ucg.drawBox(0,0,x,y); 	
        ucg.setColor(0,0,0);	
				if (nameNum[0] ==0)  ucg.drawString(1,2,0,lline[i]+iline[i]);
				else 
				{
//					cli();//stop interrupts
          ucg.drawString(1,1,0,nameNum);
					ucg.drawString(ucg.getStrWidth(nameNum)-2,2,0,lline[i]+iline[i]);
//          sei(); //enable interrupts
				}
        if ( mline[i]) mline[i] =0;
			} else
      if (i == VOLUME)
      {
        ucg.setColor(0,0,200);   
        ucg.drawBox(0,yy-4,x,4); 
        ucg.setColor(255,0,0); 
        ucg.drawBox(1,yy-3,((uint16_t)(x*volume)/255),2);                  
      }
			else 
			{
        ucg.setColor(0,0,0); 
        ucg.drawBox(0,y*i+z,x,y); 
        setColor(i);
//        cli();//stop interrupts
			  ucg.drawString(0,y*i+z,0,lline[i]+iline[i]); 
//        sei(); //enable interrupts 
        if ( mline[i]) mline[i] =0;      
			}   
}

////////////////////////////////////////
// scroll each line if out of screen
void scroll()
{
int16_t len;

	for (int i = 0;i < LINES;i++)
	{  
	   if (tline[i]>0) 
	   {
	     if (tline[i] == 3) 
	     {
	      iline[i]= 0;
	      if (ucg.getStrWidth(lline[i]) > x) markDraw(i);//draw(i);
	     }
	     tline[i]--;		 
	   } 
	   else
	   {
		   if (i == 0)
			 len = ucg.getStrWidth(nameNum) + ucg.getStrWidth(lline[i]+iline[i]);
		   else
			 len = ucg.getStrWidth(lline[i]+iline[i]);
		   if (len > x)
      {      
 		    iline[i] += x/6;
        len = iline[i];
        while ((*(lline[i]+iline[i])!=' ')&&(*(lline[i]+iline[i])!='-')&&(iline[i]!= 0))iline[i]--;
        if (iline[i]==0) iline[i]=len;     
		    markDraw(i); //draw(i);
      }
		   else 
			  {tline[i] = 4;}
	   }
	}
}

////////////////////////////
#ifdef IR
// a number of station in progress...
void nbStation(char nb)
{
  if (strlen(irStr)>=3) irStr[0] = 0;
  uint8_t id = strlen(irStr);
  irStr[id] = nb;
  irStr[id+1] = 0;
}
////////////////////////////
void translateIR() // takes action based on IR code received
//  KEYES Remote IR codes (NEC P01)
//  and Ocean Digital remote (NEC P07)
{
#ifdef IRLib2
  if (irrecv.getResults())
  {
    results.decode();    
#else
	if ((irrecv.decode(&results)))
  {
#endif  

//  Uncomment the following line to see the code of your remote control and report to the case the value
#ifdef IRLib2
//	    SERIALX.print(F("Protocol:"));SERIALX.print(results.protocolNum);SERIALX.print(F("  value:"));SERIALX.println(results.value,HEX);
#else
//      SERIALX.print(F("Protocol:"));SERIALX.print(results.decode_type);SERIALX.print(F("  value:"));SERIALX.println(results.value,HEX);
#endif
//      SERIALX.print(F("  value:"));SERIALX.println(results.value,HEX);
		switch(results.value)
		{
			case 0xFF629D: 
			case 0x10EF48B7:	/*(" FORWARD");*/  irStr[0] = 0;SERIALX.print(F("cli.next\r")); break;

			case 0xFF22DD:
			case 0x10EFA857:
			case 0x10EF42BD: /*(" LEFT");*/  irStr[0] = 0;SERIALX.print(F("cli.vol-\r"));  break;

			case 0xFF02FD:
			case 0x10EF7887:		/*(" -OK-");*/
			{  
//      state?SERIALX.print(F("cli.start\r")):SERIALX.print(F("cli.stop\r"));
        if (strlen(irStr) >0)
        {
          SERIALX.print(F("cli.play(\""));SERIALX.print(irStr);SERIALX.print(F("\")\r"));
          irStr[0] = 0;
        }
        else
        { 
          state?SERIALX.print(F("cli.start\r")):SERIALX.print(F("cli.stop\r"));
/*				  if (state)
           SERIALX.print("cli.start\r");
			  	else
			  		SERIALX.print("cli.stop\r");
           */
        }
				state = !state;
        irStr[0] = 0;
				break;
			}
			case 0xFFC23D:
			case 0x10EF28D7:
			case 0x10EF827D: /*(" RIGHT");*/ irStr[0] = 0;SERIALX.print(F("cli.vol+\r"));  break; // volume +
			case 0xFFA857:
			case 0x10EFC837:	/*(" REVERSE");*/ irStr[0] = 0;SERIALX.print(F("cli.prev\r")); break;
			case 0xFF6897:
			case 0x10EF807F: /*(" 1");*/ nbStation('1');   break;
			case 0xFF9867:
			case 0x10EF40BF: /*(" 2");*/ nbStation('2');   break;
			case 0xFFB04F:
			case 0x10EFC03F: /*(" 3");*/ nbStation('3');   break;
			case 0xFF30CF:
			case 0x10EF20DF: /*(" 4");*/ nbStation('4');   break;
			case 0xFF18E7:
			case 0x10EFA05F: /*(" 5");*/ nbStation('5');   break;
			case 0xFF7A85:
			case 0x10EF609F: /*(" 6");*/ nbStation('6');   break;
			case 0xFF10EF:
			case 0x10EFE01F: /*(" 7");*/ nbStation('7');   break;
			case 0xFF38C7:
			case 0x10EF10EF: /*(" 8");*/ nbStation('8');   break;
			case 0xFF5AA5:
			case 0x10EF906F: /*(" 9");*/ nbStation('9');   break;
			case 0xFF42BD:
			case 0x10EFE817: /*(" *");*/   irStr[0] = 0;SERIALX.print(F("cli.stop\r")); break;
			case 0xFF4AB5:
			case 0x10EF00FF: /*(" 0");*/ nbStation('0');   break;
			case 0xFF52AD:
			case 0x10EFB847: /*(" #");*/   irStr[0] = 0;SERIALX.print(F("cli.start\r")); break;
			case 0xFFFFFFFF: /*(" REPEAT");*/break;
			default:;
			/*SERIALX.println(" other button   ");*/
		}// End Case
;	
#ifdef IRLib2
    irrecv.enableIRIn();      //Restart receiver
#else
		irrecv.resume(); // receive the next value
#endif    
	}
} //END translateIR
#endif

// send a request  for the time to esp
void askTime()
{
    if (itAskTime) // time to ntp. Don't do that in interrupt.
    {
        SERIALX.print(F("sys.date\r")) ; 
        itAskTime = false;
    }  
}
////////////////////////////////////////
void loop(void) {
#ifdef IR
    translateIR();
#endif

serial();

// scrolling control and draw control
//    if (loopScroll >=2 ) // 500ms
    { 
      if (askDraw) // something to display
      {
        drawLines();   
        askDraw = false; 
      } 
      if (loopScroll >=2 ) // 500ms
      {   
        digitalWrite(PIN_LED, !digitalRead(PIN_LED)); // blink led  
        scroll();
        loopScroll = 0;
      } 
    }
// ntp control and first info demand
     if (loopDate>=5)
     {
         loopDate = 0;
          if (itAskTime)
          {
            if (!syncTime) // time to ntp. Don't do that in interrupt.
            {
              SERIALX.print(F("\rsys.tzo(\"")) ;SERIALX.print(TZO);SERIALX.print(F("\")\r")); 
              SERIALX.print(F("cli.info\r")); // Synchronise the current state   
              itAskTime = false;             
            } 
            else askTime();
          }
      }    
}

