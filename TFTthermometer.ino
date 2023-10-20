#include <TFT.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ezButton.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define cs 10
#define dc 9
#define rst 8

DHT_Unified dht(DHTPIN, DHTTYPE);
TFT TFTscreen = TFT(cs, dc, rst);
ezButton button0(A0);
ezButton button1(A1);
ezButton button2(A2);

uint8_t state = 0;                  // initial state for statemachine
const uint32_t INTERVAL_MS = 60000;  // interval in ms for statemachine
char sensorPrintout[5];
char maxTmp[5];
char maxHum[5];
char minTmp[5];
char minHum[5];
float maxTemp;
float maxHumi;
float minTemp = 99.9;
float minHumi = 99.9;

const int buttonPin0 = A0;
const int buttonPin1 = A1;
const int buttonPin2 = A2;

int buttonState0;
//int buttonState1;
//int buttonState2;

const int SHORT_PRESS_TIME = 1000;
const int LONG_PRESS_TIME = 3000;

unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;

void setup() {
  button0.setDebounceTime(50);
  button1.setDebounceTime(50);
  button2.setDebounceTime(50);
  TFTscreen.begin();
  dht.begin();
  drawScreen();
  nextState();
}
void loop() {
  static unsigned long timer = millis();

  if (millis() - timer >= INTERVAL_MS) {
    timer = millis();
    state = 0;
    nextState();
  }
  readButtons();
}
void nextState() {
  switch (state) {
    case 0:
      drawSensor();
      getMax();
      getMin();
      break;
    case 1:
      drawMax();
      break;
    case 2:
      drawMin();
      break;
    case 3:
      drawMax();
      break;
    default:
      //state = -1;
      TFTscreen.background(0, 0, 0);
      TFTscreen.stroke(0, 0, 255);
      TFTscreen.text("Error!", 0, 20);
      break;
  }
}
void readButtons() {
    button0.loop(); // MUST call the loop() function first

  if(button0.isPressed()){
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  }

  if(button0.isReleased()) {
    isPressing = false;
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if( pressDuration < SHORT_PRESS_TIME )
      state++;
      if (state > 2)  {
        state = 0;
      }
    nextState();
  }

  if(isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if( pressDuration > LONG_PRESS_TIME ) {
      resetMinMax();
      nextState();
      isLongDetected = true;
    }
  }
}
void drawScreen() { // draw Basic Sensor screen
  TFTscreen.background(0, 0, 0);

  // write small text labels
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Temperatur:\n ", 5, 3);
  TFTscreen.text("Luftfeuchtigkeit:\n ", 5, 70);

  // draw Celsius degree
  TFTscreen.stroke(0, 252, 124);
  TFTscreen.setTextSize(4);
  TFTscreen.text("C", 132, 20);
  TFTscreen.fill(0, 252, 124);
  TFTscreen.rect(115, 20, 8, 8);

  //draw Humidity percent
  TFTscreen.stroke(225, 105, 65);
  TFTscreen.text("%h", 110, 90);

  // draw middle line
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.fill(255, 255, 255);
  TFTscreen.rect(0, 60, 160, 3);
}
void drawSensor()  {
  // delete MIN/MAX labels
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect(129, 2, 20, 10);
  TFTscreen.rect(129, 69, 20, 10);

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.fill(0, 0, 0);
    TFTscreen.rect(0, 20, 160, 35);

    TFTscreen.stroke(255, 0, 0);
    TFTscreen.text("Error!", 0, 20);
  } else {
    String sensorVal = String(event.temperature);
    sensorVal.toCharArray(sensorPrintout, 5);
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.fill(0, 0, 0);
    TFTscreen.rect(5, 20, 100, 35);

    TFTscreen.stroke(34, 139, 34);
    TFTscreen.text(sensorPrintout, 5, 20);
  }
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.fill(0, 0, 0);
    TFTscreen.rect(0, 90, 160, 35);
  } else {
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.fill(0, 0, 0);
    TFTscreen.rect(5, 90, 100, 35);

    String sensorVal = String(event.relative_humidity);
    sensorVal.toCharArray(sensorPrintout, 5);
    TFTscreen.stroke(235, 206, 135);
    TFTscreen.text(sensorPrintout, 5, 90);
  }
}
void getMax() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temp(event.temperature);
  if (temp > maxTemp)  {
    maxTemp = temp;
  }
  String sensorValTemp = String(maxTemp);
  sensorValTemp.toCharArray(maxTmp, 5);

  dht.humidity().getEvent(&event);
  float humi(event.relative_humidity);
  if (humi > maxHumi)  {
    maxHumi = humi;
  }
  String sensorValHumi = String(maxHumi);
  sensorValHumi.toCharArray(maxHum, 5);
}
void getMin() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temp(event.temperature);
  if (temp < minTemp)  {
    minTemp = temp;
  }
  String sensorValminTemp = String(minTemp);
  sensorValminTemp.toCharArray(minTmp, 5);

  dht.humidity().getEvent(&event);
  float humi(event.relative_humidity);
  if (humi < minHumi)  {
    minHumi = humi;
  }
  String sensorValminHumi = String(minHumi);
  sensorValminHumi.toCharArray(minHum, 5);
}
void drawMax()  {
  TFTscreen.stroke(0, 255, 255);
  TFTscreen.fill(0, 0, 255);
  TFTscreen.rect(129, 2, 20, 10);
  TFTscreen.stroke(0, 255, 255);
  TFTscreen.fill(0, 0, 255);
  TFTscreen.rect(129, 69, 20, 10);
  // write small text labels
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("MAX\n ", 130, 3);
  TFTscreen.text("MAX\n ", 130, 70);
  TFTscreen.setTextSize(4);

  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect(5, 20, 100, 35);

  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(maxTmp, 5, 20);
  
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect(5, 90, 100, 35);

  TFTscreen.stroke(235, 206, 135);
  TFTscreen.text(maxHum, 5, 90);
}
void drawMin()  {
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.fill(255, 0, 0);
  TFTscreen.rect(129, 2, 20, 10);

  TFTscreen.stroke(0, 255, 0);
  TFTscreen.fill(255, 0, 0);
  TFTscreen.rect(129, 69, 20, 10);
  // write small text labels
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("MIN\n ", 130, 3);
  TFTscreen.text("MIN\n ", 130, 70);
  TFTscreen.setTextSize(4);

  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect(5, 20, 100, 35);

  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(minTmp, 5, 20);
  
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect(5, 90, 100, 35);

  TFTscreen.stroke(235, 206, 135);
  TFTscreen.text(minHum, 5, 90);
}
void resetMinMax() {
  maxTmp[0] = '\0';
  maxHum[0] = '\0';
  minTmp[0] = '\0';
  minHum[0] = '\0';
}