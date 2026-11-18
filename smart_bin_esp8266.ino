/*
  Smart Bin Management System - ESP8266 NodeMCU + HC-SR04
  Replace the Wi-Fi credentials, API URL, API token and bin ID before upload.

  Wiring:
    HC-SR04 VCC  -> suitable 5V supply / VIN (check board)
    HC-SR04 GND  -> GND
    HC-SR04 TRIG -> D5 (GPIO14)
    HC-SR04 ECHO -> D6 (GPIO12) THROUGH A VOLTAGE DIVIDER
  Never connect the HC-SR04 5V ECHO output directly to an ESP8266 GPIO.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* API_URL = "http://192.168.0.105/smart-bin-management-iot/api/update.php";
const char* API_TOKEN = "CHANGE_THIS_TO_A_LONG_RANDOM_SECRET";
const char* BIN_ID = "BIN-001";

const uint8_t TRIG_PIN = D5;
const uint8_t ECHO_PIN = D6;

// Calibrate these distances for the actual bin.
const float EMPTY_DISTANCE_CM = 30.0; // sensor to bottom when bin is empty
const float FULL_DISTANCE_CM = 5.0;   // sensor to waste when bin is considered full
const int FULL_THRESHOLD = 80;

unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL_MS = 10000;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout avoids blocking forever if no echo is received.
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1.0;

  return (duration * 0.0343f) / 2.0f;
}

int calculateFillPercent(float distanceCm) {
  float range = EMPTY_DISTANCE_CM - FULL_DISTANCE_CM;
  if (range <= 0) return 0;

  float percent = ((EMPTY_DISTANCE_CM - distanceCm) / range) * 100.0f;
  percent = constrain(percent, 0.0f, 100.0f);
  return (int)(percent + 0.5f);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 20000UL) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. ESP8266 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection timed out.");
  }
}

void sendReading(float distanceCm, int fillPercent) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, API_URL)) {
    Serial.println("Could not start HTTP connection.");
    return;
  }

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = "token=" + String(API_TOKEN)
              + "&bin_id=" + String(BIN_ID)
              + "&distance_cm=" + String(distanceCm, 2)
              + "&fill_percent=" + String(fillPercent);

  int responseCode = http.POST(body);
  Serial.print("Server response: ");
  Serial.println(responseCode);

  if (responseCode > 0) {
    Serial.println(http.getString());
  } else {
    Serial.println(http.errorToString(responseCode));
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  connectWiFi();
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL_MS) {
    lastSend = millis();

    float distanceCm = readDistanceCm();
    if (distanceCm < 0) {
      Serial.println("No sensor echo. Check wiring and sensor position.");
    } else {
      int fillPercent = calculateFillPercent(distanceCm);
      Serial.print("Distance: ");
      Serial.print(distanceCm);
      Serial.print(" cm | Fill: ");
      Serial.print(fillPercent);
      Serial.println("%");

      sendReading(distanceCm, fillPercent);
    }
  }
}
