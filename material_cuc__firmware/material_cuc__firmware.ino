#include <Elegoo_TFTLCD.h> // libreria di comunicazione tra arduino e display
#include <Elegoo_GFX.h> // libreria per "effetti" grafici sul display
#include <TouchScreen.h> // libreria per il touch screen
#include <Wire.h> // libreria per la comunicazione i2c
#include <RTClib.h> // libreria per il real time clock che funziona in i2c
#include <SPI.h> //libreria per la comunicazione SPI (Serial Peripheral Interface) con la scheda SD
#include <SD.h> //libreria per la gestione di lettura/scrittura sulla SD 

#define LCD_RESET A4 // pin che usa lo schermo
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0

#define TS_MINX 927  //calibrazione touch/LCD
#define TS_MINY 899
#define TS_MAXX 128
#define TS_MAXY 130

#define YP A3  //pin usati dal touch
#define XM A2
#define YM 9
#define XP 8

#define BLACK 0x0000 //colori in formato RGB565 (converter: http://www.barth-dev.de/online/rgb565-color-picker/) in ogni caso sono 5 bit per rosso, 6 per verde e 5 per blu
#define WHITE 0xFFFF
#define LIGHT_GRAY 0xC618
#define INDIGO  0x3A96

#define CS 53 //pin dello slave SPI (in parole povere é dove Arduino trova l índirizzo alla SD a cui mandare i comandi, per questo é anche detto SS (Slave Select))

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET); //comunicazione pin-libreria dell'LCD
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 364); //comunicazione pin-libreria del Touch

RTC_DS1307 rtc; //definisce il modello di RTC in base all'oscillatore al quarzo
File settings; //friendly name per il file che andremo a leggere/scrivere dalla SD

  String secondi; //sottostringa contenente i secondi passati dal telefono
  String minuti; //sottostringa contenente i minuti passati dal telefono
  String ore;  //sottostringa contenente le ore passate dal telefono
  char c = '\n'; // ultimo carattere arrivato dalla seriale, é utile per ricomporre la stringa che contiene tutti i dati utili che Arduino riceve dall'App client
  String comando = "$#{GITHUB.COM/WAPEETY}#$";//= "HHMMSSHHMMHHMMHHMMCUSTOM MESSAGE NOT FOUND1<"; (custom message = 23) stringa che riceve TUTTI i dati in seriale
  bool connected = false; //variabile booleana che controlla la connessione tra Arduino e l'app client
  int ora_int = 0; // versione int della stringa ore
  int minuto_int = 0; // versione int della stringa minuti
  int secondo_int = 0; // versione int della stringa secondi

void setup(void){
  pinMode (CS, OUTPUT); // Serve per portare in LOW il pin per  l'SPI e ricevere correttamente l'indirizzo della SD
  Serial.begin(9600); //Avvia seriale di debug
  Serial2.begin(9600); //Avvia seriale con il Bluetooth
  Wire.begin(); //Avvia il modulo I2c
  if (!rtc.begin()){ //verifica eventuali errori di boot dell'RTC
    Serial.println("ERRORE, Ho riscontrato problemi nell'inizializzare una comunicazione con l'RTC");
  }
  if (!rtc.isrunning()){ //verifica se l'RTC ha in memoria l'orario in cso contrario Arduino prende dai suoi log l'orario di compilazione
    Serial.println("il chip non ha le informazioni di data e ora, lo sto settando secondo il compilatore, se credi sia sbagliato sincronizza l'app");
    rtc.adjust(DateTime(__DATE__, __TIME__)); // preso l'orario di compilazione ovviamente lo imposta sull'RTC
    }

  Serial.println(F("Leggo la micro SD"));
  if (!SD.begin(CS)) { //verifica se la connessione con la SD é andata a buon fine
    Serial.println(F("Qualcosa é andato storto con la SD!!"));
  }
  tft.reset(); //inizializza il Display
  
  uint16_t identifier = tft.readID(); //cerca il modello di display e usa i driver appropriati per quel modello
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

  tft.begin(identifier); //riinizializza il display, questa volta peró con i driver giusti
  tft.setRotation(1); //imposta la rotazione del display per regolarne la comparsa di testo, immagini e altri elementi grafici tramite un sistema di coordinate X Y

  tft.fillScreen(WHITE); //imposta lo sfondo
  tft.fillRect(0, 0, 330, 40, INDIGO); //crea la barra superiore
  tft.fillRect(10, 60, 300, 80, LIGHT_GRAY); //crea il quadrato contenente la sveglia
  tft.fillRect(10, 160, 300, 70, LIGHT_GRAY); // crea il quadrato contenente il messaggio custom
  tft.setTextColor(INDIGO, WHITE); // definisce il colore del testo e dello "sfondo" dietro di esso
  tft.setTextSize(2); //definisce la grandezza del testo
  tft.setCursor(10, 44); // imposta dove il seguente testo sará scritto
  tft.print("NEXT ALARM"); // scrive il testo
  tft.setCursor(10, 144);// cambia posizione
  tft.print("CUSTOM MESSAGE");// scrive il testo
  tft.setCursor(160, 13); // cambia posizione
  tft.setTextSize(2); // cambia grandezza
  tft.setTextColor(WHITE, INDIGO);// cambia colore testo e "sfondo" dello stesso
  tft.print("NOT CONNECTED");// scrive il testo
}
/*FINALMENTE IL SETUP É FINITO*/
void loop() {
  DateTime now = rtc.now(); //imposta l'orario dall'RTC
    if(Serial2.available()) { //verifica se il Bluetooth vuole passarci qualcosa
      comando=""; //ripulisce la stringa
      connected = true;// segnala che la connessione é avvenuta
      do {                         // IN QUESTE RIGHE
        if(Serial2.available()) {  // (118/120)
          c = Serial2.read();      // ARDUINO RICOMPONE CIÓ CHE HA RICEVUTO IN SERIALE
          if(c != '<')             // Verifica che il messaggio sia finito e nel caso OVVIAMENTE esclude il carattere da noi scelto per porre fine alla comunicazione
            comando += c;          // Aggiunge alla stringa il nuovo carattere appena arrivato
        }
    } while(c != '<');         // IN QUESTE RIGHE 
    for(int i = 0; i<2; i++){  // (124/137)
      ore += comando[i];       // ARDUINO SMONTA LA STRINGA CHE HA RICEVUTO PRENDENDOSI
      Serial.println(ore);     // ORE
    }                          //
    for(int i = 2; i<4; i++){  //
      minuti += comando[i];    //
      Serial.println(minuti);  // MINUTI
    }                          //
    for(int i = 4; i<6; i++){  //
      secondi += comando[i];   //
      Serial.println(secondi); //SECONDI
    }                          //
    Serial.println(comando);   //
    ora_int = (ore.toInt());         //
    minuto_int = (minuti.toInt());   // Ovviamente l'RTC ci vuole male e non accetta le stringhe seppur composte al 100% da numeri quindi convertiamo il tutto in numeri INT
    secondo_int = (secondi.toInt()); //
    rtc.adjust(DateTime(now.year(),now.month(),now.day(),ora_int, minuto_int, secondo_int));  //Adesso L'RTC finalmente accetta il nuovo orario
    Serial.println(ora_int);     //
    Serial.println(minuto_int);  // 'sta roba é solo per debuggare
    Serial.println(secondo_int); //
    settings = SD.open("settings.txt", FILE_WRITE); //Apre (o crea) il file per quelle impostazioni che dobbiamo salvare anche in caso arduino si spenga
    if (settings) { // se é stato aperto correttamente il file
      settings.println(comando); //scrive dentro
      settings.close(); //e lo chiude
    }
    
    }
  tft.setCursor(10,10); //Per tutto il resto del codice stampa roba sul display
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
  tft.setCursor(160, 13);
  tft.setTextSize(2);
  if (connected == true)
    tft.print("    CONNECTED");
  else if (connected == false){
    tft.print("NOT CONNECTED");
 }
  tft.setTextColor(WHITE, LIGHT_GRAY);
  tft.setTextSize(7);
  tft.setCursor(60, 75);
  tft.print("00:00");
  tft.setTextSize(2);
  tft.setCursor(20, 190);
  tft.print(comando);
}
