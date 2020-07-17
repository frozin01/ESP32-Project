#include <DHTesp.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"

//Bluetooth
BluetoothSerial SerialBT;

//API ThingSpeak
WiFiClient client;
String apiKey = "RPAQ5YHE9R4TWSNP";
const char* server = "api.thingspeak.com";

//ssid dan pass
const char *ssid     = "rozin";
const char *password = "13217040";

//location for indonesia GMT+7
const long utcOffsetInSeconds = 25200;

//array for day and month
char daysOfTheWeek[7][12] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
char months[12][12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

//type of LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);

//Define NTP client to get time and date
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//DHT11
DHTesp dht;

//Push Button
const int buttonPin = 15; 
int buttonState = 0;
int temp= 1; 

//Relay
int relay=26;
char kondisi;

void setup(){
  Serial.begin(115200); //inisiasi serial
  pinMode(buttonPin, INPUT); //inisiasi button
  pinMode(relay, OUTPUT); //inisiasi relay
  SerialBT.begin("ESP32Rozin"); //Bluetooth device name

  WiFi.begin(ssid, password); //inisiasi wifi
  
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  dht.setup(27, DHTesp::DHT11);
  
  
  //connect to wifi
  while ( WiFi.status() != WL_CONNECTED ) {
    lcd.setCursor(5,0);
    lcd.print ( "TRYING" );
    lcd.setCursor(0,1);
    lcd.print ( "CONNECT TO WIFI!" );
    delay (1000);
  }

  timeClient.begin(); //inisiasi NTP client
  timeClient.setTimeOffset(25200); //set sesuai location indonesia

  
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("R");delay(200); lcd.print("O");delay(200);
  lcd.print("Z");delay(200); lcd.print("I");delay(200);
  lcd.print("N");delay(200);

  lcd.setCursor(0,1);
  lcd.print("D");delay(200); lcd.print("I");delay(200);
  lcd.print("G");delay(200); lcd.print("I");delay(200);
  lcd.print("T");delay(200); lcd.print("A");delay(200);
  lcd.print("L");delay(200); lcd.print(" ");delay(200);
  lcd.print("C");delay(200); lcd.print("L");delay(200);
  lcd.print("O");delay(200); lcd.print("C");delay(200);
  lcd.print("K");delay(200);
  lcd.clear();
}

void loop() {

  int h = dht.getHumidity();
  int t = dht.getTemperature();

  //kondisi button
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    //Serial.println("Ditekan");
    temp=!temp;
  } else {
    //Serial.println("Tidak ditekan");
  }
  
  if ( WiFi.status() == WL_CONNECTED ) {
    
    //komunikasi bluetooth relay
    kondisi=(char)SerialBT.read();
    Serial.println(kondisi);
    if (SerialBT.available()) {
      if(kondisi=='a'){
        digitalWrite(relay,LOW);
      }else if(kondisi=='b'){
        digitalWrite(relay,HIGH);
      }
    }
    
    if ((h<100)&& (t<100)){
      if(client.connect(server, 80)){
        String postStr= apiKey;
        postStr += "&field1=";
        postStr += String(t);
        postStr += "&field2=";
        postStr += String(h);
        postStr += "\r\n\r\n";
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr);
      }
    }
    
    //Saat ditekan dan temp HIGH
    if(temp==HIGH){
      //digitalWrite(relay,LOW);
      //Serial.println("temp 1");
      if ((h<100)&& (t<100)){
        //Serial.println(t);
        //Serial.println(h);
        lcd.setCursor(8,1);
        lcd.print(" ");
        lcd.print(t);
        lcd.print("C/");
        lcd.setCursor(13,1);
        lcd.print(h);
        lcd.print("%");
      }
      timeClient.update();
  
      unsigned long epochTime = timeClient.getEpochTime(); //get epochtime
  
      //Serial.print(daysOfTheWeek[timeClient.getDay()]);
      // Serial.print(", ");
      // Serial.println(timeClient.getFormattedTime());
      
      lcd.setCursor(0, 0);
      lcd.print(daysOfTheWeek[timeClient.getDay()]); //show day
      lcd.print(", ");
      struct tm *ptm = gmtime ((time_t *)&epochTime); 
      
      int monthDay = ptm->tm_mday;
      int currentMonth = ptm->tm_mon+1;
      String currentMonthName = months[currentMonth-1];
      int currentYear = ptm->tm_year+1900;
    
      String currentDate = String(monthDay)+ " " + String(currentMonthName) + " " + String(currentYear) + " " ;
      //Serial.println(currentDate);
      lcd.print(currentDate); //show date month year
      
      lcd.setCursor(0, 1);      
      lcd.print(timeClient.getFormattedTime()); //show time

   //Saat ditekan temp jadi LOW
   }else{
      //digitalWrite(relay,HIGH);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("INI KONDISI");
      lcd.setCursor(0,1);
      lcd.print("BEDA");
      //Serial.println(relay);
      //Serial.println("temp 0");
   }
   client.stop();

 //Kondisi saat connection hilang
 }else{
    lcd.setCursor(0, 0); 
    lcd.print("CONNECTION LOST!!");
    lcd.setCursor(0, 1); 
    lcd.print("CONNECTION LOST!!");
  }
  
  delay(1000);
}
