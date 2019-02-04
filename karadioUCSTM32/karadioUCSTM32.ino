/*

  KaradioucSTM32.pde
  Only for STM32 CPU
*/
//-------------------------------------------------------
// UnComment the following lines if you want the IR remote
#define IR
// UnComment the following lines if you want the rotary encoder
#define RENC
//-------------------------------------------------------
// your timezone offset
#define TZO 1

#undef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 256
#include <MapleFreeRTOS.h>
#include <time.h>
#include "ucglibConf.h"
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
#define TIME      8

// second before time display in stop state
#define DTIDLE  60

// constants
const int  BAUD            = 115200;  // any standard serial value: 300 - 115200
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

#ifdef IR
const char  msg[]  PROGMEM    = {"Karadio IR+clcd V1.2"}; //
#else
const char  msg[] PROGMEM     = {"Karadio clcd V1.2"}; //
#endif
const char  msg1[] PROGMEM   = {"(c) KaraWin"}; //
const char  msg2[] PROGMEM   = {"https://hackaday.io/project/11570-wifi-webradio-with-esp8266-and-vs1053"};

#ifdef IR
  IRMP_DATA irmp_data;
#endif

#ifdef RENC
  ClickEncoder *encoder;
  unsigned timerEncoder = 0;

#endif

#ifndef F_INTERRUPTS
#define F_INTERRUPTS     10000   // interrupts per second, min: 10000, max: 20000, typ: 15000
#endif

// Hardware timer 2 of the STM32
// used for all timers in this software
HardwareTimer timer(2);    

// Karadio specific data
#define BUFLEN  256
#define LINES	9
char irStr[4];
bool state = false; // start stop on Ok ir key
char line[SERIAL_RX_BUFFER_SIZE]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char genre[BUFLEN];  // received genre
char nameset[BUFLEN]= {"0"}; ; // the local name of the station
int16_t volume;
char nameNum[5] ; // the number of the current station
char futurNum[5] = {"0"}; // the number of the wanted station
time_t timestamp = 0;

//
// list of screen
enum typeScreen {smain,smain0,svolume,sstation,snumber,stime};
typeScreen stateScreen = smain;
//
// data for smain:the main screen to display
char* lline[LINES] ; // array of ptr of n lines 
uint8_t  iline[LINES] ; //array of index for scrolling
uint8_t  tline[LINES] ; // tempo at end or begin
uint8_t  mline[LINES] ; // mark to display
//
// svolume: display the volume
char aVolume[4] = {"0"}; 

// state of the transient screen
uint8_t mTscreen = 1; // 0 dont display, 1 display full, 2 display variable part
//
//sstation: display a list of stations
uint8_t highlight=0;// index in sline of the highlighted station
char sline[BUFLEN] ; // station 
bool playable = true;
//

unsigned ledcount = 0; // led counter
unsigned timerScreen = 0;
unsigned timerScroll = 0;
unsigned timein = 0;
struct tm *dt;
bool syncTime = false;
bool itAskTime = true;
bool itAskStime = false;
  
// ip
char oip[20];

int16_t y ;		//Height between line
int16_t yy;		//Height of screen
int16_t x ;		//Width of the screen
int16_t z ;		// an internal offset for y

void Screen(typeScreen st);

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
// one second tempo: led blink, date/time and scrren and scroll 
      if ( ++ledcount >= (F_INTERRUPTS)) //1 sec
      {
         ledcount = 0;// 
         digitalWrite(PIN_LED, !digitalRead(PIN_LED));  
         // Time compute
         timestamp++;  // time update  
        /* if (state) timein = 0; // only on stop state
         else*/ timein++;

         if (((timein % DTIDLE)==0)&&(!state)  ) 
		     {
            if ((timein % (30*DTIDLE))==0)
			      { 
				      itAskTime=true;
			      } // synchronise with ntp every x*DTIDLE
            if (stateScreen != stime) {itAskStime=true;} // start the time display
          } 

          if (((timestamp%60)==0)&&(stateScreen == stime)) { mTscreen = 1; }
          if (stateScreen == smain) { markDraw(TIME); }
          if (!syncTime) itAskTime=true; // first synchro if not done
// Other slow timers        
          timerScreen++;
          timerScroll++;
      }
}

//-------------------------------------------------------
// Main task used for display the screen and blink the led
static void mainTask(void *pvParameters) {
//Serial.println(F("mainTask"));
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
    vTaskDelay(100);
//    drawScreen();      
  }
}

//-------------------------------------------------------
// Uart task: receive the karadio log and parse it.
static void uartTask(void *pvParameters) {

//  Serial.println(F("uartTask"));
  vTaskDelay(200);
  SERIALX.print(F("\r")); // cleaner
  SERIALX.print(F("\r")); // cleaner
  setTzo();
  vTaskDelay(1);
  SERIALX.print(F("cli.info\r")); // Synchronise the current state
  itAskTime = true;
  vTaskDelay(1);
  while (1){
    serial(); 
    vTaskDelay(1);
  }
}


//-------------------------------------------------------
// IR task: receive the IR and encoder  control and parse it.
static void ioTask(void *pvParameters) {
//  Serial.println(F("irTask"));
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
// setup on esp reset
void setup2(bool ini)
{

  lline[0] = (char*)msg;
  lline[1] = (char*)msg1;
  lline[2] = (char*)msg2;

  eepromReadStr(EEaddrIp, oip);
  lline[3] = (char*)"IP:";
  lline[4] = oip;
  drawFrame();
  digitalWrite(PIN_PLAYING, LOW);
  if (ini) lline[STATIONNAME] = nameset;
}

//Setup all things, check for contrast adjust and show initial page.
void setup(void) {
   char  msg3[] = {"Karadio"};
   portBASE_TYPE s1, s2, s3;
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PLAYING, OUTPUT);
  digitalWrite(PIN_PLAYING, LOW);
   SERIALX.begin(BAUD);
   Serial.begin(BAUD);
#ifdef IR
    irmp_init();   // initialize irmp
 irStr[0] = 0;
#endif  
#ifdef RENC
encoder = new ClickEncoder(PB7, PB8, PB9);
#endif
Serial.println(F("Setup"));
// start the graphic library  
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
  ucg.setRotate90();
  ucg.setFont(ucg_font_6x13_mf);
  ucg.setFontPosTop();
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
    delay(500);
    if (i%2) ucg.setScale2x2(); 
    else ucg.undoScale(); 
    ucg.clearScreen();
  }
  ucg.undoScale(); 
  
  setup2(false);
  
  drawFrame(); 
  s1=xTaskCreate(mainTask, NULL, configMINIMAL_STACK_SIZE + 320, NULL, tskIDLE_PRIORITY + 1, NULL);
  s2=xTaskCreate(uartTask, NULL, configMINIMAL_STACK_SIZE +250, NULL, tskIDLE_PRIORITY + 2, NULL);
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
	  for (int i=1;i<LINES;i++) {lline[i] = NULL;iline[i] = 0;tline[i] = 0;mline[i]=1;}
}
////////////////////////////////////////
void cleartitle(uint8_t froml)
{
     title[0] = 0;
     for (int i = froml;i<LINES-1;i++)  // clear lines
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
  int iindex = 0;
  while (characters[iindex])
  {
    if ((characters[iindex] >= 0xc2)&&(characters[iindex] <= 0xc3)) // only 0 to FF ascii char
    {
      //      SERIALX.println((characters[iindex]));
      characters[iindex+1] = ((characters[iindex]<<6)&0xFF) | (characters[iindex+1] & 0x3F);
      int sind = iindex+1;
      while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
      characters[sind-1] = 0;
    }
    iindex++;
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
//    len = strlen(from);
    ucg.setFont(ucg_font_6x13_mf);
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr(from," - ");
	if (from == nameset) {/*lline[0] = nameset;*/lline[1] = NULL;lline[2] = NULL;return;}
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
  static byte dvolume = true; // display volume screen
  char* ici;
Serial.println(line);
//return; 
   removeUtf8((byte*)line);
   
 //////  reset of the esp
   if ((ici=strstr(line,"VS Version")) != NULL) 
   {
      clearAll();
      setup2(true);
   }
   else
 ////// Meta title   ##CLI.META#:
   if ((ici=strstr(line,"META#: ")) != NULL)
   { 
     cleartitle(3); 
     strcpy(title,ici+7);    
	   separator(title); 
   } else 
 ////// ICY4 Description  ##CLI.ICY4#:
    if ((ici=strstr(line,"ICY4#: ")) != NULL)
    {
	    strcpy(genre,ici+7);
//	    if (lline[GENRE] == NULL)
	    {
	      lline[GENRE] = genre;
        markDraw(GENRE);  
	    }
    } else 
 ////// ICY0 station name   ##CLI.ICY0#:
   if ((ici=strstr(line,"ICY0#: ")) != NULL)
   {
//      clearAll();
	    if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
	    separator(station);
   } else
 ////// STOPPED  ##CLI.STOPPED#
   if ((ici=strstr(line,"STOPPED")) != NULL)
   {
       digitalWrite(PIN_PLAYING, LOW);
       state = false;
	     cleartitle(3);
       strcpy(title,"STOPPED");
       lline[TITLE1] = title;
       mline[TITLE1] = 1;
       
   }    
 /////// Station Ip      ' Station Ip:
   else  
   if ((ici=strstr(line,"Station Ip: ")) != NULL) 
   {
       eepromReadStr(EEaddrIp, oip);
       if ( strcmp(oip,ici+12) != 0)
         eepromWriteStr(EEaddrIp,ici+12 ); 
   } else
 //////Nameset    ##CLI.NAMESET#:
   if ((ici=strstr(line,"MESET#: ")) != NULL)  
   {
     clearAll();
     strcpy(nameset,ici+8);
	   ici = strstr(nameset," ");
     if (ici != NULL)
     {
	      strncpy(nameNum,nameset,ici-nameset+1);
	      nameNum[ici - nameset+1] = 0;
        strcpy (futurNum,nameNum);
     }
	   strcpy(nameset,nameset+strlen(nameNum));
     lline[STATIONNAME] = nameset;
     markDraw(STATIONNAME);          
   } else
 //////Playing    ##CLI.PLAYING#
   if ((ici=strstr(line,"YING#")) != NULL)  
   {
	   digitalWrite(PIN_PLAYING, HIGH);
     state = true;
     if (stateScreen == stime) Screen(smain0);
     if (strcmp(title,"STOPPED") == 0)
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
      markDraw(VOLUME); 
      timerScreen = 0;
   } else
 //////Volume offset    ##CLI.OVOLSET#:
   if ((ici=strstr(line,"OVOLSET#:")) != NULL)  
   {
      dvolume = false; // don't show volume on start station
   }else
 //////list station   #CLI.LISTINFO#:
   if (((ici=strstr(line,"LISTNUM#:")) != NULL)  || ((ici=strstr(line,"LISTNUM#:")) != NULL))
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
      struct tm dtl;
      char lstr[BUFLEN];
      if (*(ici+11) != '2') {vTaskDelay(100);itAskTime=true;return;} // invalid date. try again
      strcpy(lstr,ici+11);
      sscanf(lstr,"%d-%d-%dT%d:%d:%d",&dtl.tm_year,&dtl.tm_mon,&dtl.tm_mday,&dtl.tm_hour,&dtl.tm_min,&dtl.tm_sec);
      dtl.tm_year -= 1900;
      dtl.tm_mon-=1;
//      timein = dtl.tm_sec; //
      taskDISABLE_INTERRUPTS();
      timestamp = mktime(&dtl);
      taskENABLE_INTERRUPTS(); 
      syncTime = true;     
   }
}

////////////////////////////////////////
// receive the esp8266 stream
void serial()
{
    int temp;
    static unsigned  Rbindex = 0; // receive buffer Rbindex
//    SERIALX.println(F("Serial"));
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
// scroll each line if out of screen
void scroll()
{
int16_t len;
  if(stateScreen == smain)
  {
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
			case 0xF70812:	/*(" FORWARD");*/  stationPlus(); break;

			case 0xFF0044:
      case 0xF70842:
			case 0xF70815: /*(" LEFT");*/  minusVol();  break;

			case 0xFF0040:
			case 0xF7081E:		/*(" -OK-");*/ stationOk();			break;
			case 0xFF0043:
      case 0xF70841:
			case 0xF70814: /*(" RIGHT");*/ plusVol();     break; // volume +
			case 0xFF0015:
			case 0xF70813:	/*(" REVERSE");*/ stationMinus(); break;
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
  } else
  {
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

void drawTTitle(char* ttitle)
{
        ucg.setFont(ucg_font_helvB18_tf); 
        uint16_t xxx = (x/2)-(ucg.getStrWidth(ttitle)/2);
        ucg.setColor(0,60,60,60);  
        ucg.drawBox(0,0,x,41); 
        ucg.setColor(0,200,200,255);  
        ucg.drawString(xxx,10,0,ttitle);   
}
#ifdef IR
////////////////////
// draw the number entered from IR
void drawNumber()
{
  char ststr[] = {"Number"};
    switch (mTscreen){
      case 1:     
        drawTTitle(ststr);   
      // no break
      case 2:  
        uint16_t xxx ;
        xxx = (x/2)-(ucg.getStrWidth(irStr)/2); 
        ucg.setColor(0,0,0,0);  
        ucg.drawBox(0,40,x,yy);     
        ucg.setFont(ucg_font_inr38_tf); 
        ucg.setColor(0,20,255,20);  
        ucg.drawString(xxx,60,0, irStr);
        break;
      default:; 
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
    switch (mTscreen){
      case 1:  
        drawTTitle(ststr);        
      // no break
      case 2:   
        ucg.setColor(0,0,0,0);  
        ucg.drawBox(0,40,x,yy);     
        ucg.setFont(ucg_font_fur14_tf);
        ucg.setColor(0,20,255,20);
        ddot = strstr(sline,":");
        if (ddot != NULL)
        {
          *ddot=0; 
          char* ptl = sline+1;
          while ( *ptl == 0x20){*ptl = '0';ptl++;}
          ptl = ++ddot;
          while ( *ptl == 0x20){ddot++;ptl++;}
          if (strcmp(ddot,"not defined")==0) // don't start an undefined station
              playable = false; 
          else 
              playable = true;             
          strcpy (futurNum,sline+1); 
          ucg.drawString((x/2)-(ucg.getStrWidth(sline+1)/2),55,0, sline+1);
          len = (x/2)-(ucg.getStrWidth(ddot)/2);
          if (len <0) len = 0;
          ucg.drawString(len,90,0, ddot);
        }
        break;
      default:; 
    } 
    mTscreen = 0;    
}
////////////////////
// draw the volume screen
void drawVolume()
{
  char vlstr[] = {"Volume"};
    switch (mTscreen){
      case 1: 
        drawTTitle(vlstr) ;  
      // no break
      case 2:
        ucg.setColor(0,0,0,0);  
//        ucg.setFont(ucg_font_inr49_tf);
        ucg.setFont(ucg_font_inr38_mf);  
        uint16_t xxx;
        xxx = (x/2)-(ucg.getStrWidth(aVolume)/2);
        ucg.drawBox(0,40,x,yy);     
        ucg.setColor(0,20,255,20);  
        ucg.drawString(xxx,60,0,aVolume); 
        break;
      default:; 
    }
    mTscreen = 0; 
}

void drawSecond()
{
  static unsigned insec;
  if (insec != timein)
  {
  char strsec[3]; 
  dt=gmtime(&timestamp);
  uint16_t len;
  sprintf(strsec,":%02d",dt->tm_sec);
  ucg.setFont(ucg_font_6x13_mf);
  len = ucg.getStrWidth(strsec);
//  ucg.setColor(0,0,0,0);  
//  ucg.drawBox(x-len-8,yy-18,x,yy);  
  ucg.setColor(0,dt->tm_sec*5,255,dt->tm_sec*5);
  ucg.setColor(1,0,0,0); 
  ucg.setFontMode(UCG_FONT_MODE_SOLID); 
  ucg.drawString(x-len-8,yy-18,0,strsec); 
  ucg.setFontMode(UCG_FONT_MODE_TRANSPARENT);
  insec = timein; //to avoid redisplay
  }    
}
void drawTime()
{
  char strdate[23];
  char strtime[20];
  unsigned len;
    switch (mTscreen){
      case 2:
      case 1:
        dt=gmtime(&timestamp);
        sprintf(strdate,"%02d-%02d-%04d", (dt->tm_mon)+1, dt->tm_mday, dt->tm_year+1900);
        sprintf(strtime,"%02d:%02d", dt->tm_hour, dt->tm_min);
        drawTTitle(strdate);
        ucg.setColor(0,0,0,0); 
        ucg.setFont(ucg_font_inr38_mf); 
         
        if ( mTscreen ==1)
          ucg.drawBox(0,40,x,yy); 
        else
          ucg.drawBox(0,55,x,40+ucg.getFontDescent()+ucg.getFontAscent());     
        //ucg.setFont(ucg_font_helvB18_hf);  
        ucg.setColor(0,100,255,100);  
        ucg.drawString((x/2)-(ucg.getStrWidth(strtime)/2),55,0,strtime); 

        // draw ip
        ucg.setFont(ucg_font_6x13_mf);
        eepromReadStr(EEaddrIp, strtime);
        sprintf(strdate,"IP: %s",strtime);
        len = ucg.getStrWidth(strdate);
        ucg.setColor(0,0,0,0);  
        ucg.drawBox(0,yy-20,4+len,yy);  
        ucg.setColor(0,255,128,128);  
        ucg.drawString(4,yy-18,0,strdate);         break;
      default:;
    }
    mTscreen = 0;       
}

// Mark the lines to draw
void markDraw(int i)
{
  mline[i] = 1;
}
////////////////////////////////////////
// draw the full screen of lines
void drawLines()
{
   ucg.setFont(ucg_font_6x13_mf);
   for (int i=0;i<LINES;i++)
   {
     if (mline[i]) draw(i); 
   }
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
          case VOLUME:  ucg.setColor(200,200,255); break; 
          default:ucg.setColor(100,255,100);  
        }  
}
////////////////////
// draw one line
void draw(int i)
{
//  Serial.print("Draw ");Serial.print(i);Serial.print("  ");Serial.println(lline[i]);
     if ( mline[i]) mline[i] =0;
      if (i >=3) z = y/2 ; else z = 0;
      switch (i) {
        case STATIONNAME:
        ucg.setColor(255,255,255);  
        ucg.drawBox(0,0,x,((x == 84)?10:13)-ucg.getFontDescent());  
        ucg.setColor(0,0,0);  
        if (nameNum[0] ==0)  ucg.drawString(1,2,0,lline[i]+iline[i]);
        else 
        {
          ucg.drawString(1,2,0,nameNum);
          ucg.drawString(ucg.getStrWidth(nameNum)-2,2,0,lline[i]+iline[i]);
        }
        break;
        case VOLUME:
          ucg.setColor(0,0,200);   
          ucg.drawFrame(0,yy-10,x/2,8); 
          ucg.setColor(255,0,0); 
          ucg.drawBox(1,yy-9,((uint16_t)(x/2*volume)/255),6);                  
        break;
        case TIME:
          char strsec[13]; 
          dt=gmtime(&timestamp);
          uint16_t len,xpos,xxpos,yyy;
          sprintf(strsec,"%02d:%02d:%02d",dt->tm_hour, dt->tm_min,dt->tm_sec);
          ucg.setFont(ucg_font_6x13_mf);
          len = ucg.getStrWidth(strsec);
          xpos = (3*x/4)-(len/2);
          xxpos = xpos+len;
          yyy = yy -12;
//          ucg.setColor(0,0,0,0);  
//          ucg.drawBox(xpos,yyy,xxpos,yy);  
          ucg.setColor(250,250,255); 
          ucg.setColor(1,0,0,0); 
          ucg.setFontMode(UCG_FONT_MODE_SOLID);
          ucg.drawString(xpos,yyy,0,strsec); 
          ucg.setFontMode(UCG_FONT_MODE_TRANSPARENT);
        break;
        default:
          ucg.setColor(0,0,0); 
          ucg.drawBox(0,y*i+z,x,((x == 84)?10:13)-ucg.getFontDescent()); 
          setColor(i);
          ucg.drawString(0,y*i+z+1,0,lline[i]+iline[i]);                
      }      
}

////////////////////
void drawFrame()
{
    ucg.setFont(ucg_font_6x13_mf);
    ucg.setColor(0,255,255,0);  
    ucg.setColor(1,0,255,255);  
    ucg.drawGradientLine(0,(4*y) - (y/2)-5,x,0);
    ucg.setColor(0,255,255,255);  
    ucg.drawBox(0,0,x-1,(x == 84)?10:13);  

    for (int i=0;i<LINES;i++) { 
      draw(i);
    }
    mTscreen = 1;
}
////////////////////////////
// Change the current screen
////////////////////////////
void Screen(typeScreen st){
  if (stateScreen != st)
  {
    mTscreen = 1;
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
}

////////////////////
// Display a screen on the lcd
void drawScreen()
{
  switch (stateScreen)
  {
    case smain0:  // force the draw of the complete screen
      ucg.clearScreen();
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
      drawSecond();
      break;     
    case snumber:
#ifdef IR    
      drawNumber();
#endif      
      break;
    default: 
      drawLines();       
  }
}
////////////////////////////////////////
void loop(void) { // not used on freertos
}
