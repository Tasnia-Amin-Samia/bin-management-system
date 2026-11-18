/*
  Smart Bin Management System - ESP8266 NodeMCU + HC-SR04
  Replaced the Wi-Fi credentials, API URL, API token and bin ID before upload.

  Wiring:
    HC-SR04 VCC  -> suitable 5V supply / VIN (check board)
    HC-SR04 GND  -> GND
    HC-SR04 TRIG -> D5 (NodeMCU)
    HC-SR04 ECHO -> D6 (NodeMCU) THROUGH A VOLTAGE DIVIDER(echo->1kohm->D6->2kohm->GND) 
  Never connect the HC-SR04 5V ECHO output directly to an ESP8266 GPIO.
*/

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

#define BLYNK_TEMPLATE_ID "template id from blynk"
#define BLYNK_TEMPLATE_NAME "Bin Management"
#define BLYNK_AUTH_TOKEN "auth token from blynk"

const char* ssid = "YOUR_WIFI_NAME";
const char* pass = "YOUR_WIFI_PASSWORD";


#define TRIG D5
#define ECHO D6
#define SERVO D7


// Calibrate these distances for the actual bin.
float BIN_HEIGHT = 18.0; //sensor to bin bottom height in cm
const int OPEN_ANGLE = 180; //value for servo motor 90 degree rotation support
const int CLOSED_ANGLE = 0;  
const int NUM_READINGS = 10;
bool alertSent = false;

Servo lidServo;

//setup function runs only once
void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  lidServo.attach(SERVO);
  lidServo.write(CLOSED_ANGLE);
  delay(500);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("🗑️ BIN SYSTEM READY!");
}


void loop() {
  Blynk.run();
  checkBinLevel();
  delay(1000);
}


// read ultrasonic sensor
float getDistance() {
  long sum = 0;
  int valid = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    digitalWrite(TRIG, LOW); delayMicroseconds(2);
    digitalWrite(TRIG, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    long duration = pulseIn(ECHO, HIGH, 30000);
    if (duration > 0) { sum += duration; valid++; }
    delay(50);
  }
  if (valid == 0) return -1;
  return (sum / valid) * 0.034 / 2;
}

//calculate bin level and sends alert through blynk app 
void checkBinLevel() {
  float dist = getDistance();
  if (dist < 0 || dist > BIN_HEIGHT + 10) {
    Serial.println("⚠ Sensor error");
    return;
  }
  
  float fillPercent = 100.0 - (dist / BIN_HEIGHT * 100.0);
  fillPercent = constrain(fillPercent, 0, 100);
  
/* fillPercent = 100 - (distance/bin_height * 100)
Example:
If bin height = 18 cm
dust height = 5 cm
fill = 100 – (5/18 × 100) = 72%   */
  
  Blynk.virtualWrite(V0, fillPercent);
  Blynk.virtualWrite(V2, lidServo.read());
  
  Serial.printf("Dist: %.1f cm | Fill: %.1f%%\n", dist, fillPercent);
  
  if (fillPercent >= 80.0 && !alertSent) {
    Blynk.logEvent("bin_full", String("🚨 BIN ") + fillPercent + "% FULL!");
    alertSent = true;
  }
  if (fillPercent < 70.0) alertSent = false;
}


//bin lid on/off via manual servo control from blynk dashboard
BLYNK_WRITE(V1) {
  int state = param.asInt();
  int target = state ? OPEN_ANGLE : CLOSED_ANGLE;
  lidServo.write(target);
  delay(800);
  Serial.println(state ? "🔓 OPENED 180°" : "🔒 CLOSED 0°");
}
