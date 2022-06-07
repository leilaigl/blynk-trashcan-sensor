#include <Ultrasonic.h>
#include <ESP8266WiFi.h>
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>

Ultrasonic ultrasonic(D3,D2);

const char* ssid = "HotspotSSID";
const char* pass = "HotspotPassword";
char auth[] = "AuthTokenForBlynkApp";

int distance = 0;
int initDistance = 0;
int percentage;
int thresh[2] = {50, 75}; //percentage thresholds
int count = 0; //counter for each distance measurement
int timer = 0; //counter for 2 days notification
int countLCD = 0; //LCD state

unsigned long initTime, currentTime;

WidgetLED green(V1);
WidgetLED orange(V2);
WidgetLED red(V3);
WidgetLCD lcd(V6);

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting... ");
    Serial.printf("Connection Status: %d\n", WiFi.status());
    delay(1000);
  }
  Serial.println("Wi-Fi connected.");
  Blynk.begin(auth, ssid, pass);
  lcd.clear();
  initDistance = ultrasonic.read();
  initTime = millis();
  Serial.println(initDistance);
  Serial.println("Done Setup");
}

void loop() {
  Serial.println("____________________");
  Serial.printf("count#: %d\n", count);
  Serial.printf("initDistance: %d\n", initDistance);
  distance = ultrasonic.read();
  currentTime = millis();

  if (currentTime - initTime >= 60000) {
    timer++;
    Serial.printf("TIME OUT");
    if (timer >= 1) {
      lcd.print(0, 0, "2 DAYS PASSED! ");
      lcd.print(0, 1, "Take out trash! ");
      if (timer == 1) {
        Blynk.notify("Two days have passed, take out your trash before it
        smells!");
      }
    }
  }

  Serial.printf("Raw: %d\n", distance);
  int difference = 0;
  difference = ((initDistance)-(distance));
  if (difference < 0) difference = 0;
  Serial.printf("difference: %d\n", difference);
  percentage = 100*difference/initDistance;
  Serial.printf("percentage: %d\n", percentage);
  Blynk.run();
  Blynk.virtualWrite(V4, percentage);
  Blynk.virtualWrite(V5, percentage);

  if (percentage <= 5) {
    count = 0;
    timer = 0;// timer
    countLCD = 0;// LCD
    initTime = millis();
  }

  if(percentage <= thresh[0]){ //when distance < 50%
    orange.off();
    red.off();
    green.on();
    if (countLCD > 1) { // wasn't green before
      lcd.clear();
    }
    if (timer < 1) { // not passed 2 days
      lcd.print(0, 0, "Status: ");
      lcd.print(0, 1, "Very Healthy! ");
      countLCD = 1;
    }
  }
  else if(percentage <= thresh[1]) { //when distance < 75%
    red.off();
    green.on();
    orange.on();
    if (countLCD != 2) { //wasn't orange before
      lcd.clear();
    }
    if (timer < 1) { //not passed 2 days
      lcd.print(0, 0, "Status: ");
      lcd.print(0, 1, "Halfway full! ");
      countLCD = 2;
    }
  }
  else {
    count++;
    green.on();
    orange.on();
    red.on();
    if (countLCD != 3) {
      lcd.clear();
    }
    if (timer < 1) {
      lcd.print(0, 0, "Status: ");
      lcd.print(0, 1, "Take out trash! ");
      countLCD = 3;
    }
  }
  if (count == 1) {
    Blynk.notify("Your trash is more than 75%. Take it out!");
  }
  delay(1000);
}
