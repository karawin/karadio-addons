/*

  KaradioU8glib.pde
  
*/

//-------------------------------------------------------
// UnComment the following line if you want the IR remote
#define IR
// Uncomment the following line to use the IRLib2 library
#define IRLib2
//=========================
// Uncomment your oled type
//=========================
#define oled096  
//#define oled130 
//#define Nokia5110
//-------------------------------------------------------

// your timezone offset
#define TZO 1

#undef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 128
#include "u8glibConf.h"
#include <EEPROM.h>
#include <time.h>
#include <avr/pgmspace.h>
#ifdef IR
#ifdef IRLib2
#include <IRLibDecodeBase.h>
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
//#include <IRLibRecv.h>
#include <IRLibRecvPCI.h>
#else
#include "IRremote.h"
#endif
#endif


#define PIN_LED 13
#define PIN_PLAYING 12
#define  BAUD       28800   // any standard serial value: 300 - 115200

// nams <--> num of line
#define STATIONNAME 0
#define STATION1  1
#define STATION2  2
#define IP        3
#define GENRE     2
#define TITLE1    3
#define TITLE2    4
#define VOLUME    5


// constants
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

const byte ContrastPin     = 8;     // D8 low activates the Contrast adjustment


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
bool state = false; // start stop on Ok key
//-----------
#ifdef IR
// Character array pointers
 char  msg2[]       = {"IR+lcd V1.5"}; //
#else
 char  msg2[]     = {"lcd V1.5"}; //
#endif
 char  msg1[]   = {"(c) KaraWin"}; //
 char  msg[]   = {" Karadio"};
      
// Karadio specific data
#define BUFLEN  180
#define LINES	6
char line[BUFLEN]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char nameset[BUFLEN/2]; // the local name of the station
char nameNum[5]; // the number of the station
char genre[BUFLEN/2]; // the local name of the station
char* lline[LINES] ; // array of ptr of n lines 
uint8_t  iline[LINES] ; //array of index for scrolling
uint8_t  tline[LINES] ;
int16_t volume;
unsigned index = 0;
unsigned loopScroll = 0;
unsigned loopDate = 0;
char oip[20];

uint16_t y ;		//Height of a line
uint16_t yy;		//Height of screen
uint16_t x ;		//Width
uint16_t z ;		// an internal offset for y

byte NOKIAcontrast;                 // LCD initialization contrast values B0 thru BF

struct tm *dt;
time_t timestamp = 0;
bool syncTime = false;
bool askDraw  = false;
bool itAskTime = true;
unsigned loopTime = 0;

////////////////////////////////////////
void u8g_prepare(void) {
  if (u8g.getWidth() == 84)
	u8g.setFont(u8g_font_5x8);
  else 
//	u8g.setFont(u8g_font_6x13);
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
}


////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.
void setup2()
{
	clearAll();
	lline[0] = (char*)msg;
	lline[1] = (char*)msg1;
	lline[2] = (char*)msg2;
	nameNum[0]=0;
	eepromReadStr(EEaddrIp, oip);
	lline[3] = (char*)"IP:";
	lline[4] = oip;
	draw(0);
	digitalWrite(PIN_PLAYING, LOW);
}

void setTimer2()
{
  cli();//stop interrupts
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1khz increments
  OCR2A = F_CPU/1000/64 -1;// (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 64 prescaler
  TCCR2B |= (1 << CS22) ;   
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
  cli();//stop interrupts
  serial();
  sei();//allow interrupts
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


void setup(void) {
   char  msg3[] = {"Karadio"};
   Serial.begin(BAUD);
   while (!Serial) ;

   pinMode(PIN_LED, OUTPUT);
	 pinMode(PIN_PLAYING, OUTPUT);
   pinMode(ContrastPin, INPUT);
   digitalWrite(ContrastPin, HIGH); // activate internal pullup resistor
	 digitalWrite(PIN_PLAYING, LOW);

#ifdef IR
	 irrecv.enableIRIn(); // Start the IR receiver
   irStr[0] = 0;
#endif   

ReStart:  // Come back here if LCD contract is changed
    // Read the EEPROM to determine if display is using a custom contrast value
  NOKIAcontrast = EEPROM.read(EEaddr1);
    // Set the Nokia LCD Contrast to default or reset if EEPROM is corrupt or set to new value
  if (NOKIAcontrast  < 0xB0 || NOKIAcontrast > 0xCF) NOKIAcontrast = 0xB8;
	u8g.setContrast(2*(NOKIAcontrast-0x80));
	u8g_prepare();
	y = u8g.getFontLineSpacing();
	yy = u8g.getHeight();
	x = u8g.getWidth();
	z = 0; 
	clearAll();  

	for (int i = 0;i<5;i++)
	{
	  u8g.firstPage();
	  do {
//  		if (!(i%2)) u8g.drawFrame(0,0,x/2-1,yy/2-1); 
//	      else u8g.drawFrame(0,0,x-1,yy-1); 
		   u8g.drawStr(u8g.getWidth()/2 - (u8g.getStrWidth(msg3)/2), u8g.getHeight()/3, msg3);
	  } while( u8g.nextPage() );
	  delay(500);
	  if (i%2)u8g.setScale2x2(); 
	  else u8g.undoScale(); 
	}
	lline[0] = (char*)msg;
	lline[1] = (char*)msg1;
	//	lline[2] =(char*) msg2;
	nameNum[0]=0;
	eepromReadStr(EEaddrIp, oip);
	lline[3] = (char*)"IP:";
	lline[4] = oip;
	draw(0);
	delay(2000);
	if (!digitalRead(ContrastPin)) {
		NOKIAcontrast+=1;
		if (NOKIAcontrast > 0xCF) NOKIAcontrast = 0xB0;
		EEPROM.write(EEaddr1,NOKIAcontrast) ;
		itoa(NOKIAcontrast,title,16);
		lline[2] = title;
		goto ReStart;
	}
  setTimer2();
  setTimer1();
	lline[2] = (char*)msg2;
	draw(0);
}

////////////////////////////////////////
// Clear all buffers and indexes
void clearAll()
{
      title[0] = 0;
      station[0]=0;
    for (int i=1;i<LINES;i++) {lline[i] = NULL;iline[i] = 0;tline[i] = 0;}
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
     }  
}

////////////////////////////////////////
void removeUtf8(byte *characters)
{
  int index = 0;
  while (characters[index])
  {
    if ((characters[index] >= 0xc2)&&(characters[index] <= 0xc3)) // only 0 to FF ascii char
    {
      //      Serial.println((characters[index]));
      characters[index+1] = ((characters[index]<<6)&0xFF) | (characters[index+1] & 0x3F);
      int sind = index+1;
      while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
      characters[sind-1] = 0;

    }
    index++;
  }
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
    byte len;
    char* interp;
    len = strlen(from);
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr_PF(from,PSTR(" - "));
  if (from == nameset) {lline[0] = nameset;lline[1] = NULL;lline[2] = NULL;return;}
  if (interp != NULL)
  {
    from[interp-from]= 0;
    lline[(from==station)?1:3] = from;
    lline[(from==station)?2:4] = interp+3;
  } else
  {
    lline[(from==station)?1:3] = from;
  }

}
////////////////////////////////////////
// parse the karadio received line and do the job
void parse(char* line)
{
  char* ici;
   removeUtf8((byte*)line);
 //////  reset of the esp
   if ((ici=strstr_PF(line,PSTR("VS Version"))) != NULL) setup2();
   else
 ////// Meta title   
   if ((ici=strstr_PF(line,PSTR("META#: "))) != NULL)
   {     
      cleartitle(); 
      strcpy(title,ici+7);    
      separator(title); 
      askDraw = true;
   } else 
    ////// ICY4 Description
    if ((ici=strstr_PF(line,PSTR("ICY4#: "))) != NULL)
    {
//      cleartitle();
      strcpy(genre,ici+7);
      lline[2] = genre;
      askDraw = true;
    } else 
 ////// ICY0 station name
   if ((ici=strstr_PF(line,PSTR("ICY0#: "))) != NULL)
   {
      int len;
      clearAll();
      if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
      separator(station);
      askDraw = true;
   } else
 ////// STOPPED  
   if ((ici=strstr_PF(line,PSTR("STOPPED"))) != NULL)
   {
      digitalWrite(PIN_PLAYING, LOW);
      cleartitle();
      strcpy_P(title,PSTR("STOPPED"));
      separator(title);
      askDraw = true;
   }    
 /////// Station Ip      
   else  
   if ((ici=strstr_PF(line,PSTR("Station Ip: "))) != NULL) 
   {
      eepromReadStr(EEaddrIp, oip);
      if ( strcmp(oip,ici+12) != 0)
        eepromWriteStr(EEaddrIp,ici+12 ); 
   } else
 //////Nameset
   if ((ici=strstr_PF(line,PSTR("MESET#: "))) != NULL)  
   {
      int len;
      strcpy(nameset,ici+8);
      ici = strstr(nameset," ");
      strncpy(nameNum,nameset,ici-nameset+1);
      nameNum[ici - nameset+1] = 0;
      strcpy(nameset,nameset+strlen(nameNum));
      separator(nameset);            
   } else
 //////Playing
   if ((ici=strstr_PF(line,PSTR("YING#"))) != NULL)  
   {
      digitalWrite(PIN_PLAYING, HIGH);
      if (strcmp_P(title,PSTR("STOPPED")) == 0)
      {
        title[0] = 0;
        separator(title);
        askDraw = true;
     }
   } else
   //////Volume
   if ((ici=strstr(line,"VOL#:")) != NULL)  
   {
      volume = atoi(ici+6);
      askDraw = true;;
   } else
 //////Date Time  ##SYS.DATE#: 2017-04-12T21:07:59+01:00
   if ((ici=strstr(line,"SYS.DATE#:")) != NULL)  
   {
      char lstr[30];
      if (*(ici+11) != '2')//// invalid date. try again later
      {
        askDraw = true;
        return;
      }
      strcpy(lstr,ici+11);
      dt = gmtime(&timestamp);
      int year,month,day,hour,minute,second;
      sscanf(lstr,"%04d-%02d-%02dT%02d:%02d:%02d",&(year),&(month),&(day),&(hour),&(minute),&(second));
      dt->tm_year = year; dt->tm_mon = month-1; dt->tm_mday = day;
      dt->tm_hour = hour; dt->tm_min = minute;dt->tm_sec =second;
      dt->tm_year -= 1900;
      timestamp = mktime(dt); 
      syncTime = true;
   }
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
         line[index++] = temp;
				if (index>BUFLEN-1) //break; // small memory so small buffer
        {
          Serial.println(F("overflow"));
         line[index] = 0;
         parse(line);
         index = 0;
        } 
       
	    }
    }  
}

////////////////////////////////////////
// draw all lines
void draw(int xx)
{
  char strsec[30]; 
  dt=gmtime(&timestamp);
  if (x==84)
  sprintf(strsec,"%02d-%02d  %02d:%02d:%02d",(dt->tm_mon)+1,dt->tm_mday, dt->tm_hour, dt->tm_min,dt->tm_sec);
  else
  sprintf(strsec,"%02d-%02d-%04d  %02d:%02d:%02d",(dt->tm_mon)+1,dt->tm_mday,dt->tm_year+1900, dt->tm_hour, dt->tm_min,dt->tm_sec);

	u8g.firstPage();
	do {
		u8g.drawHLine(0,(4*y) - (y/2)-5,x);
    u8g.drawBox(0,0,x-1,y);
		for (int i = 0;i < LINES;i++)
		{
			if (i == 0)u8g.setColorIndex(0);
			else u8g.setColorIndex(1);
			if (i >=3) z = y/2 -3; else z = -1;
			if ((lline[i] != NULL))
      {
			  if (i == 0) 
			  {				
			  	if (nameNum[0] ==0)  u8g.drawStr(1,0,lline[i]+iline[i]);
			  	else 
			  	{
			  		u8g.drawStr(1,0,nameNum);
			  		u8g.drawStr(u8g.getStrPixelWidth(nameNum)-2,0,lline[i]+iline[i]);
			  	}
			  }      
			  else u8g.drawStr(0,y*i+z,lline[i]+iline[i]);
      }
      if (i == VOLUME)
      {        
          u8g.drawFrame(0,y*i+z+(y/2)-1,x-1,3);
          u8g.drawHLine(0,y*i+z+(y/2),((uint16_t)(x*volume)/255));                    
      }  
    }
      //time
//  if ((lline[4] == NULL)||(x==84))
    u8g.drawStr(x/2-(u8g.getStrWidth(strsec)/2),yy-y,strsec); 
	} while( u8g.nextPage() );
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
       len = u8g.getStrWidth(lline[i]+iline[i]);
		   if (i == 0)	 len += u8g.getStrWidth(nameNum) ;
		   if (len > x) 
		    {iline[i]++;askDraw = true;}
		   else 
			  tline[i] = 6;
	   }
	}	
}

#ifdef IR
void nbStation(char nb)
{
  if (strlen(irStr)>=3) irStr[0] = 0;
  uint8_t id = strlen(irStr);
  irStr[id] = nb;
  irStr[id+1] = 0;
}
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
//	    Serial.print(F("Protocol:"));Serial.print(results.protocolNum);Serial.print(F("  value:"));Serial.println(results.value,HEX);
#else
//      Serial.print(F("Protocol:"));Serial.print(results.decode_type);Serial.print(F("  value:"));Serial.println(results.value,HEX);
#endif
//      Serial.print(F("  value:"));Serial.println(results.value,HEX);
		switch(results.value)
		{
			case 0xFF629D: 
			case 0x10EF48B7:	/*(" FORWARD");*/  irStr[0] = 0;Serial.print(F("cli.next\r")); break;

			case 0xFF22DD:
			case 0x10EFA857:
			case 0x10EF42BD: /*(" LEFT");*/  irStr[0] = 0;Serial.print(F("cli.vol-\r"));  break;

			case 0xFF02FD:
			case 0x10EF7887:		/*(" -OK-");*/
			{  
//      state?Serial.print(F("cli.start\r")):Serial.print(F("cli.stop\r"));
        if (strlen(irStr) >0)
        {
          Serial.print(F("cli.play(\""));Serial.print(irStr);Serial.print(F("\")\r"));
          irStr[0] = 0;
        }
        else
        { 
          state?Serial.print(F("cli.start\r")):Serial.print(F("cli.stop\r"));
/*				  if (state)
           Serial.print("cli.start\r");
			  	else
			  		Serial.print("cli.stop\r");
           */
        }
				state = !state;
        irStr[0] = 0;
				break;
			}
			case 0xFFC23D:
			case 0x10EF28D7:
			case 0x10EF827D: /*(" RIGHT");*/ irStr[0] = 0;Serial.print(F("cli.vol+\r"));  break; // volume +
			case 0xFFA857:
			case 0x10EFC837:	/*(" REVERSE");*/ irStr[0] = 0;Serial.print(F("cli.prev\r")); break;
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
			case 0x10EFE817: /*(" *");*/   irStr[0] = 0;Serial.print(F("cli.stop\r")); break;
			case 0xFF4AB5:
			case 0x10EF00FF: /*(" 0");*/ nbStation('0');   break;
			case 0xFF52AD:
			case 0x10EFB847: /*(" #");*/   irStr[0] = 0;Serial.print(F("cli.start\r")); break;
			case 0xFFFFFFFF: /*(" REPEAT");*/break;
			default:;
			/*Serial.println(" other button   ");*/
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

void askTime()
{
    if (itAskTime) // time to ntp. Don't do that in interrupt.
    {
        Serial.print(F("sys.date\r")) ; 
        itAskTime = false;
    }  
}

////////////////////////////////////////
void loop(void) {
#ifdef IR
    translateIR();
#endif
// scrolling control and draw control
    if (loopScroll >=1 ) // 500ms
    { 
      digitalWrite(PIN_LED, !digitalRead(PIN_LED)); // blink led  
      loopScroll = 0;
      if (askDraw) // something to display
      {
        askDraw = false; 
        draw(0); 
      }   
//      else
      scroll(); 
    }
// ntp control and first info demand
     if (loopDate>=5)
     {
         loopDate = 0;
          if (itAskTime)
          {
            if (!syncTime) // time to ntp. Don't do that in interrupt.
            {
              Serial.print(F("\rsys.tzo(\"")) ;Serial.print(TZO);
              Serial.print(F("\")\r")); 
              Serial.print(F("cli.info\r")); // Synchronise the current state   
              itAskTime = false;             
            } 
            else askTime();
          }
      }
}

