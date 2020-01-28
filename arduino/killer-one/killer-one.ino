
// import external libraries
#include <Adafruit_ssd1306syp.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SoftwareSerial.h> 

//constants
#define ULTRASONIC_TRIGGER A3
#define ULTRASONIC_ECHO A2 
int In1 = 7;
int In2 = 8;
int In3 = 9;
int In4 = 4;
int ENA = 5;
int ENB = 6;
int SPEED = 100; //210 too fast 75 too slow
int LED = 10;
const int pResistor = A0; // Photoresistor at Arduino analog pin A0
const int pResistor2 = A1;
#define SDA_PIN A4
#define SCL_PIN A5
#define CE 2
#define CSN 3
#define SCK 13
#define MOSI 11
#define MISO 12

//global variables
Adafruit_ssd1306syp display(SDA_PIN,SCL_PIN);
boolean disableCollisionDetection = false ; // HACK
boolean dead = false;
unsigned long startMotors = 0;

 RF24 radio(CE,CSN);
 //uint8_t address[] = { 0xab,0x34,0xFC,0xBC,0xFC };
  uint8_t address[] = { 0xc2,0xc2,0xc2,0xc2,0xc2 };
 bool reader = false;
 bool writer = false;
 bool printStatus = true;

 bool autoShutOff = false;
 unsigned long startTime = 0; 
 int numReads = 0;
 int numWrites = 0;
 int numPulses = 0;
 int numCmds = 0;
 int distanceCm = 9999;

 int photoLeft;
 int photoRight;


 SoftwareSerial MyBlue(2, 3);
  
void turnLeft() {
  digitalWrite(In1, HIGH);
  digitalWrite(In2,LOW);
  digitalWrite(In3, LOW);
  digitalWrite(In4,HIGH);
        
  analogWrite(ENA,SPEED);
  analogWrite(ENB,SPEED); 
}

void turnRight() {
  digitalWrite(In3, HIGH);
  digitalWrite(In4,LOW);
  digitalWrite(In1, LOW);
  digitalWrite(In2,HIGH);

  analogWrite(ENA,SPEED);
  analogWrite(ENB,SPEED); 
}

void forward() {
  //Clockwise motor1
  digitalWrite(In1, HIGH);
  digitalWrite(In2,LOW);

  //Clockwise motor2
  digitalWrite(In3, HIGH);
  digitalWrite(In4,LOW);
  
  analogWrite(ENA,SPEED);
  analogWrite(ENB,SPEED); 
}

void stop() {
  analogWrite(ENA,0); 
  analogWrite(ENB,0); 
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

void displayStatus(String line1, String line2, String line3, String line4) {
  display.clear();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println(line1);
  display.setCursor(0,20);
  display.println(line2);
  display.setCursor(0,30);
  display.println(line3);
  display.setCursor(0,40);
  display.println(line4);
}

void displayCollisionDetect(String line1) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,50);
  display.println(line1);
  display.update();
}

void setup() {  
  // Record what time we started.
  startTime = millis();
  
  // Init the serial line; important for debug messages back to the Arduino Serial Monitor.
  // Make sure you set the baudrate at 9600 in Serial Monitor as well.
  Serial.begin(9600);
  printf_begin();

  // Set all the pins to the correct mode.
  pinMode(In1, OUTPUT);
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT);
  pinMode(In4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  pinMode(ULTRASONIC_TRIGGER,OUTPUT);
  pinMode(pResistor, INPUT);// Set pResistor - A0 pin as an input (optional)
  pinMode(pResistor2, INPUT);

  dead = false;
  startMotors = 0;

  digitalWrite(LED, LOW);
  
  delay(1000);
  display.initialize();
  displayCool("YoTE", "y0tTie");

  if (reader || writer) {
    // Init NFC2401
    radio.begin();
    //radio.setPALevel(RF24_PA_LOW);
    //radio.setPALevel(RF24_PA_MIN);
    //radio.setDataRate(RF24_250KBPS);
    //radio.setChannel(5);
    //radio.enableDynamicAck();
    //radio.setAutoAck(false);
    
    if (reader) {
      radio.openReadingPipe(1, address);
      Serial.println("starting, reader");
    } 
    
   if (writer) {
       radio.openWritingPipe(address);
        Serial.println("starting, writer");
    } 
    
    radio.startListening();
    radio.printDetails();
  }

  MyBlue.begin(9600); 
}

unsigned long lastSend = 0;
unsigned long lastStatus = 0;
bool lastDistanceCheck = false;

bool blueToothDetected = false;

void loop() {
  
    // hack 

    // Blue tooth detection
    if (MyBlue.available()) {
      blueToothDetected = true;
    }
    
 /*  flag = MyBlue.read(); 
 if (flag == 1) 
 { 
   digitalWrite(LED, HIGH); 
   Serial.println("LED On"); 
 } 
 else if (flag == 0) 
 { 
   digitalWrite(LED, HIGH); 
   Serial.println("LED Off"); 
 } 
}  */

    // to make loop run only once
    if (dead) { 
      //displayCool("DEAD", "");
      return;
    }
  
    //Do Light detection
    photoLeft = analogRead(pResistor);
    photoRight = analogRead(pResistor2);
    Serial.println("Left = " + String(photoLeft));
    Serial.println("Right = " + String(photoRight));
    
    //You can change value "25"
    /*if (photoLeft < 25 || photoRight < 25) {
      digitalWrite(LED, HIGH);  //Turn led off
    } else {
      digitalWrite(LED, LOW); //Turn led on
    }*/

    //turn towards light
    int lightDif = photoLeft - photoRight;
    boolean goForward = false;
    if(photoLeft > photoRight && lightDif > 25) {
      turnLeft();
      delay(100);
      stop();
      Serial.println("turning left towards light");
    } else if (photoRight > photoLeft && lightDif < -25) {
      turnRight();
      delay(100);
      stop();
      Serial.println("turning right towards light");
    } else {
      //stop();
      goForward = true;
      Serial.println("stopping: under 25 lightDif");
    }
    
    unsigned long upTime = millis() - startTime;
    int upSecs = upTime / 1000;
    displayStatus(blueToothDetected ? "ONLINE" : "OFFLINE", 
      "runtime: " + String(upSecs), 
      "io: " + String(numReads) + "/" + String(numPulses) + "/" + String(numCmds) + "/" + String(numWrites),
      "objective: " + String("KILL"));
    displayCollisionDetect("distance: " + String(distanceCm));
    display.update();
  
    if ((reader || writer) && printStatus) {
      unsigned long now = millis();
      if ((now - lastStatus) > 20000) {
         radio.printDetails();
         lastStatus = now;
      }
    }
      
     if (writer) {
      unsigned long now = millis();
      if ((now - lastSend) > 5000) {
        radio.stopListening();     
        unsigned long pulseCmd = 111;
         if (!radio.write(&pulseCmd, sizeof(unsigned long), true)){
           Serial.println(F("failed on pulse"));
         } else {
          Serial.println("pulse sent");
         }
         numWrites++;
        radio.startListening();
        lastSend = now;
      }
     }
  
     if (reader) {
  
      uint8_t pipeNum;
      if ( radio.available(&pipeNum) ){                
        unsigned long msg[4];
        radio.read( &msg, sizeof(unsigned long) * 4);
        numReads++;
        Serial.println("net: (" + String(millis()) + ") pipe: " + String(pipeNum) + 
          ", read: " + String(msg[0]) + ":" + String(msg[1])  + ":" + String(msg[2])  + ":" + String(msg[3]) + 
          ", size=" + String(radio.getPayloadSize()) +
          ", channel=" + String(radio.getChannel()));
         
        if (msg[0] == 111) {
          numPulses++;
          digitalWrite(LED, LOW);
        } else if (msg[0] == 666) {
          numCmds++;
          digitalWrite(LED, HIGH);
          displayCool("Avoided Collision", "Stopped Because of  BUDDY");
          //stop();
          //dead = true;
        }
      }
    }
  
   /* if (startMotors == 0) {
      startMotors = millis();
       Serial.println("motors: start");

       turnLeft();
       delay(1000);
       turnRight();
       delay(1000);
       Serial.println("motors: end");
    } */

  
    if (goForward) { 
     
      if (distanceCm > 15) {
        
        lastDistanceCheck = true;
        forward();
        delay(250);
        stop();

       /* dead = true;
        digitalWrite(LED, HIGH);
        displayCool("Avoided Collision", "Stopped Because of   Obstacle in Path");
        stop();
  
        if (writer) {
          // make other leds go off
          radio.stopListening();     
          unsigned long msg = 666;
          if (!radio.write( &msg, sizeof(unsigned long), true )){
             Serial.println(F("failed off update"));
           }
          radio.startListening();  
        } */
        
      } else {
        if (lastDistanceCheck) {
          lastDistanceCheck = false;
           /* if (writer) {
          radio.stopListening();     
          unsigned long pulseCmd = 111;
           if (!radio.write(&pulseCmd, sizeof(unsigned long), true)){
             Serial.println(F("failed on update"));
           } else {
            Serial.println("mesg sent update-on");
           }
           numWrites++;
          radio.startListening();
        } */
        }
        stop();
      }
    }
  
   if (autoShutOff) { 
      /* HACK: autoshutoff the robot after 3 seconds. */
      if (millis() > (startMotors + 3000)) {
        Serial.println("motors: end");
        dead = true;
        displayCool("Auto-Stop", "Stopped due to shutdown timer");
        stop();
      }
  }
}
