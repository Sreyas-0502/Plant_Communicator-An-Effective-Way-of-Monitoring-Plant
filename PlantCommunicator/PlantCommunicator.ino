#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#define DHTPIN 2
#define DHTTYPE DHT11
#define I2C_SCL 5
#define I2C_SDA 4
#define pin_mode A0
#define relay 14
#define ALTITUDE 77.0  //81.0 for Trichy 77.0 for Thanjavur

SFE_BMP180 pressure;
char status;
char ssid[] = "Your SSID";
char pass[] = "Your SSID's Password";
unsigned long Channel_ID = 0; //Your Channel Number
const char *myWriteAPIKey = "Your Channel Write Key";
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
float T, P, p0, a, H, moisture;

void sendSensor() 
{
  //Humidity and Temperature from DHT11
  H = dht.readHumidity();
  T = dht.readTemperature();

  if (isnan(H) || isnan(T)) 
  {
    Serial.println("Failed to read from DHT11 Sensor");
    return;
  }

  //Absolute Pressure
  status = pressure.startPressure(3);
  if (status != 0) 
  {
    delay(status);
    status = pressure.getPressure(P, T);
    if (status != 0) 
    {
      Serial.print("Temperature : ");
      Serial.print(T, 2);
      Serial.println(" °C");
      Serial.print("Humidity : ");
      Serial.print(H, 2);
      Serial.println(" %");
      Serial.print("Absolute pressure : ");
      Serial.print(P, 2);
      Serial.println(" hPa");

      //Sea-level Pressure
      p0 = pressure.sealevel(P, ALTITUDE);
      Serial.print("relative (sea-level) pressure : ");
      Serial.print(p0, 2);
      Serial.println(" hPa");

      //Altitude
      a = pressure.altitude(P, p0);
      Serial.print("computed altitude : ");
      Serial.print(a, 0);
      Serial.print(" meters, ");
      Serial.print(a * 3.28084, 0);
      Serial.println(" feet");
    }
  }
  //Soil Moisture and Solenoid Valve
  int sensor_analog;
  sensor_analog = analogRead(pin_mode);
  moisture = (100 - ((sensor_analog / 1023.00) * 100));  //10-bit ADC so 2^n - 1 --> 2^10 - 1 = 1023
  if (moisture < 20 && H > 90) 
  {
    Serial.print("No need of water its seems to raining");
    digitalWrite(relay, HIGH);
  } 
  else if (moisture < 99 && H < 90) 
  {
    Serial.print("Solenoid valve is turned ON");
    digitalWrite(relay, LOW);
  } 
  else 
  {
    Serial.print("off condition");
    digitalWrite(relay, HIGH);
  }
  Serial.print("\nMoisture Percentage = ");
  Serial.print(moisture);
  Serial.print("%\n\n");
}

void setup() 
{
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  if (pressure.begin()) 
  {
    Serial.println("BMP180 init success");
  } 
  else 
  {
    Serial.println("BMP180 init fail\n\n");
    while (1);
  }
  dht.begin();
  WiFi.mode(WIFI_STA);  //Station mode connects to access point
  ThingSpeak.begin(client);
}

void loop() 
{  
  int statusCode = 0;
  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect");
    while (WiFi.status() != WL_CONNECTED) 
    {
      WiFi.begin(ssid, pass);
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
  delay(2000);
  sendSensor();
  delay(500);
  ThingSpeak.setField(1, P);
  ThingSpeak.setField(2, H);
  ThingSpeak.setField(3, T);
  ThingSpeak.setField(4, moisture);
  ThingSpeak.writeFields(Channel_ID, myWriteAPIKey);
}