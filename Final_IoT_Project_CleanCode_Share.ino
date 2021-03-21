// This is Independent Study project (Grade 11)
// Author : Thunyaluk Sasiwarinkul
// Instructor : Waraluk Siracarm
// From Manchasuksa School: Manchakhiri District, Khonkaen Thailand.
// Version 1.3.0 (21/3/2021)


#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
#define DHTPIN 2 
#include <ESP8266WiFi.h>
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" //Enter Your Line TOKEN
#include <Time.h>

#define SAMPLING 400
#define ADC_PIN A0
#define AMPLITUDE 400
#define REAL_VAC  255

char auth[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; //Enter Your BLYNK TOKEN
char ssid[] = "xxxxxxxxxxxxxxx"; //Enter Your Wifi SSID
char pass[] = "xxxxxxxxxxx"; //Enter Your Wifi Password
const String month_name[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const String day_name[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

int timezone = 7 * 3600; 
int dst = 0; 

int adc_max,adc_min;
int adc_vpp;

bool Connected2Blynk = false;

void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.printf("STARTING........");
  Blynk.begin(auth, ssid, pass);
  LINE.setToken(LINE_TOKEN);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");}

  Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
  while (Blynk.connect() == false) {
    // Wait until connected
  }

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov"); 
  Serial.println("\nLoading time");
  while (!time(nullptr)) {
    Serial.print("*");
    delay(1000);
  }
}

int flagV = 0;

void CheckConnection(){
  Connected2Blynk = Blynk.connected();
  if(!Connected2Blynk){
    Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk  
  }
  else{
       
  }
}

void read_VAC()
{
  int cnt;
  adc_max = 0;
  adc_min = 1024;
  
  for(cnt=0;cnt<SAMPLING;cnt++)
  {
    int adc = analogRead(ADC_PIN);
    if(adc > adc_max)
    {
      adc_max = adc;
    }
    if(adc < adc_min)
    {
      adc_min = adc;
    }
  }
  adc_vpp = adc_max-adc_min;   
}


void loop() {
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
  CheckConnection();
  if(Connected2Blynk){
    Blynk.run();
  }
  time_t now = time(nullptr);
   struct tm* p_tm = localtime(&now);
   int Hour = p_tm->tm_hour;
   int Minute = p_tm->tm_min;
   int Sec = p_tm->tm_sec;
  float humidity = dht.readHumidity(); 
  float temperature = dht.readTemperature();
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  String Time_Hour ;
  String Time_Minute ;
  String Time_Sec ;
  if (Hour<10){
    Time_Hour = "0"+Hour ;}
  else {
    Time_Hour = Hour;}
  if (Minute<10){
    Time_Minute = "0"+Minute ;}
  else {
    Time_Minute = Minute;}
  String TimeNOW = "Time : "+Time_Hour+"."+Time_Minute;
  read_VAC();
  float V = map(adc_vpp,0,AMPLITUDE,0,REAL_VAC*100.00)/100.00;
  String dataV;
  Blynk.virtualWrite(V3, V);
  String DateNOW = "Date : "+String(day_name[(p_tm->tm_wday)])+" "+String(p_tm->tm_mday)+" "+String(month_name[(p_tm->tm_mon)])+" "+String(p_tm->tm_year + 1900) ;
  if ( (Hour == 5 && Minute == 0 && Sec == 0) || (Hour == 6 && Minute == 0 && Sec == 0) ){
    LINE.notify("\nTemperature : "+String(temperature)+" °C\n"
    "Humidity : "+String(humidity)+" %\n"+
    "Date : "+String(day_name[(p_tm->tm_wday)])+" "+String(p_tm->tm_mday)+" "+String(month_name[(p_tm->tm_mon)])+" "+String(p_tm->tm_year + 1900)
    +"\nTime : 0"+String(Hour)+":"+"00"); } ;
  if( (V>=185) && (flagV == 1)){
    LINE.setToken(LINE_TOKEN);
    LINE.notify("\nPower Status : Normal\n"+String(DateNOW)+"\n"+String(TimeNOW));
    flagV = 0;      
  }
  if( (V<=20) && (flagV == 0)){
    LINE.setToken(LINE_TOKEN);
    LINE.notify("\nPower Status : Voltage Drop\n"+String(DateNOW)+"\n"+String(TimeNOW));
    flagV = 1;      
  }
  }
