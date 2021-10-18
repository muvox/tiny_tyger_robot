#include <Arduino.h>
#include <string.h>


// BLE Pins:
#define BLE_RX  PA3
#define BLE_TX  PA2

// Define motor controller pins
#define MTR_A1 PA6
#define MTR_A2 PA7
#define MTR_B1 PB0
#define MTR_B2 PB1

// Prototype functions
void recieveBLEValues();
void controlMotors();

// maximum number of character that can be stored at once from serial
const byte numChars = 32;

// char array to store the chars
char receivedChars[numChars];

// Is new valid data incoming?
boolean newData = false;

void setup()
{
  // Wait for the board and BLE module to properly power up
  delay(3000);

  //Setup motor pins
  pinMode(MTR_A1, OUTPUT);  //Left forward
  pinMode(MTR_A2, OUTPUT);  //Left backward
  pinMode(MTR_B1, OUTPUT);  //Right backward
  pinMode(MTR_B2, OUTPUT);  //Right foward

  //Initiate serial connection between board and the BLE module
  Serial2.begin(115200); 
  delay(3000);
}

void loop() {
  recieveBLEValues();
  controlMotors();
}

// Scans the serial input for start marker (<)
// then starts storing data into receivedChars array until end marker (>)
// comes in via serial
void recieveBLEValues(){
  static bool recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  // Read serial
  while (Serial2.available() > 0 && newData == false) {
    rc = Serial2.read();

    // if recieve in progress, start storing characters
    // to receivedChars at index, unless the character
    // is a endmarker
    if (recvInProgress == true) {
      if ( rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars -1;
        }
      } else {
        // end marker recieved, slap end string null
        receivedChars[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
      // if recieved character is the start marker
      // set recieve in progress to true
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

// Function for sending motor control commands to L298N
// All the SerialUSB.print/ln are for debuggin
void controlMotors() {
  // If new data has arrived
  if (newData == true) {
    char * pch;
    pch = strtok(receivedChars,"Y"); // The app pushed X and Y values, parsing Y here.
    if (pch[0] == 'L') { // If first character of data is L, it is referring to left.
      char* command = pch+2; // Remove axis and side identifier
      int power;
      sscanf(command, "%d", &power); // Serial input parsed into a numbe value (-255 - 255)
      if (power > 0) { 
        // forward
        analogWrite(MTR_A2, 0);
        analogWrite(MTR_A1, power);
      } else if (power < 0) { 
        // reverse
        analogWrite(MTR_A1, 0);
        analogWrite(MTR_A2, abs(power));
      } else {
        // both off
        analogWrite(MTR_A1, 0);
        analogWrite(MTR_A2, 0);
      }
    } else {
      //right motor
      char* command = pch+2;
      int power;
      sscanf(command, "%d", &power); // Serial input parsed into a numbe value (-255 - 255)
      if (power > 0) {
        // forward
        analogWrite(MTR_B2, 0);
        analogWrite(MTR_B1, power);
      } else if (power < 0) {
        // reverse
        analogWrite(MTR_B1, 0);
        analogWrite(MTR_B2, abs(power));
      } else {
        // both off
        analogWrite(MTR_B1, 0);
        analogWrite(MTR_B2, 0);
      }
    }
    newData = false;
  }
}
