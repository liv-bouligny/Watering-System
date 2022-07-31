/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Jason.000/Documents/IoT/Watering-System/Plant_Watering_System/src/Plant_Watering_System.ino"
/*
 * Project Plant_Watering_System
 * Description: Water a plant based on input from a moisture sensor 
 *  in addition to integrating Dust, Air Quality, and BME sensors.
 * Author: Liv Bouligny
 * Date: 29 July 2022
 */

#include "credentials.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT.h" 
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"
#include "Grove_Air_quality_Sensor.h"
void setup();
void loop();
void MQTT_connect();
void envCheck ();
void moistCheck ();
void displayEnvironment();
#line 16 "c:/Users/Jason.000/Documents/IoT/Watering-System/Plant_Watering_System/src/Plant_Watering_System.ino"
const int OLEDADD = 0x3C;
const int BMEADD = 0x76;
const int OLED_RESET = 20;
const int SOILPIN = 14;
const int DUSTPIN = 2;
const int AQPIN = 19;
const int PUMPPIN = 10;
const char degree = 0xF8;
unsigned long currentTime, duration;
unsigned long lowpulseoccupancy = 0;
unsigned long sampleTime_ms = 300000;
float ratio = 0;
float concentration = 0;
int last, moist, moistP, quality, currentM, currentS, lastM;
float tempC, tempF, pressPA, pressHG, humidRH;
bool status;
String DateTime , TimeOnly;
TCPClient TheClient;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 
Adafruit_MQTT_Subscribe mqttButton = Adafruit_MQTT_Subscribe (& mqtt , AIO_USERNAME "/feeds/WSButton");
Adafruit_MQTT_Publish mqttMoist = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WSMoisture");
Adafruit_MQTT_Publish mqttTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WSTemp");
Adafruit_MQTT_Publish mqttPress = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WSPressure");
Adafruit_MQTT_Publish mqttDust = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WSDust");
Adafruit_MQTT_Publish mqttAQ = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WSQuality");
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;
AirQualitySensor sensor(A0);
SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, OLEDADD); 
  status = bme.begin(BMEADD);
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Time.zone ( -6);
  Particle.syncTime ();    
  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();
  display.display();  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if (status == false)  {    
    display.printf("BME280 at address 0x%02X failed to initialize\n", BMEADD);
    display.display();
  }
  pinMode(SOILPIN, INPUT);
  pinMode(DUSTPIN, INPUT);
  pinMode(AQPIN, INPUT);
  pinMode(PUMPPIN, OUTPUT);
  envCheck();
  displayEnvironment();
  if (sensor.init()) {
    Serial.printf("Sensor ready.\n");
  }
  else {
    Serial.printf("Sensor ERROR!\n");
  }
  mqtt.subscribe (&mqttButton);
}

void loop() {  
  MQTT_connect();

  if ((millis()-last)>120000) {
    Serial.printf("Pinging MQTT \n");
    if(! mqtt.ping()) {
      Serial.printf("Disconnecting \n");
      mqtt.disconnect();
    }
    last = millis();
  }
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(500))) {
    if (subscription == &mqttButton) {
                                                    //Button input from dashboard (insert manual pump operation)
    }
  }
  duration = pulseIn(DUSTPIN, LOW);                 //Dust sensor duration of particle interference
  lowpulseoccupancy = lowpulseoccupancy+duration;   //Dust sensor LPO Calculation  
  currentTime = millis();
  currentM = Time.minute();
  currentS = Time.second();
  if ((currentTime - last) > sampleTime_ms) {       //update sensor values every 5 mins/publish to adafruit
    envCheck();    
    last = millis();
  }  
  if (currentM == 0) {                              //Every 30 min (on the hour and half hour)   
    if (currentS <= 1) {                            //check moisture and water plant if needed
      moistCheck();                                 //and update OLED Display every 15 min
      displayEnvironment();
    }
  }
  if (currentM == 30) {  
    if (currentS <= 1) {      
      moistCheck();
      displayEnvironment();
    }
  }
  if (currentM == 15) {
    if (currentS <= 1)  {      
      displayEnvironment();
    }
  }  
  if (currentM == 45) {
    if (currentS <= 1)  {      
      displayEnvironment();
    }
  }  
}

void MQTT_connect() {
  int8_t ret;
 
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.printf("%s\n",(char *)mqtt.connectErrorString(ret));
    Serial.printf("Retrying MQTT connection in 5 seconds..\n");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.printf("MQTT Connected!\n");
}

void envCheck ()  {      
  moist = analogRead(SOILPIN);                    //Moisture Sensor reading
  moistP = map(moist,2700,1150,0,100);                  
  tempC = bme.readTemperature();                  //BME Temp reading
  tempF = (map(tempC,0.0,100.0,32.0,212.0));      //BME Temp conversion C to F      
  pressPA = bme.readPressure();                   //BME Pressure reading
  pressHG = (map(pressPA,0.0,3386.39,0.0,1.0));   //BME Pressure conversion PA to inHG      
  humidRH = bme.readHumidity();                   //BME Humidity reading
  ratio = lowpulseoccupancy/(sampleTime_ms*10.0); // Dust Sensor Integer percentage 0=>100
  concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // Dust particle concentration using spec sheet curve
  quality = sensor.slope();
  for (i=0;i<5;i++) {
    digitalWrite(D7,HIGH);
    delay(100);
    digitalWrite(D7,LOW);
    delay(100);
  }
}

void moistCheck () {                              //check moisture and activate pump for 250ms if needed
  display.clearDisplay();  
  display.setCursor(0,0);
  display.display();   
  display.printf("Reading Moisture...\nOptimal Range: 58%% to 71%%\n");
  display.display();
  moist = analogRead(SOILPIN);                    //Moisture Sensor reading
  moistP = map(moist,2700,1150,0,100);
  display.printf("Moisture Level: %i %i%%\n",moist, moistP);
  display.display();
  if (moist > 1800) {
    display.clearDisplay();      
    display.setCursor(0,0);
    display.printf("Engaging Water Pump...\n");
    display.display();
    delay (800);
    digitalWrite (PUMPPIN, HIGH);
    delay(250);
    digitalWrite (PUMPPIN,LOW); 
    display.clearDisplay();      
    display.setCursor(0,0);
    display.printf("Watering complete!\n");
    display.display();       
  } 
  else {
    display.clearDisplay();      
    display.setCursor(0,0);
    display.printf("Moisture within parameters\nNo watering needed.\n");
    display.display();
    delay (1050);
  }   
}

void displayEnvironment()  {
  if (mqtt.Update())  {
    mqttMoist.publish(moist);
    mqttTemp.publish(tempF);
    mqttPress.publish(pressHG);
    mqttDust.publish(concentration);
    mqttAQ.publish(quality);
  }  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.printf("Date: %i/%i/%i\nTime: %i:%02i\n",Time.month(),Time.day(),Time.year(),Time.hour(),Time.minute());
  display.printf("Moisture: %i %i%%\nTemp: %0.01f%cC, %0.01f%cF\n",moist,moistP,tempC,degree,tempF,degree);
  display.printf("Pressure: %0.02f inHG\nDust:%0.02f\nAir Quality: %i",pressHG,concentration,quality);
  display.display();
}