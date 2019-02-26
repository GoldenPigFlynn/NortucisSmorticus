// import external libraries
#include <Adafruit_ssd1306syp.h>

//constants
#define ULTRASONIC_TRIGGER A3
#define ULTRASONIC_ECHO A2 
int In1 = 7;
int In2 = 8;
int In3 = 9;
int In4 = 11;
int ENA = 5;
int ENB = 6;
int SPEED = 210;
int LED = 12;
#define SDA_PIN A4
#define SCL_PIN A5

//global variables
Adafruit_ssd1306syp display(SDA_PIN,SCL_PIN);
boolean disableCollisionDetection = false; // HACK
boolean dead = false;
unsigned long startMotors = 0;

void stop() {
  analogWrite(ENA,0); 
  analogWrite(ENB,0); 
}

float ultrasonic_distance_test_internal(){
  // Send a 20 microsecond pulse to the trigger pin, which is the signal to send an ultrasonic beam
  // out the front end.
  digitalWrite(ULTRASONIC_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIGGER, HIGH);
  delayMicroseconds(20);
  digitalWrite(ULTRASONIC_TRIGGER, LOW);

  // After we trigger it, the ultrasonic will send us a pulse on the echo pin; the width of the pulse 
  // is the measurement of the echo delay. We divide by the magic-number of 58 to get the distance to 
  // the nearest object, in centimeters.
  float pulseWidth = pulseIn(ULTRASONIC_ECHO, HIGH);
  float distanceCm = pulseWidth / 58;
  Serial.print("Distance_test centimeters=");
  Serial.println(distanceCm);
  return distanceCm;
}

int ultrasonic_distance_test() {
  const int numTests = 3;
  float results[numTests];
  float sum = 0;
  for (int i = 0 ; i < numTests ; i++) {
    results[i] = ultrasonic_distance_test_internal();
    sum += results[i];
  }

  float avg = sum / numTests;
  Serial.print("Distance_test avg centimeters=");
  Serial.println(avg);
  return (int) avg; 
  //return ultrasonic_distance_test_internal();
}

void displayCool(String line1, String line2) {
  display.clear();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(line1);
  display.setCursor(0,20);
  display.println(line2);
  display.update();
}

void setup() {  
// Init the serial line; important for debug messages back to the Arduino Serial Monitor.
  // Make sure you set the baudrate at 9600 in Serial Monitor as well.
  Serial.begin(9600);

  
  pinMode(In1, OUTPUT);
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT);
  pinMode(In4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
   pinMode(LED, OUTPUT);
   
  pinMode(ULTRASONIC_ECHO, INPUT);
  pinMode(ULTRASONIC_TRIGGER,OUTPUT);
  dead = false;
  startMotors = 0;

  digitalWrite(LED, LOW);

  delay(1000);
 display.initialize();
}

void loop() {

  displayCool("hi there", "yoyoyo");


 // hack to make loop run only once
  if (dead) { 
    return;
  }

  if (startMotors == 0) {
    startMotors = millis();
     Serial.println("motors: start");

      // put your main code here, to run repeatedly:
     //Clockwise motor1
      digitalWrite(In1, HIGH);
      digitalWrite(In2,LOW);
    
      //Clockwise motor2
      digitalWrite(In3, HIGH);
      digitalWrite(In4,LOW);
      
      analogWrite(ENA,SPEED);
      analogWrite(ENB,SPEED); 
  }

  if (!disableCollisionDetection) { 
    int distanceCm = ultrasonic_distance_test();
    Serial.println("front distance=" + String(distanceCm));
    if (distanceCm < 20) {
      dead = true;
      digitalWrite(LED, HIGH);
      stop();
      return;
    }
  }


  if (millis() > (startMotors + 3000)) {
    Serial.println("motors: end");
    dead = true;
    stop();
  }
}
