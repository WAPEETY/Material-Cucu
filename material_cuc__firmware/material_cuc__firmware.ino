#include <Elegoo_TFTLCD.h>
#include <Elegoo_GFX.h>
#include <TouchScreen.h>
#include <Wire.h>
#include <RTClib.h>

#define LCD_RESET 10 //a 4 pin che usa lo schermo
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1 //a1
#define LCD_RD A0 //a0

#define TS_MINX 927  //calibrazione touch/LCD
#define TS_MINY 899
#define TS_MAXX 128
#define TS_MAXY 130

#define YP A3  //pin usati dal touch
#define XM A2
#define YM 9
#define XP 8

#define BLACK 0x0000 //colori in formato RGB565 (converter: http://www.barth-dev.de/online/rgb565-color-picker/) in ogni caso sono 5 bit per il rosso, 6 per il verde e 5 per il blu
#define WHITE 0xFFFF
#define LIGHT_GRAY 0xC618
#define INDIGO  0x3A96

#define rxPin 0
#define txPin 1

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET); //comunicazione pin-libreria
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 364);

RTC_DS1307 rtc;

  unsigned long time;
  unsigned long secondi;
  unsigned long minuti;
  unsigned long ore;
  unsigned long giorni;
  char c = 'N';
  String comando = "SALVAMI SULL'SD, CESSO<";
  bool unix = false;

void setup(void){
  Serial.begin(9600);
  Wire.begin();
  if (!rtc.begin()){
    Serial.println("ERRORE, Ho riscontrato problemi nell'inizializzare una comunicazione con l'RTC");
  }
  if (!rtc.isrunning()){
    Serial.println("il chip non ha le informazioni di data e ora, lo sto settando secondo il compilatore, se credi sia sbagliato sincronizza l'app");
    rtc.adjust(DateTime(__DATE__, __TIME__));
    }
  tft.reset();
  
  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Trovati driver dell'LCD ILI9325"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Trovati driver dell'LCD ILI9328"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Trovati driver dell'LCD LGDP4535"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Trovati driver dell'LCD HX8347G"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Trovati driver dell'LCD ILI9341"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Trovati driver dell'LCD HX8357D"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else {
    Serial.print(F("Driver non trovati per: "));
    Serial.println(identifier, HEX);
    Serial.println(F("Se stai usando Elegoo 2.8\" TFT Arduino shield, la riga:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("dovrebbe apparire all'inizio della libreria (Elegoo_TFT.h)."));
    identifier=0x9341;
   
  }

  tft.begin(identifier);
  tft.setRotation(1);

  tft.fillScreen(WHITE);
  tft.fillRect(0, 0, 330, 40, INDIGO);
  tft.fillRect(10, 60, 300, 80, LIGHT_GRAY);
  tft.fillRect(10, 160, 300, 70, LIGHT_GRAY);
  tft.setTextColor(INDIGO, WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 44);
  tft.print("NEXT ALARM");
  tft.setCursor(10, 144);
  tft.print("CUSTOM MESSAGE");
  tft.setCursor(160, 13);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, INDIGO);
  tft.print("NOT CONNECTED");
}

void loop() {
  DateTime now = rtc.now();
    if(Serial.available()) {
      comando="";
      do {
        if(Serial.available()) {
          c = Serial.read();
          comando += c;
        }
    } while(c != '<');
    }
  tft.setCursor(10,10);
  if (unix == false){
  tft.setTextSize(3);
  tft.setTextColor(WHITE, INDIGO);
  if (now.hour() < 10){
    tft.print("0");
  }
  tft.print(now.hour(), DEC);
  tft.print(":");
  if (now.minute() < 10){
    tft.print("0");
  }
  tft.print(now.minute(), DEC);
  tft.print(":");
  if (now.second() < 10){
    tft.print("0");
  }
  tft.print(now.second(), DEC);
  }
  else{
  tft.setTextSize(2);
  tft.print(now.unixtime(), DEC);
  }
  tft.setCursor(160, 13);
  tft.setTextSize(2);
  if (comando == "SI<")
    tft.print("    CONNECTED");
  else if (comando == "NO<"){
    tft.print("NOT CONNECTED");
 }
  tft.setTextColor(WHITE, LIGHT_GRAY);
  tft.setTextSize(7);
  tft.setCursor(60, 75);
  tft.print("00:00");
  tft.setTextSize(2);
  tft.setCursor(20, 190);
  tft.print(comando);
  
//  TSPoint p = ts.getPoint();
//  if (p.z > ts.pressureThreshhold) {
//    p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
//    p.y = map(p.y, TS_MAXY, TS_MINY, 0, 480);
//    if(p.x > 10 && p.x < 150 && p.y > 10 && p.y < 40){
//      if (unix == false)
//        unix = true;
//      else
//        unix = false;
//    }
//  }
}
