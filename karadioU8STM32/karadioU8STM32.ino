/*

  KaradioU8STM32.pde
  Only for STM32 CPU  

If build complain about HardWire.h missing, replace it with Wire.h
It is due to the new arduinoSTM32 since july 2017
  
*/
//-------------------------------------------------------
// Please define your options here
//-------------------------------------------------------
// UnComment the following line if you want the IR remote
#define IR
// UnComment the following lines if you want the rotary encoder
#define RENC
// Uncomment your oled type
//#define oled096  U8GLIB_SSD1306_128X64
#define oled130 U8GLIB_SH1106_128X64

// your timezone offset
#define TZO 1  // comment if the tzo is already given in the esp8266 or change the value
//-------------------------------------------------------


#undef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 256
#include <arduino.h>
#include <MapleFreeRTOS.h>
#include <avr/pgmspace.h>
#include <itoa.h>
#include <time.h>
#include "u8glibConf.h"
#include <EEPROM.h>
#ifdef RENC
#include <ClickEncoder.h>
#endif
#ifdef IR
#include <irmp.h>
#endif


#define SERIALX Serial1
#define PIN_LED LED_BUILTIN
#define PIN_PLAYING PB10
//uncomment REVERSE if you need to invert the play led
#define REVERSE !

// Optional input switchs (to gnd)
#define SWITCH1 PA0
#define SWITCH2 PA1
#define SWITCH3 PA2

#define  BAUD       115200   // any standard serial value: 300 - 115200

// nams <--> num of line
#define STATIONNAME 0
#define STATION1  1
#define STATION2  2
#define IP        3
#define GENRE     2
#define TITLE1    3
#define TITLE2    4

// second before time display in stop state
#define DTIDLE  60

// constants
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

//const byte ContrastPin     = 8;     // D8 low activates the Contrast adjustment


#ifdef IR
  IRMP_DATA irmp_data;
#endif

#ifdef RENC
  ClickEncoder *encoder;
  unsigned timerEncoder = 0;
#endif

bool state = false; // start stop on Ok key
//-----------
#ifdef IR
// Character array pointers
 char  msg2[]       = {"IR+lcd V1.0"}; //
#else
 char  msg2[]     = {"lcd V1.0"}; //
#endif
 char  msg1[]   = {"(c) KaraWin"}; //
 char  msg[]   = {" Karadio"};

#ifndef F_INTERRUPTS
#define F_INTERRUPTS     10000   // interrupts per second, min: 10000, max: 20000, typ: 15000
#endif

// Hardware timer 2 of the STM32
// used for all timers in this software
HardwareTimer timer(2);    

      
// Karadio specific data
#define BUFLEN  180
#define LINES	5
char irStr[4];

char line[SERIAL_RX_BUFFER_SIZE]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char nameset[BUFLEN/2]; // the local name of the station
uint16_t volume;
char nameNum[5] ; // the number of the current station
char futurNum[5] = {"0"}; // the number of the wanted station
char genre[BUFLEN/2]; // the local name of the station
//
// list of screen
enum typeScreen {smain,smain0,svolume,sstation,snumber,stime};
typeScreen stateScreen = smain;
char* lline[LINES] ; // array of ptr of n lines 
uint8_t  iline[LINES] ; //array of index for scrolling
uint8_t  tline[LINES] ;

//
// svolume: display the volume
char aVolume[4] = {"0"}; 
// time string
char strsec[30]; 

// state of the transient screen
uint8_t mTscreen = 1; // 0 dont display, 1 display full, 2 display variable part
//
//sstation: display a list of stations
uint8_t highlight=0;// index in sline of the highlighted station
char sline[BUFLEN] ; // station 
bool playable = true;

unsigned ledcount = 0; // led counter
unsigned timerScreen = 0;
unsigned timerScroll = 0;
unsigned timein = 0;
struct tm *dt;
static bool syncTime = false;
static bool itAskTime = true;
static bool itAskStime = false;

//ip
char oip[20];

uint16_t y ;		//Height of a line
uint16_t yy;		//Height of screen
uint16_t x ;		//Width
uint16_t z ;		// an internal offset for y

byte NOKIAcontrast;                 // LCD initialization contrast values B0 thru BF
time_t timestamp = 0;

// switches state
bool Switch1 = false;
bool Switch2 = false;
bool Switch3 = false;


//U8GLIB_SSD1306_128X64 u8g;
#ifdef oled096
U8GLIB_SSD1306_128X64 u8g;
#else
#ifdef oled130
U8GLIB_SH1106_128X64 u8g;
#endif
#endif



// init timer 2 for irmp led screen etc
void timer2_init ()
{
    timer.pause();
//    timer.setPrescaleFactor( ((F_CPU / F_INTERRUPTS)/8) - 1);
//    timer.setOverflow(7);
    timer.setPrescaleFactor(  ((F_CPU / F_INTERRUPTS)/10) );
    timer.setOverflow(9);
    timer.setChannelMode(TIMER_CH1,TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 0);  // Interrupt  after each update
    timer.attachInterrupt(TIMER_CH1,TIM2_IRQHandler);
        // Refresh the timer's count, prescale, and overflow
    timer.refresh();
    // Start the timer counting
    timer.resume();
}

void TIM2_IRQHandler()     // Timer2 Interrupt Handler
{
#ifdef IR
      (void) irmp_ISR(); // call irmp ISR
#endif      
#ifdef RENC
     if (++timerEncoder >= (F_INTERRUPTS/1000)) //1msec = 1000 Hz
     {
        timerEncoder = 0;
        encoder->service(); //call encoder engine
     }
#endif 
// one second tempo: led blink, date/time and screen and scroll 
      if ( ++ledcount >= (F_INTERRUPTS)) //1 sec
      {
         ledcount = 0;// 
         digitalWrite(PIN_LED, !digitalRead(PIN_LED));  
         // Time compute
         timestamp++;  // time update  
        /* if (state) timein = 0; // only on stop state
         else*/ timein++;
		 
         if (((timein % DTIDLE)==0)&&(!state)  ) {
            if ((timein % (30*DTIDLE))==0){ itAskTime=true;timein = 0;} // synchronise with ntp every x*DTIDLE
            if (stateScreen != stime) {itAskStime=true;} // start the time display
          } 
          if ((stateScreen == stime)||(stateScreen == smain)) { mTscreen = 1; } // display time
          if (!syncTime) itAskTime=true; // first synchro if not done
// Other slow timers        
          timerScreen++;
          timerScroll++;
      }
}


//-------------------------------------------------------
// Main task used for display the screen and blink the led
static void mainTask(void *pvParameters) {
Serial.println(F("mainTask"));
  while (1) {
    drawScreen();   
    if (itAskTime) // time to ntp. Don't do that in interrupt.
    {
      askTime();
      itAskTime = false;
    }

    if (itAskStime) // time start the time display. Don't do that in interrupt.
    {    
      Screen(stime);
      itAskStime = false;
    }
    
    if (timerScreen >= 3) // 3 sec
    {
        timerScreen = 0;
        if ((stateScreen != smain)&&(stateScreen != stime))
        {
          Screen(smain0);  //Back the the main screen
          // Play the changed station on return to main screen
          if (playable && ( atoi(futurNum)!= atoi(nameNum))) playStation(futurNum);
        }
    }
    vTaskDelay(1);
     if ( timerScroll >= 1) //
     {
        if (stateScreen == smain) scroll();  
        timerScroll = 0;
     }    
    vTaskDelay(200);
//    drawScreen();      
  }
}

//-------------------------------------------------------
// Uart task: receive the karadio log and parse it.
static void uartTask(void *pvParameters) {

  Serial.println(F("uartTask\n"));
  vTaskDelay(200);
  SERIALX.print(F("\r")); // cleaner
  SERIALX.print(F("\r")); // cleaner
  #ifdef TZO
  setTzo();
  #endif
  vTaskDelay(1);
  SERIALX.print(F("cli.info\r")); // Synchronise the current state
  itAskTime = true;
  vTaskDelay(1);
  while (1){
    serial(); 
    recSwitch(); // read the switches
    vTaskDelay(1);
  }
}


//-------------------------------------------------------
// IO task: receive the IR remote and encoder control and parse it.
static void ioTask(void *pvParameters) {
//  Serial.println(F("ioTask"));
  while (1){
#ifdef IR    
    translateIR() ;
#endif     
#ifdef RENC
    translateENC();
#endif      
    vTaskDelay(100);
  }
}

////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.
void setup2(bool ini)
{
  clearAll();
  lline[0] = (char*)msg;
  lline[1] = (char*)msg1;
  lline[2] = (char*)msg2;
  nameNum[0]=0;
  eepromReadStr(EEaddrIp, oip);
  lline[3] = (char*)"IP:";
  lline[4] = oip;
  drawFrame();
  digitalWrite(PIN_PLAYING, REVERSE LOW);
  if (ini) lline[STATIONNAME] = nameset;
}

void setFont(const u8g_fntpgm_uint8_t *font)
{
  u8g.setFont(font) ; 
  u8g.setFontPosTop();
  y = u8g.getFontLineSpacing();  
}

void setup(void) {
   portBASE_TYPE s1, s2, s3;
//uint8_t u8g_com_hw_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);
//U8GLIB_SSD1306_128X64 u8g((u8g_com_fnptr)u8g_com_hw_i2c_fn);
  SERIALX.begin(BAUD);
  Serial.begin(BAUD);
delay(2000);
Serial.println("ready");

uint8_t u8g_com_hw_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);
#ifdef oled096
U8GLIB_SSD1306_128X64 u8g((u8g_com_fnptr)u8g_com_hw_i2c_fn,U8G_I2C_OPT_NONE);
#else
#ifdef oled130
U8GLIB_SH1106_128X64 u8g((u8g_com_fnptr)u8g_com_hw_i2c_fn,U8G_I2C_OPT_NONE);
#endif
#endif

#ifdef IR
    irmp_init();   // initialize irmp
 irStr[0] = 0;
#endif  
#ifdef RENC
encoder = new ClickEncoder(PB12, PB13, PB14);
#endif
   pinMode(PIN_LED, OUTPUT);
	 pinMode(PIN_PLAYING, OUTPUT);
	 digitalWrite(PIN_PLAYING,REVERSE LOW); 

   pinMode(SWITCH1, INPUT_PULLUP);
   pinMode(SWITCH2, INPUT_PULLUP);
   pinMode(SWITCH3, INPUT_PULLUP);
   
ReStart:  // Come back here if LCD contract is changed
    // Read the EEPROM to determine if display is using a custom contrast value
  NOKIAcontrast = EEPROM.read(EEaddr1);
    // Set the Nokia LCD Contrast to default or reset if EEPROM is corrupt or set to new value
  if (NOKIAcontrast  < 0xB0 || NOKIAcontrast > 0xCF) NOKIAcontrast = 0xB8;
	u8g.setContrast(2*(NOKIAcontrast-0x80));
  
  Serial.print(F("Width: ")); Serial.println(u8g.getWidth());
  
//  u8g.setFontRefHeightExtendedText();
//  u8g.setDefaultForegroundColor();
  if (u8g.getWidth() == 84)
    setFont(u8g_font_5x8);
  else 
    setFont(u8g_font_6x10);
// Serial.print("Spacing y: "); Serial.println(y);
	yy = u8g.getHeight();
// Serial.print("Spacing yy: "); Serial.println(yy);
	x = u8g.getWidth();
// Serial.print("Spacing x: "); Serial.println(x);
	z = 0; 
	clearAll();
 
//LOGO
Serial.println(F("Logo in\n"));
    u8g.firstPage();
    do {
      u8g.drawXBMP( x/2-logo_width/2, yy/2-logo_height/2, logo_width, logo_height, logo_bits);
    } while( u8g.nextPage() );
delay(3000);
Serial.println(F("Logo done\n"));
	lline[0] = (char*)msg;
	lline[1] = (char*)msg1;
	//	lline[2] =(char*) msg2;
	nameNum[0]=0;
	eepromReadStr(EEaddrIp, oip);
	lline[3] = (char*)"IP:";
	lline[4] = oip;
	lline[2] = (char*)msg2;
	drawFrame();
  delay(1000);
  s1=xTaskCreate(mainTask, NULL, configMINIMAL_STACK_SIZE + 350, NULL, tskIDLE_PRIORITY + 1, NULL);
  s2=xTaskCreate(uartTask, NULL, configMINIMAL_STACK_SIZE +300, NULL, tskIDLE_PRIORITY + 2, NULL);  
  s3=xTaskCreate(ioTask, NULL, configMINIMAL_STACK_SIZE +220, NULL, tskIDLE_PRIORITY + 1, NULL);
 
  if ( s1 != pdPASS || s2 != pdPASS || s3 != pdPASS ) {
    Serial.println(F("Task or Semaphore creation problem.."));
    while(1);
  }
  timer2_init(); // initialize timer2
  // Start the task scheduler - here the process starts
  vTaskStartScheduler();
  Serial.println(F("Started")); 
  // In case of a scheduler error
  Serial.println(F("Die - insufficient heap memory?"));
  while(1);   
  
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
//    byte len;
    char* interp;
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr_P(from,PSTR(" - "));
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
  static bool dvolume = true; // display volume screen
  char* ici;
   removeUtf8((byte*)line);
   Serial.println(line);
 //////  reset of the esp
   if ((ici=strstr_P(line,PSTR("VS Version"))) != NULL) 
   {
      clearAll();
      setup2(true);
   }
   else
 ////// Meta title  ##CLI.META#: 
   if ((ici=strstr_P(line,PSTR("META#: "))) != NULL)
   {     
      cleartitle(); 
      strcpy(title,ici+7);    
      separator(title); 
   } else 
    ////// ICY4 Description  ##CLI.ICY4#:
    if ((ici=strstr_P(line,PSTR("ICY4#: "))) != NULL)
    {
//      cleartitle();
      strcpy(genre,ici+7);
      lline[2] = genre;
    } else 
 ////// ICY0 station name   ##CLI.ICY0#:
   if ((ici=strstr_P(line,PSTR("ICY0#: "))) != NULL)
   {
      int len;
      clearAll();
      if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
      separator(station);
   } else
 ////// STOPPED  ##CLI.STOPPED#  
   if ((ici=strstr_P(line,PSTR("STOPPED"))) != NULL)
   {
      digitalWrite(PIN_PLAYING, REVERSE LOW);
      state = false;
      cleartitle();
      strcpy_P(title,PSTR("STOPPED"));
       lline[TITLE1] = title;
   }    
   else  
 /////// Station Ip       ' Station Ip:     
   if ((ici=strstr_P(line,PSTR("Station Ip: "))) != NULL) 
   {
      eepromReadStr(EEaddrIp, oip);
      if ( strcmp(oip,ici+12) != 0){
		EEPROM.init();
        eepromWriteStr(EEaddrIp,ici+12 ); 
		}
   } else
 //////Namesett    ##CLI.NAMESET#:
   if ((ici=strstr_P(line,PSTR("MESET#: "))) != NULL)  
   {
      int len;
      strcpy(nameset,ici+8);
      ici = strstr(nameset," ");
     if (ici != NULL)
     {
       clearAll();
       strncpy(nameNum,nameset,ici-nameset+1);
       nameNum[ici - nameset+1] = 0; 
       strcpy (futurNum,nameNum);     
     }
     strcpy(nameset,nameset+strlen(nameNum));
     lline[STATIONNAME] = nameset;
//     separator(nameset);            
   } else
 //////Playing    ##CLI.PLAYING#
   if ((ici=strstr_P(line,PSTR("YING#"))) != NULL)  
   {
      digitalWrite(PIN_PLAYING, REVERSE HIGH);
      state = true;
      if (stateScreen == stime) Screen(smain0);      
      if (strcmp_P(title,PSTR("STOPPED")) == 0)
      {
        title[0] = 0;
        separator(title);
     }
   } else
   //////Volume   ##CLI.VOL#:
   if ((ici=strstr(line,"VOL#:")) != NULL)  
   {
      volume = atoi(ici+6);
      strcpy(aVolume,ici+6); 
      if (dvolume)
         Screen(svolume); 
      else 
         dvolume = true;
      timerScreen = 0;
   } else
  //////Volume offset    ##CLI.OVOLSET#:
   if ((ici=strstr(line,"OVOLSET#:")) != NULL)  
   {
      dvolume = false; // don't show volume on start station
   }else
 //////list station   #CLI.LISTINFO#:
   if ((ici=strstr(line,"LISTINFO#:")) != NULL)
   {
      char* ptrstrstr;
      strcpy(sline, ici+10);
      ptrstrstr = strstr(sline,",");
      if (ptrstrstr != NULL)  *ptrstrstr =0;
      Screen(sstation);
      timerScreen = 0;      
   } else
 //////Date Time  ##SYS.DATE#: 2017-04-12T21:07:59+01:00
   if ((ici=strstr(line,"SYS.DATE#:")) != NULL)  
   {
      char lstr[30];
      if (*(ici+11) != '2')//// invalid date. try again later
      {
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
// Receive the switches
void recSwitch()
{
  bool val;
  val =  digitalRead(SWITCH1);

  if (val != Switch1)
  {
    Switch1 = val;
    if (!val) // do the action
      startStop();
  }
  val =  digitalRead(SWITCH2);
  if (val != Switch2)
  {
    Switch2 = val;
    if (!val) // do the action
      stationPlus();
  }
  val =  digitalRead(SWITCH3);
  if (val != Switch3)
  {
    Switch3 = val;
    if (!val) // do the action
      stationMinus();
  }    
}
////////////////////////////////////////
// receive the esp8266 stream
void serial() 
{
    int temp;
    static unsigned  Rbindex = 0; // receive buffer Rbindex
//    Serial.println(F("Serial"));
//    if (SERIALX.available() == 0) return;
    while ((temp=SERIALX.read()) != -1)
    {
      switch (temp)
      {
        case '\\':break;
        case '\n' : 
        case '\r' : if (Rbindex == 0) break;
        line[Rbindex] = 0; // end of string
        parse(line);
        Rbindex = 0;
        break;
        default : // put the received char in line
        line[Rbindex++] = temp;
        if (Rbindex>SERIAL_RX_BUFFER_SIZE-1) 
        {
          Serial.println(F("overflow"));
         line[Rbindex] = 0;
         parse(line);
         Rbindex = 0;
        }       
      }
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
       len = u8g.getStrWidth(lline[i]+iline[i]);
		   if (i == 0)	 len += u8g.getStrWidth(nameNum) ;
		   if (len > x) 
		    {iline[i]++;}
		   else 
			  tline[i] = 6;
	   }
	}	
}

////////////////////////////
// Action functions
void askTime()
{
    irStr[0] = 0;
    SERIALX.print(F("sys.date\r")) ;   
}
void setTzo()
{
    irStr[0] = 0;
    SERIALX.print(F("sys.tzo(\"")) ;SERIALX.print(TZO);SERIALX.print(F("\")\r"));     
}
void startStop()
{   
    state?stopStation():startStation();
}  
void stopStation()
{
    irStr[0] = 0;
    SERIALX.print(F("cli.stop\r")) ; 
}
void startStation()
{
    irStr[0] = 0;
    SERIALX.print(F("cli.start\r")) ; 
}
void stationOk()
{
       if (strlen(irStr) >0)
          playStation(irStr);
        else
        {
          if (stateScreen == sstation) 
            playStation(futurNum);
          else currentStation();
        }  
        irStr[0] = 0;  
}
void incrVol()
{
    irStr[0] = 0;
    volume++;
    if (volume > 254) volume = 254;
    SERIALX.print(F("cli.vol(\"")) ;SERIALX.print(volume);SERIALX.print(F("\")\r")); 
}
void decrVol()
{
    irStr[0] = 0;
    volume--;
    if (volume < 0) volume = 0;
    SERIALX.print(F("cli.vol(\"")) ;SERIALX.print(--volume);SERIALX.print(F("\")\r")); 
}
void setVol(int16_t nvol)
{
    irStr[0] = 0;
    if (nvol > 254) nvol = 254;
    if (nvol < 0) nvol = 0;
    SERIALX.print(F("cli.vol(\"")) ;SERIALX.print(nvol);SERIALX.print(F("\")\r")); 
}

void minusVol()
{
    irStr[0] = 0;
    SERIALX.print(F("cli.vol-\r")) ; 
}
void plusVol()
{
    irStr[0] = 0;
    SERIALX.print(F("cli.vol+\r")) ; 
}

void playStation(char* num){
//  Serial.println(num);
  SERIALX.print(F("cli.play(\""));SERIALX.print(num);SERIALX.print(F("\")\r"));
  irStr[0] = 0;
}

//////////////////////
// ask info on station
void currentStation()
{
  uint8_t stat;
  irStr[0] = 0;
  stat = atoi(futurNum);
//  Serial.println(stat);
  SERIALX.print(F("cli.list(\""));
  SERIALX.print(stat);
  SERIALX.print(F("\")\r"));
}
void stationMinus(){
  uint8_t stat;
  irStr[0] = 0;
  stat = atoi(futurNum)-1;
  if (stat >254) stat = 0;
//  Serial.println(stat);
  SERIALX.print(F("cli.list(\""));
  SERIALX.print(stat);
  SERIALX.print(F("\")\r"));
  sprintf(futurNum,"%d",stat);
}
void stationPlus(){
  uint8_t stat;
  irStr[0] = 0;
  stat = atoi(futurNum)+1;
  if (stat >254) stat = 0;
//  Serial.println(stat);
  SERIALX.print(F("cli.list(\""));
  SERIALX.print(stat);
  SERIALX.print(F("\")\r"));
  sprintf(futurNum,"%d",stat);
}

void stationNum(uint8_t thisone)
{
  irStr[0] = 0;
  if (thisone >254) thisone = 0;
//  Serial.println(stat);
  SERIALX.print(F("cli.list(\""));
  SERIALX.print(thisone);
  SERIALX.print(F("\")\r"));
  sprintf(futurNum,"%d",thisone);
}
////////////////////////////
#ifdef IR
// a number of station in progress...
void nbStation(char nb)
{
  Screen(snumber);
  timerScreen = 0;
  if (strlen(irStr)>=3) irStr[0] = 0;
  uint8_t id = strlen(irStr);
  irStr[id] = nb;
  irStr[id+1] = 0;
}
////////////////////////////
// takes action based on IR code received
void translateIR() 
//  KEYES Remote IR codes (NEC P01)
//  and Ocean Digital remote (NEC P07)
{
  if (irmp_get_data (&irmp_data))
  {
    uint32_t code =   (irmp_data.address<<8)|irmp_data.command;
//  Uncomment to see a new remote control  
//    Serial.println(irmp_protocol_names[irmp_data.protocol]);
//    Serial.println(irmp_data.address,HEX);
//    Serial.println(irmp_data.command,HEX);
//    Serial.println(irmp_data.flags,HEX);
//    Serial.println(code,HEX);
//    Serial.println();
    
    if (!(irmp_data.flags& IRMP_FLAG_REPETITION)) // avoid repetition
    switch(code)
    {
      case 0xFF0046: 
      case 0xF70812:  /*(" FORWARD");*/  stationPlus(); break;

      case 0xFF0044:
      case 0xF70842:
      case 0xF70815: /*(" LEFT");*/  minusVol();  break;

      case 0xFF0040:
      case 0xF7081E:    /*(" -OK-");*/ startStop();//stationOk();     break;
      case 0xFF0043:
      case 0xF70841:
      case 0xF70814: /*(" RIGHT");*/ plusVol();     break; // volume +
      case 0xFF0015:
      case 0xF70813:  /*(" REVERSE");*/ stationMinus(); break;
      case 0xFF0016:
      case 0xF70801: /*(" 1");*/ nbStation('1');   break;
      case 0xFF0019:
      case 0xF70802: /*(" 2");*/ nbStation('2');   break;
      case 0xFF000D:
      case 0xF70803: /*(" 3");*/ nbStation('3');   break;
      case 0xFF000C:
      case 0xF70804: /*(" 4");*/ nbStation('4');   break;
      case 0xFF0018:
      case 0xF70805: /*(" 5");*/ nbStation('5');   break;
      case 0xFF005E:
      case 0xF70806: /*(" 6");*/ nbStation('6');   break;
      case 0xFF0008:
      case 0xF70807: /*(" 7");*/ nbStation('7');   break;
      case 0xFF001C:
      case 0xF70808: /*(" 8");*/ nbStation('8');   break;
      case 0xFF005A:
      case 0xF70809: /*(" 9");*/ nbStation('9');   break;
      case 0xFF0042:
      case 0xF70817: /*(" *");*/   startStation();   break;
      case 0xFF0052:
      case 0xF70800: /*(" 0");*/ nbStation('0');   break;
      case 0xFF004A:
      case 0xF7081D: /*(" #");*/  stopStation();    break;
      default:;
      /*SERIALX.println(F(" other button   "));*/
    }// End Case

    if ((irmp_data.flags& IRMP_FLAG_REPETITION)) // repetition
    switch(code)
    {
      case 0xFF0046: 
      case 0xF70812:  /*(" FORWARD");*/  stationPlus(); break;
      case 0xFF0015:
      case 0xF70813:  /*(" REVERSE");*/ stationMinus(); break;
      case 0xFF0044:
      case 0xF70842:
      case 0xF70815: /*(" LEFT");*/  decrVol();  break;
      case 0xFF0043:
      case 0xF70841:
      case 0xF70814: /*(" RIGHT");*/ incrVol();  break; // volume +
      default:;
    }   
  }
} //END translateIR
#endif

////////////////////////////
#ifdef RENC
void translateENC()
{
  enum modeStateEncoder { encVolume, encStation } ;
  static modeStateEncoder stateEncoder = encVolume;
  int16_t newValue = 0;
  ClickEncoder::Button newButton = ClickEncoder::Open;
  static int16_t oldValue = 0;
//  static ClickEncoder::Button oldButton = ClickEncoder::Open;
/*    
 *   ClickEncoder::Open
 *   ClickEncoder::Closed
 *   ClickEncoder::Pressed
 *   ClickEncoder::Held
 *   ClickEncoder::Released
 *   ClickEncoder::Clicked
 *   ClickEncoder::DoubleClicked
 */
  newValue = - encoder->getValue();
  newButton = encoder->getButton();
  
  if (newValue != 0) 
  {
//    Serial.print("Encoder: ");Serial.println(newValue);
    // reset our accelerator
    if ((newValue >0)&&(oldValue<0)) oldValue = 0;
    if ((newValue <0)&&(oldValue>0)) oldValue = 0;
  }
  else
  {
    // lower accelerator 
    if (oldValue <0) oldValue++;
    if (oldValue >0) oldValue--;
  }
    
  if (newButton != ClickEncoder::Open)
  {    
//    Serial.print("Button: ");Serial.println(newButton);
    if (newButton == ClickEncoder::Clicked) {startStop();}
    if (newButton == ClickEncoder::DoubleClicked) 
    {
        (stateScreen==smain)?Screen(stime):Screen(smain0);
    } 
    if (newButton == ClickEncoder::Held)
    {
      if (stateScreen  != sstation)  currentStation();
      if (newValue != 0)
        stationNum(atoi(futurNum)+newValue);
        timerScreen = 0;
    }
  }
  else{
  if ((stateScreen  != sstation)&&(newValue != 0))
  {    
//    Serial.print("Value: ");Serial.println(newValue);
//    Serial.print("Volume: ");Serial.println(volume+newValue+(oldValue*2));
    setVol(volume+newValue+(oldValue*3));
  } 
  if ((stateScreen  == sstation)&&(newValue != 0))
  {    
//    Serial.print("Value: ");Serial.println(newValue);
      stationNum(atoi(futurNum)+newValue);
  } 
  }
  oldValue += newValue;
}
#endif

////////////////////////////
// Change the current screen
////////////////////////////
void Screen(typeScreen st){
  if (stateScreen != st)
  {
#ifdef IR
// if a number is entered, play it.
    if (strlen(irStr) >0)
       playStation(irStr);
// clear the number       
    irStr[0] = 0;
#endif
  }
  else
    if (mTscreen == 0) mTscreen = 2;
  stateScreen = st;  
  timein = 0;
  mTscreen = 1;
}

// Bottom of screens
void screenBottom()
{
//VOLUME
    u8g.drawFrame(0,yy-3,x-1,3);
    u8g.drawHLine(0,yy-2,((uint16_t)(x*volume)/255));                         
//TIME
//  if ((lline[4] == NULL)||(x==84))
    setFont(u8g_font_5x8);
    u8g.drawStr(x/2-(u8g.getStrWidth(strsec)/2),yy-y-4,strsec);   
}

////////////////////////////////////////
// draw all lines
void drawFrame()
{
  dt=gmtime(&timestamp);
  if (x==84)
  sprintf(strsec,"%02d-%02d  %02d:%02d:%02d",(dt->tm_mon)+1,dt->tm_mday, dt->tm_hour, dt->tm_min,dt->tm_sec);
  else
  sprintf(strsec,"%02d-%02d-%04d  %02d:%02d:%02d",(dt->tm_mon)+1,dt->tm_mday,dt->tm_year+1900, dt->tm_hour, dt->tm_min,dt->tm_sec);
  if (u8g.getWidth() == 84)
    setFont(u8g_font_5x8);
  else 
    setFont(u8g_font_6x10);
  u8g.setColorIndex(1); 

//    y = u8g.getFontLineSpacing();
//    u8g.setFontRefHeightText(); 
  u8g.firstPage();
  do {
    u8g.drawHLine(0,(4*y) - (y/2)-1,x);
    u8g.drawBox(0,0,x-1,y);
    for (int i = 0;i < LINES;i++)
    {
      if (i == 0)u8g.setColorIndex(0);
      else u8g.setColorIndex(1);
      if (i >=3) z = y/2+2 ; else z = 1;
      if ((lline[i] != NULL))
      {
//Serial.print("Liney: ");Serial.println(y);  
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

    }
    screenBottom();    
  } while( u8g.nextPage() );
   mTscreen = 0;
}

//////////////////////////
void drawTTitle(char* ttitle)
{
        setFont(u8g_font_9x15); 
        uint16_t xxx = (x/2)-(u8g.getStrWidth(ttitle)/2);
        u8g.setColorIndex(1);  
        u8g.drawBox(0,0,x,u8g.getFontLineSpacing()+1); 
        u8g.setColorIndex(0);  
        u8g.drawStr(xxx,1,ttitle);
        u8g.setColorIndex(1);  
}
#ifdef IR
////////////////////
// draw the number entered from IR
void drawNumber()
{
  char ststr[] = {"Number"};
  if(mTscreen)
  {
     u8g.firstPage();
     do {  
        drawTTitle(ststr);   
        uint16_t xxx ;
        setFont(u8g_font_ncenR18);  
        xxx = (x/2)-(u8g.getStrWidth(irStr)/2); 
        u8g.drawStr(xxx,yy/3, irStr);        
        screenBottom();  
     } while( u8g.nextPage() ); 
  }      
  mTscreen = 0;    
}
#endif
////////////////////
// draw the station screen
void drawStation()
{
  char ststr[] = {"Station"};
  char* ddot;
  int16_t len;
  if (mTscreen)
  { 
    ddot = strstr(sline,":");
    if (ddot != NULL)
    {
      *ddot=0; 
      char* ptl = sline+1;
Serial.print("Station: ");Serial.println(sline);        
      while ( *ptl == 0x20){*ptl = '0';ptl++;}
      ptl = ++ddot;
      while ( *ptl == 0x20){ddot++;ptl++;}
      if (strcmp(ddot,"not defined")==0) // don't start an undefined station
          playable = false; 
      else 
        playable = true;             
      strcpy (futurNum,sline+1);       
    }      
    u8g.firstPage();
    do {  
        drawTTitle(ststr);        
        if (ddot != NULL)
        {
          u8g.drawStr((x/2)-(u8g.getStrWidth(sline+1)/2),yy/3, sline+1);
          len = (x/2)-(u8g.getStrWidth(ddot)/2);
          if (len <0) len = 0;
          u8g.drawStr(len,yy/3+y, ddot);
        }
        screenBottom();  
     } while( u8g.nextPage() );  
  }        
  mTscreen = 0;    
}
////////////////////
// draw the volume screen
void drawVolume()
{
  char vlstr[] = {"Volume"}; 
   if (mTscreen){
      u8g.firstPage();
      do { 
        drawTTitle(vlstr) ;  
        uint16_t xxx;
        setFont(u8g_font_ncenR18);  
        xxx = (x/2)-(u8g.getStrWidth(aVolume)/2);     
        u8g.drawStr(xxx,yy/3,aVolume);
        screenBottom();   
      } while( u8g.nextPage() );             
    }      
    mTscreen = 0; 
}

void drawTime()
{
  char strdate[23];
  char strtime[20];
//  char strsec[3]; 
  unsigned len;
  if (mTscreen)
  {
    u8g.firstPage();
    do {   
        dt=gmtime(&timestamp);
        sprintf(strdate,"%02d-%02d-%04d", (dt->tm_mon)+1, dt->tm_mday, dt->tm_year+1900);
        sprintf(strtime,"%02d:%02d:%02d", dt->tm_hour, dt->tm_min,dt->tm_sec);
        drawTTitle(strdate);            
        //setFont(u8g_font_9x15);  
        setFont(u8g_font_ncenR18);  
        u8g.drawStr((x/2)-(u8g.getStrWidth(strtime)/2),yy/3+4,strtime); 
        // draw ip
        setFont(u8g_font_5x8);
        eepromReadStr(EEaddrIp, strtime);
        sprintf(strdate,"IP: %s",strtime);
//        len = u8g.getStrWidth(strdate);
        u8g.drawStr(4,yy-u8g.getFontLineSpacing(),strdate);
    } while( u8g.nextPage() ); 
  }        
  mTscreen = 0;       
}


////////////////////
// Display a screen on the lcd
void drawScreen()
{
  switch (stateScreen)
  {
    case smain0:  // force the draw of the complete screen
//      u8g.clearScreen();
      drawFrame();
      stateScreen = smain;
      break;
    case svolume:
      drawVolume();
      break;
    case sstation:
      drawStation(); 
      break; 
    case stime:
      drawTime(); 
//      drawSecond();
      break;     
    case snumber:
#ifdef IR    
      drawNumber();
#endif      
      break;
    default: 
      drawFrame();       
  }
}
////////////////////////////////////////
void loop(void) { // not used on freertos
}

