#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// WiFi credentials (Wokwi's built-in network)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak
const char* thingSpeakAPIKey = "QTQ79FDR4XALJBL8
";
const char* thingSpeakURL = "http://api.thingspeak.com/update";

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize I2C and MPU6050
  Wire.begin(21, 22);
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found! Check wiring.");
    while (1) delay(10);
  }
  Serial.println("MPU6050 ready.");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print to Serial
  Serial.print("Accel X: "); Serial.print(a.acceleration.x);
  Serial.print(" Y: "); Serial.print(a.acceleration.y);
  Serial.print(" Z: "); Serial.println(a.acceleration.z);
  Serial.print("Gyro  X: "); Serial.print(g.gyro.x);
  Serial.print(" Y: "); Serial.print(g.gyro.y);
  Serial.print(" Z: "); Serial.println(g.gyro.z);

  // Send to ThingSpeak via HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(thingSpeakURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "api_key=" + String(thingSpeakAPIKey)
                    + "&field1=" + String(a.acceleration.x)
                    + "&field2=" + String(a.acceleration.y)
                    + "&field3=" + String(a.acceleration.z)
                    + "&field4=" + String(g.gyro.x)
                    + "&field5=" + String(g.gyro.y)
                    + "&field6=" + String(g.gyro.z);

    int httpCode = http.POST(postData);
    Serial.print("HTTP response: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println("Response: " + http.getString());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }

  delay(20000); // ThingSpeak free tier: min 15s between update
}
