
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>



//Pin for ESP32
  #define TFT_CS         13  //case select connect to pin 5
  #define TFT_RST        15 //reset connect to pin 15
  #define TFT_DC         14 //AO connect to pin 32  (not sure that this is really used)  try pin 25
  #define TFT_MOSI       12 //Data = SDA connect to pin 23
  #define TFT_SCLK       2 //Clock = SCK connect to pin 18


// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//Weather 

String URL = "http://api.openweathermap.org/data/2.5/weather?";
String ApiKey = "283e9fb3e957e04bcaeb3aefb6268d4e";

String lat = "30.768747840124973";
String lon = "76.57524314177009";





#define RXPin 16
#define TXPin 17

#define GPS_BAUD 9600

// Create an instance of the HardwareSerial class for Serial 2
SoftwareSerial ss(RXPin, TXPin);


// The TinyGPS++ object
TinyGPSPlus gps;

 

//wifi credentials
const char* ssid = "SmartMirror";
const char* password = "12345678";


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;




void setup(void) {
  
  Serial.begin(115200);
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  Serial.println("Initialized");

  // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
  ss.begin(GPS_BAUD);
  Serial.println("Serial 2 started at 9600 baud rate");



tft.fillScreen(ST77XX_BLACK);
tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
tft.setRotation(1);
int w = tft.width();
int h = tft.height();
Serial.print("width =");
Serial.println(w);
Serial.print("height =");
Serial.println(h);
//expecting 128 x 160
  WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

}//end of void setup

void centerHorizontally( const String &textValue, int cursorXPos, int cursorYPos ) {
    int16_t x1, y1;
    uint16_t textWidth, textHeight;

    tft.getTextBounds( textValue, 0, 0, &x1, &y1, &textWidth, &textHeight );
    tft.setCursor( ( 160 - textWidth ) / 2, cursorYPos );

    tft.print( textValue );
}

String toString(IPAddress& ip) { // IP v4 only
  String ips;
  ips.reserve(16);
  ips = ip[0];  ips += ':';
  ips += ip[1]; ips += ':';
  ips += ip[2]; ips += ':';
  ips += ip[3];
  return ips;
}


void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();

int w = tft.width();
int h = tft.height();  

if(gps.location.isValid()){
  lat=gps.location.lat();
  lon=gps.location.lng();
}


  if (WiFi.status() == WL_CONNECTED) {

    tft.fillCircle(w-4, 2, 2, ST77XX_GREEN);

    HTTPClient http;

    //Set HTTP Request Final URL with Location and API key information
    http.begin(URL + "lat=" + lat + "&lon=" + lon + "&units=metric&appid=" + ApiKey);

    // start connection and send HTTP Request
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {

      //Read Data as a JSON string
      String JSON_Data = http.getString();
      Serial.println(JSON_Data);

      //Retrieve some information about the weather from the JSON format
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, JSON_Data);
      JsonObject obj = doc.as<JsonObject>();

      //Display the Current Weather Info
      const char* description = obj["weather"][0]["description"].as<const char*>();
      const float temp = obj["main"]["temp"].as<float>();
      const float humidity = obj["main"]["humidity"].as<float>();
      const String city=obj["name"].as<const char*>();
      tft.setTextSize(1.0);
      String t="Temp: "+String(temp)+"C";
      String h="Hum: "+String(humidity)+"%";
      tft.setCursor(5, 100);
      tft.print(t);
      tft.setCursor(85, 100);
      tft.print(h);
      centerHorizontally(city,0,110);
      
    } 
    else {
      tft.fillCircle(w-4, 2, 2, ST77XX_BLUE);
      tft.setTextSize(1.0);
      centerHorizontally("No Internet",0,100);
      Serial.println("Error!");

      
    }

    http.end();
  }


configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  int hour=timeinfo.tm_hour;
  int min=timeinfo.tm_min;
  int hour12;
  if(hour==0){
    hour12=12;
  }
  else if(hour>=13){
    hour12=hour-12;
  }
  else{
    hour12=hour;
  }
  String Time;
  if(hour12<10 && min>9){

  Time="0"+String(hour12)+":"+String(min);
  }
  else if(hour12>9 && min<10){
  Time=String(hour12)+":"+"0"+String(min);
  }
  else if(hour12<10 && min<10){
    Time="0"+String(hour12)+":"+"0"+String(min);
  }
  else{
    Time=String(hour12)+":"+String(min);
  }
  tft.setTextSize(3.0);
  if(hour>=12){
  centerHorizontally(Time+" PM",0,53);
  }
  else{
  centerHorizontally(Time+" AM",0,53);
  }


 if(hour>=6 && hour<12){
  tft.setTextSize(2.0);
  if(hour==6 && min==0){
  tft.fillRect(0,10,160,30,ST77XX_BLACK);}
  centerHorizontally("Good Morning",0,10);
 }
 else if(hour>=12 && hour<17){
  tft.setTextSize(1.9);
  if(hour==12 && min==0){
  tft.fillRect(0,10,160,30,ST77XX_BLACK);}
  centerHorizontally("Good Afternoon",0,10);
 }
 else if(hour>=17 && hour<21){
  tft.setTextSize(2.0);
  if(hour==17 && min==0){
  tft.fillRect(0,10,160,30,ST77XX_BLACK);}
  centerHorizontally("Good Evening",0,10);
 }
 else{
  tft.setTextSize(2.0);
  if(hour==21 && min==0){
  tft.fillRect(0,10,160,30,ST77XX_BLACK);}
  centerHorizontally("Good Night",0,10);
 }

}



void loop() {

  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

}







  


