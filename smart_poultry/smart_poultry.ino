#include "DHT.h"
#include <WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <HTTPClient.h>
#include "DHT.h"
const char* device_id = "ENV0001";
char* t_sensor_id = "TEMP00101";
char* h_sensor_id = "HUM00101";

const char* ssid     = "Khasamoh"; // Change this to your WiFi SSID
const char* password = "Khasamoh12354+"; // Change this to your WiFi password

#define DHTPIN 32     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float humidity=0;
float temperature=0;
int lightsensorPin = 34; //define analog piin 34
int lampPin = 14;
int fanPin = 27;
int lightsensorvalue = 0;
float heatIndex;
int powerLedPin = 16;
int lampstate= 0;
int fanState = 0;



void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  pinMode(lampPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(powerLedPin, OUTPUT);
  digitalWrite(powerLedPin, HIGH);
  dht.begin();
  connectWifi();
  delay(5000);
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);
  readSensorValues();
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
   Serial.print(F("%  Heat Index: "));
  Serial.print(heatIndex); 
  Serial.print(F("°C "));
  Serial.print("light: ");
  Serial.println(lightsensorvalue);
  actuation();

   //float randomValue = random(20, 30);
  sendDataToWaziCloud(h_sensor_id,humidity);
  delay(5000); // Wait for 1 minutes (in ms) to send the next generated random value
  sendDataToWaziCloud(t_sensor_id,temperature);
  delay(5000);
  //actuation();
  
}

void readSensorValues(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float light = analogRead(lightsensorPin); 
  lightsensorvalue=map(light, 4010, 0, 100, 0);


   // float hic = dht.computeHeatIndex(t, h, false);

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }else{
    humidity = h;
    temperature = t;
    heatIndex = dht.computeHeatIndex(t, h, false);

  }

}

void actuation(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float light = analogRead(lightsensorPin); 
  lightsensorvalue=map(light, 4000, 0, 100, 0);
  // if (lightsensorvalue>60 and lampstate==0) {
  //   digitalWrite(lampPin, HIGH);
  //   lampstate = 1;
  // }else if(lightsensorvalue<=60 and lampstate==1){
  //   digitalWrite(lampPin, LOW);
  //   lampstate = 0;
  // }

  if (temperature>29.00) {
    //fanState = 1;
    digitalWrite(fanPin, HIGH);
    delay(1000);
    digitalWrite(lampPin, LOW);
  }else if (temperature<=28.99){
    //fanState = 0;
    digitalWrite(fanPin, LOW);
    delay(1000);
    digitalWrite(lampPin, HIGH);
    
  }

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
