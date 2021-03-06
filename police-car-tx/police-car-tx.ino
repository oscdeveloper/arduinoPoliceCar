// The master or the transmitter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <PS2X_lib.h> // Bill Porter's PS2 Library

PS2X ps2x;  //The PS2 Controller Class


#define PIN_RF24_CE 8
#define PIN_RF24_CSN 7

#define PIN_PS2_CLK 5
#define PIN_PS2_COM 4
#define PIN_PS2_ATT 3
#define PIN_PS2_DAT 2

#define PIN_LED_STATUS 6

// nRF hash string to link up connection between transmitter/receiver
const byte nrf24Address[5] = {'l','e','g','o','1'};

RF24 radio(PIN_RF24_CE, PIN_RF24_CSN); // Create a Radio

/* 
 * this must match dataToSend in the RX
 * parameters are sending between tx/rx:
 * 0 - statusHorn
 * 1 - statusFrontTailLights
 * 2 - statusPoliceTopLights
 * 3 - motor
 * 4 - statusGear, gear change, which gear is chose
 */
unsigned int dataToSend[6]; 

bool radioWriteResult;

unsigned int statusHorn = 0;
unsigned int statusFrontTailLights = 0;
unsigned int statusPoliceTopLights = 0;
unsigned int statusGear = 1;

unsigned int speedIntervalMin;
unsigned int speedIntervalMax;

void setup() {

  // power on, led status, continuous light
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  //Serial.begin(9600);

  // setup pins and settings: GamePad(clock, command, attention, data, Pressures, Rumble)
  ps2x.config_gamepad(PIN_PS2_CLK, PIN_PS2_COM, PIN_PS2_ATT, PIN_PS2_DAT, false, false);
      
  //Serial.println("Transmitter Starting...");

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setRetries(3,5); // delay, count
  radio.openWritingPipe(nrf24Address);

}




void loop() {

  ps2x.read_gamepad(); // This needs to be called at least once a second to get data from the controller.

  statusHorn = 0;
  if ( ps2x.Button(PSB_R2) ) { // horn sound
    //Serial.println("R2 pressed - horn");
    statusHorn = 1;
  }
  dataToSend[0] = statusHorn;

  if ( ps2x.ButtonPressed(PSB_GREEN) ) { // statusFrontTailLights
    //Serial.println("Triangle 1 pressed - statusFrontTailLights");
    statusFrontTailLights = !statusFrontTailLights;
  } 
  dataToSend[1] = statusFrontTailLights;

  if ( ps2x.ButtonPressed(PSB_BLUE) ) { // statusPoliceTopLights
    //Serial.println("Cross 3 pressed - statusPoliceTopLights");
    statusPoliceTopLights = !statusPoliceTopLights;
  }
  dataToSend[2] = statusPoliceTopLights;

  // motor, only Y axis forward/reverse      
  dataToSend[3] = ps2x.Analog(PSS_RY);

  // steeringServo, only X axis left/right      
  dataToSend[4] = ps2x.Analog(PSS_RX);
  
  // gear change
  dataToSend[5] = gearChange();
  

  radioWriteResult = radio.write( &dataToSend, sizeof(dataToSend) );  

    /*Serial.print("Data Sent: ");
    Serial.print(dataToSend[0]);
    Serial.print("\t");
    Serial.print(dataToSend[1]);
    Serial.print("\t");
    Serial.print(dataToSend[2]);
    Serial.print("\t");
    Serial.print(dataToSend[3]);
    Serial.print("\t");
    Serial.print(dataToSend[4]);
    Serial.print("\t");  */

  if (radioWriteResult) {
    //Serial.println("  Acknowledge received");
  } else {
    //Serial.println("  Tx failed");

    // led status, blinking light
    digitalWrite(PIN_LED_STATUS, LOW);
    delay(100);
    digitalWrite(PIN_LED_STATUS, HIGH);
  }

  delay(20);

}


int gearChange() {

  if ( ps2x.ButtonPressed(PSB_L1) && statusGear < 3 ) {
    statusGear++;
  } else if ( ps2x.ButtonPressed(PSB_L2) && statusGear > 1 ) {
    statusGear--;
  }
  
  return statusGear;
}

