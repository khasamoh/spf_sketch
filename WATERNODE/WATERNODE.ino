
#include <WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <HTTPClient.h>
#include "DHT.h"

const char* ssid     = "Khasamoh"; // Change this to your WiFi SSID
const char* password = "Khasamoh12354+"; // Change this to your WiFi password
const char* device_id = "WATERING001";
char* w_sensor_id = "WATERLEVEL00101";

#define TRIG_PIN 25 // ESP32 pin GPIO23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN 32 // ESP32 pin GPIO22 connected to Ultrasonic Sensor's ECHO pin
int relayPin=27;
int powerPin= 26;
#define REPORTING_PERIOD_MS     5000
#define WREPORTING_PERIOD_MS    20000

uint32_t lastReportTime = 0;
uint32_t wlastReportTime = 0;

float duration_us, distance_cm;
float tank_radius_cm=3.7;
float tank_volume_cm3 = 290;
float water_remain=0;
float out_distance=0;
int state = 0;

void setup() {
  Serial.begin (115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(relayPin,OUTPUT);
  pinMode(powerPin,OUTPUT);
  digitalWrite(powerPin, HIGH);
  connectWifi();
  delay(2000);


}

float DecimalRound(float input, int decimal){
  float scale  = pow(10, decimal);
  return round(input*scale)/scale;
}
void readWaterRemain(){
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration_us = pulseIn(ECHO_PIN, HIGH);
  distance_cm = 0.017 * duration_us-1;
  out_distance =distance_cm;

  Serial.print("distance: ");
  Serial.print(distance_cm);


  Serial.print(" cm ");
  float volume = 3.14 * tank_radius_cm * tank_radius_cm * distance_cm;
  Serial.print("Water Used: ");
  Serial.print(volume);
  Serial.println(" mls");

  float remain= tank_volume_cm3 - DecimalRound(volume, 3);
 

  if(remain<0 or remain>290){
      //Serial.print("Invalid Reading: ");
      //water_remain = -1;
  }else{
    water_remain = remain;
      //float randomValue = random(20, 30);
  if ((millis() - wlastReportTime > WREPORTING_PERIOD_MS))
    {   
        sendDataToWaziCloud(w_sensor_id,water_remain);
        wlastReportTime = millis();

      state = 0;
    }

  }
}

void loop() {
  readWaterRemain();
  Serial.print("Water Left: ");
  Serial.print(water_remain);
  Serial.println(" mils");
//   if(water_remain<70 && water_remain >0 && state==0){
//       digitalWrite(relayPin,HIGH);
//       state = 1;
//       lastReportTime = millis();
//   }
// if ((millis() - lastReportTime > REPORTING_PERIOD_MS) && state==1)
//     {
//       digitalWrite(relayPin,LOW);
//       state = 0;
//     }

  if(out_distance>14.00){
      digitalWrite(relayPin,HIGH);
  }else if(out_distance<7.00){
      digitalWrite(relayPin,LOW);
  }
if ((millis() - lastReportTime > REPORTING_PERIOD_MS) && state==1)
    {
      digitalWrite(relayPin,LOW);
      state = 0;
    }

  delay(1000);

}
void connectWifi(){
  WiFi.begin(ssid, password); //Initiate the wifi connection here with the credentials earlier preset

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting to WiFi...: ");
    Serial.println(ssid);
  }

  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return;
}

void sendDataToWaziCloud(char* sensor_id, float value) {

  // We cancel the send process if our board is not yet connectedd to the internet, and try reconnecting to wifi again.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    connectWifi();
    return;
  }
  Serial.println("Sending Data ....");
  HTTPClient http;

  // Initialize the API endpoint to send data. This endpoint is responsible for receiving the data we send
  String endpoint = "https://api.waziup.io/api/v2/devices/" + String(device_id) + "/sensors/" + String(sensor_id) + "/value";
 // String hum_endpoint = "https://api.waziup.io/api/v2/devices/" + String(device_id) + "/sensors/" + String(sensor_id) + "/value";
  //http.begin(temp_endpoint);
  http.begin(endpoint);

  // Header content for the data to send
  http.addHeader("Content-Type", "application/json;charset=utf-8");
  http.addHeader("accept", "application/json;charset=utf-8");

  // Data to send
  String data = "{ \"value\": " + String(value) + " }";

  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println("Response: " + response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
