#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Servo.h>

#define PIN_RF24_CE 8
#define PIN_RF24_CSN 7

#define PIN_STEERING_SERVO 3

#define PIN_MOTOR_FORWARD 5
#define PIN_MOTOR_REVERSE 6

const byte thisSlaveAddress[5] = {'l','e','g','o','1'};
RF24 radio(PIN_RF24_CE, PIN_RF24_CSN);

int dataReceived[6]; // this must match dataToSend in the TX

Servo steeringServo;

unsigned int motorSpeed;
unsigned int steering;

void setup() {

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.openReadingPipe(1, thisSlaveAddress);
  radio.startListening();

  pinMode(PIN_STEERING_SERVO, OUTPUT);
  steeringServo.attach(PIN_STEERING_SERVO, 500, 2000);
  steeringServo.write(90);
  
  pinMode(PIN_MOTOR_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_REVERSE, OUTPUT); 
  analogWrite(PIN_MOTOR_FORWARD, 0); 
  analogWrite(PIN_MOTOR_REVERSE, 0); 

}

void loop() {

  if ( radio.available() ) {
    
    radio.read( &dataReceived, sizeof(dataReceived) );

    // motor
    if ( dataReceived[3] < 128 ) { // forward
      motorSpeed = map(dataReceived[3], 127, 0, 100, 255);
      analogWrite(PIN_MOTOR_FORWARD, motorSpeed);
      digitalWrite(PIN_MOTOR_REVERSE, 0);
    } else if ( dataReceived[3] > 128 ) { // reverse
      motorSpeed = map(dataReceived[3], 129, 255, 100, 255);
      digitalWrite(PIN_MOTOR_FORWARD, 0);
      analogWrite(PIN_MOTOR_REVERSE, motorSpeed);
    } else { // stop
      analogWrite(PIN_MOTOR_FORWARD, 0);
      analogWrite(PIN_MOTOR_REVERSE, 0);         
    }    


    // steering left/right
    if ( dataReceived[4] < 128 ) { // left
      steering = map(dataReceived[4], 127, 0, 91, 180);
    } else if ( dataReceived[4] > 128 ) { // right
      steering = map(dataReceived[4], 129, 255, 89, 0);
    } else { // neutral, straight
      steering = 90;    
    }
    steeringServo.write(steering);    

  }

}
