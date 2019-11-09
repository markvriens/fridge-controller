#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 13    //DS18B20 pin
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_FEATHER52832)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

#define RELAY1 12
#define RELAY2 14

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer = { 0x28, 0xD5, 0xD2, 0xC6, 0x07, 0x00, 0x00, 0xA6 };

float tempC = -130.0; //Default value by startup
float MinTemp = 20.0;
float MaxTemp = 21.0;
int buttonState = 0;
int programMode = 0;
int buttonUp = 0;
int buttonDown = 0;
unsigned long currentMillis = 0;
unsigned long previousCoolingMillis = 0;
const int coolingDuration = 50000;
int ControlStatus = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("OLED FeatherWing test");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  Serial.println("OLED begun");
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  //Clear the buffer.
  display.clearDisplay();
  display.display();

  Serial.println("SET BUTTON IO");
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  
  Serial.println("SET RELAY IO");
  pinMode(RELAY1, HIGH);
  pinMode(RELAY2, HIGH);
  
  Serial.println("Start sensors");
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(insideThermometer, 12);
  
  // text display startup
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display(); // actually display all of the above
}

void statusMessage(){
  // Clear the buffer.
  display.clearDisplay();
  display.display();
  // text display tests
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("Act: ");
  display.println(tempC);
  display.print("Min ");
  display.print(MinTemp);
  display.print(" - Max ");
  display.println(MaxTemp);
  if(ControlStatus == 0)
    display.print("Neutral");
  if(ControlStatus == 1)  
    display.print("Cooling");
  if(ControlStatus == 2)  
    display.print("Warming");
  display.display(); // actually display all of the above
}
void mainControl(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures();
  tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
  } 
  else{
    if(tempC > MaxTemp)
      {
        Serial.println("cooling");
         ControlStatus = 1;
         previousCoolingMillis = millis();
      }
      if(tempC < MinTemp)
      {
         Serial.println("warming");
         ControlStatus = 2;
      }
      if (tempC < MaxTemp & tempC > MinTemp) {
        if(ControlStatus == 1){
          if(millis() - previousCoolingMillis > coolingDuration){
            ControlStatus = 0;
          }
        }
        else
        {
          ControlStatus = 0;
        }
      }
      Serial.println(ControlStatus);
      //Visual and Set the Relay
      if(ControlStatus == 0){
        //lcd.setBacklight(GREEN);
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
      }
      if(ControlStatus == 1){
        //lcd.setBacklight(BLUE);
        digitalWrite(RELAY1, HIGH);
        digitalWrite(RELAY2, LOW); 
      }
      if(ControlStatus == 2){
        //lcd.setBacklight(BLUE);
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, HIGH);
      }
  }
}

void setProgram(){
  buttonState = digitalRead(BUTTON_B);

  if(!digitalRead(BUTTON_B)){
    Serial.print("program");
    programMode++;
    if(programMode > 2)
    {
      programMode = 0;
    }
    delay(1000);
    Serial.print(programMode);
  }
  //Set the minimale temp
  if(programMode == 1)
  {
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    display.println("Set minimal temp: ");
    if (!digitalRead(BUTTON_C)) {
      MinTemp = MinTemp - 0.1;
    } 
    if (!digitalRead(BUTTON_A)) {
      MinTemp = MinTemp + 0.1;
    }
    MaxTemp = MinTemp + 0.5;
    display.print(MinTemp);
    display.display();
    delay(500);
  }
  //Set the max temp
  if(programMode == 2)
  {
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    display.println("Set max temp: ");
    if(MaxTemp <= MinTemp)
    {
      MaxTemp = MinTemp + 0.5;
    }
    if (!digitalRead(BUTTON_C)) {
      MaxTemp = MaxTemp - 0.1;
    } 
    if (!digitalRead(BUTTON_A)) {
      MaxTemp = MaxTemp + 0.1;
    }
    display.print(MaxTemp);
    display.display();
    delay(500);
  }
}

void loop() {
  setProgram();
  if(programMode == 0){
    mainControl(insideThermometer);
    statusMessage();
    delay(10);
  }

}

