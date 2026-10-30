#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h> // library for ST7789 disp. 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h" // bme680 sensor lib. 
#include <WiFi.h>
#include <time.h>



// pin defs::(ESP32)
    //tft pins
#define tft_SCLK D9
#define tft_MOSI D10
#define tft_DC D4
#define tft_CS D5
#define tft_RST -1 //WIRED TO 3.3V DUE TO PIN SHORTAGE AS BME680 WAS ADDED...


    //BME680 pins
#define i2c_SDA D6
#define i2c_SCL D8

    //SWITCH pins
#define SW1 D3
#define SW2 D2
#define SW3 D1
#define SW4 D0

    // buzzer pins
#define buzzer_PIN D7


// network+time settt..(noteeeee: you gotta put the details of your wifi here in home or office....)
const char* ssid ="insert_ssid_of_wifi";
const char* password ="insert_password_of_wifi";
const char* ntpServer  = "pool.ntp.org";
const long  gmtOffset_sec = 20700; // GMT+5:45 for  Nepal my country(put your country's gmt respectively here)
const int   daylightOffset_sec = 0;


// ui states that display diff. screens acc to the condition...
enum State { CLOCK_VIEW, MENU_SELECT_ALARM, MENU_SET_HR, MENU_SET_MIN, MENU_TOGGLE };
State currentState = CLOCK_VIEW;

// alarm logic.....
struct Ghanti
{
    int hour;
    int minute;
    bool active;
};

// added 5 alarm that can be set in the clock that can be customied to be turned or or not
Ghanti alarms[5] = {
  {6, 0, false}, {7, 0, false}, {8, 0, false}, {9, 0, false}, {10, 0, false}
};
int currindex=0;

// ghanti state mgmt. 
bool ring=false;
bool snooze=false;
int snoozetar_hr=-1;
int snoozetar_min=-1;
int curr_ring_min=-1;
int curr_min=-1;

// hardware objs. 
Adafruit_BME680 bme; //bme680 i2c



// fix tft row start....
class st7789 :public Adafruit_ST7789{
public:
    st7789(int8_t cs, int8_t dc,int8_t mosi, int8_t sclk,int8_t rst) :Adafruit_ST7789(cs,dc,mosi,sclk,rst){}
    void setoff(unit8_t col,unit8_t row){
        _colstart=_colstart2=col;
        _rowstart=_rowstart2=row;
    }
};

st7789 tft(tft_CS,tft_DC,tft_MOSI,tft_SCLK,tft_RST);


void setup(){
    Serial.begin(115200);

    // initialization:::::..
    // buzzer:
    pinMode(buzzer_PIN,OUTPUT);
    digitalWrite(buzzer_PIN,LOW);

    // mx switches.
    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(SW3, INPUT_PULLUP);
    pinMode(SW4, INPUT_PULLUP);

    // TFT display
    // bl(backlight) is connected to 3.3v so it turn on automatically
    tft.init(76, 284);
    tft.setoff(82,18);
    tft.invertDisplay(false);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(3);
    tft.print("BOOTING......");

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 50);
    tft.print("Wi-Fi Syncing...");

    //  wifi- setup...
     WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);



    // INIT. bme i2c
    Wire.begin(i2c_SDA,i2c_SCL);
    bme.begin(0x76);     // 0x76 i2c for bme680 
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setGasHeater(320, 150);

      tft.fillScreen(ST77XX_BLACK);
    

}

void checkAlarms(struct tm *timeinfo) {
  if (ring) return;
  
  // chk standard ghantis
  for (int i = 0; i < 5; i++) {
    if (alarms[i].active && alarms[i].hour == timeinfo->tm_hour && alarms[i].minute == timeinfo->tm_min) {
      if (curr_ring_min != timeinfo->tm_min) {
        ring = true;
        snooze = false;
        curr_ring_min = timeinfo->tm_min;
        tft.fillScreen(ST77XX_RED);
        return;
      }
    }
  }

  // chk snooze target
  if (snooze && timeinfo->tm_hour == snoozetar_hr && timeinfo->tm_min == snoozetar_min) {
    if (curr_ring_min != timeinfo->tm_min) {
      ring = true;
      snooze = false;
      curr_ring_min = timeinfo->tm_min;
      tft.fillScreen(ST77XX_RED);
    }
  }
}

void drawDashboard(struct tm *timeinfo) {
  tft.fillRect(0, 0, 240, 135, ST77XX_BLACK);

  // bigg Clock
  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(5);
  tft.setCursor(10, 10);
  tft.print(timeStr);

  // Active ghantis & Snooze Stat..
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 60);
  
  if (snooze) {
    tft.setTextColor(ST77XX_ORANGE);
    tft.print("Snoozed until ");
    tft.print(snoozetar_hr); tft.print(":");
    if(snoozetar_min< 10) tft.print("0");
    tft.print(snoozetar_min);
  } else {
    tft.print("Alarms ON: ");
    bool active_all = false;
    for (int i = 0; i < 5; i++) {
      if (alarms[i].active) {
        tft.print(i + 1); tft.print(" ");
        active_all = true;
      }
    }
    if (!active_all) tft.print("None");
  }

  // env. datas ....bme
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, 85);
  tft.print(bme.temperature, 1); tft.print("˚C | ");
  tft.print(bme.humidity, 1); tft.print("%");

  tft.setCursor(10, 110);
  if (bme.gas_resistance > 50000.0) { tft.setTextColor(ST77XX_GREEN); tft.print("Air STAT: Fresh"); }
  else if (bme.gas_resistance > 20000.0) { tft.setTextColor(ST77XX_ORANGE); tft.print("Air STAT: Fair"); }
  else { tft.setTextColor(ST77XX_RED); tft.print("Air STAT: POOR"); }
}

void drawMenu() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);

  if (currentState == MENU_SELECT_ALARM) {
    tft.print("Select Alarm:");
    tft.setCursor(10, 50);
    tft.setTextColor(ST77XX_CYAN);
    tft.print("Alarm ");
    tft.print(currindex + 1);
  } 
  else if (currentState == MENU_SET_HR) {
    tft.print("Set Hour:");
    tft.setCursor(10, 50);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print(alarms[currindex].hour);
  }
  else if (currentState == MENU_SET_MIN) {
    tft.print("Set Minute:");
    tft.setCursor(10, 50);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print(alarms[currindex].minute);
  }
  else if (currentState == MENU_TOGGLE) {
    tft.print("Alarm is:");
    tft.setCursor(10, 50);
    if (alarms[currindex].active) {
      tft.setTextColor(ST77XX_GREEN); tft.print("ON");
    } else {
      tft.setTextColor(ST77XX_RED); tft.print("OFF");
    }
  }
  
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 115);
  tft.print("UP  DN  SEL  BACK");
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  // read all btns 
  bool bt1 = !digitalRead(SW1);
  bool bt2 = !digitalRead(SW2);
  bool bt3 = !digitalRead(SW3);
  bool bt4 = !digitalRead(SW4);

  // alarm state
  if (ring) {
    // pulse buzz
    if ((millis() % 1000) < 500) digitalWrite(buzzer_PIN, HIGH);
    else digitalWrite(buzzer_PIN, LOW);

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(4);
    tft.setCursor(20, 50);
    tft.print("WAKE UP!");
    tft.setTextSize(1);
    tft.setCursor(20, 100);
    tft.print("SW1+SW4: Stop | SW2+SW3: Snooze");

    // DISMISS: when sw1 and sw4 are pressed together...
    if (bt1 && bt4) {
      ring = false;
      snooze = false;
      digitalWrite(buzzer_PIN, LOW);
      curr_min = -1; // force dash. reinitiate
      delay(500); 
    }
    // SNOOZE: when sw2 and sw3 are pressed together...
    else if (bt2 && bt3) {
      isRinging = false;
      isSnoozed = true;
      digitalWrite(buzzer_PIN, LOW);
      
      // Add 5 minutes,+ wrap around hour if needed
      snoozetar_min = timeinfo.tm_min + 5;
      snoozetar_hr = timeinfo.tm_hour;
      if (snoozetar_min >= 60) {
        snoozetar_min -= 60;
        snoozetar_hr = (snoozetar_hr + 1) % 24;
      }
      currentMinute = -1;
      delay(500);
    }
    return;
  }

  // mormal state and menus
  if (currentState == CLOCK_VIEW) {
    checkAlarms(&timeinfo);

    // kill switch press btn 1 and 4 here.....(i.e turn all alarm off. )
    if (bt1 && bt4) {
      for(int i=0; i<5; i++) alarms[i].active = false;
      snooze = false;
      tft.fillRect(0, 60, 240, 20, ST77XX_BLACK);
      tft.setCursor(10, 60); tft.setTextSize(1); tft.setTextColor(ST77XX_RED);
      tft.print("ALL ALARMS DISABLED");
      delay(1000);
      currentMinute = -1;
    }
    else if (bt3) 
    { // enter Menu
      currentState = MENU_SELECT_ALARM;
      drawMenu();
      delay(300);
    }
    else if (timeinfo.tm_min != curr_min) 
    { // Redraw once per min
      if (bme.performReading()) {
        drawDashboard(&timeinfo);
        curr_min = timeinfo.tm_min;
      }
    }
  } 
  else {
    // menu navs. 
        // globl back btn ultimate one..
    if (bt4) 
    {
      currentState = CLOCK_VIEW;
      curr_min = -1;
      delay(300);
      return;
    }

    if (currentState == MENU_SELECT_ALARM) {
      if (bt1) 
      { 
        currindex = (currindex + 1) % 5; 
        drawMenu(); 
        delay(200); 
    }
      if (bt2) 
      { 
        currindex = (currindex - 1 < 0) ? 4 : currindex - 1; 
        drawMenu();
         delay(200);
     }
      if (bt3) {
         currentState = MENU_SET_HR; 
         drawMenu();
          delay(300);
     }


    }
    else if (currentState == MENU_SET_HOUR) {
      if (bt1) { 
        alarms[currindex].hour = (alarms[currindex].hour + 1) % 24;
         drawMenu();
          delay(150);
     }
      if (bt2) {
         alarms[currindex].hour = (alarms[currindex].hour - 1 < 0) ? 23 : alarms[currindex].hour - 1;
         drawMenu();
         delay(150);
        }
      if (bt3) { 
         currentState = MENU_SET_MIN;
         drawMenu();
         delay(300);
       }
    }
    else if (currentState == MENU_SET_MIN) {
        if (bt1) { 
            alarms[currindex].minute = (alarms[currindex].minute + 1) % 60; 
            drawMenu(); 
            delay(100); 
        }
        if (bt2) { 
            alarms[currindex].minute = (alarms[currindex].minute - 1 < 0) ? 59 : alarms[currindex].minute - 1; 
            drawMenu(); 
            delay(100); 
        }
        if (bt3) { 
            currentState = MENU_TOGGLE; 
            drawMenu(); 
            delay(300); 
        }
    }
    else if (currentState == MENU_TOGGLE) {
        if (bt1 || bt2) { 
            alarms[currindex].active = !alarms[currindex].active; 
            drawMenu(); 
            delay(200); 
        }
        if (bt3) { 
            currentState = CLOCK_VIEW; 
            currentMinute = -1; 
            delay(300); 
        }
    }
  }
}