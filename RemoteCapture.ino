 /*
******************************************************************************************************************************************************************
*
* Markisol protocol remote control capture
* Compatible with remotes like BF-101, BF-301, BF-305, possibly others
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Use this code to capture the commands from your remotes. Outputs to serial
* (Tools -> Serial Monitor). What you need for mastering your shades are 41
* bits commands. Protocol is described in Markisol.ino.
* 
* 
* HOW TO USE
* 
* Plug a 433.92MHz receiver to digital pin 2 and start pressing buttons
* from your original remotes (copy pasting them to Markisol.ino).
* 
* 
* NOTE ABOUT THE L (LIMITS) BUTTON
* 
* You have to hold down this button for 6 seconds before the remote
* starts transmitting.
*
******************************************************************************************************************************************************************
*/



// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   2

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// data bit capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG         false
#define ADDITIONAL    false    // Display some additional info after capture



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  int i = 0;
  unsigned long t = 0;
  String command = "";


  // *********************************************************************
  // Wait for the first AGC:
  // *********************************************************************
  // HIGH between 4500-6000 us
  // *********************************************************************
  
  while (t < 4500 || t > 6000) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // / Waits for HIGH and times it
    
    if (DEBUG) { // For finding AGC timings
      if (t > 3000 && t < 7000) Serial.println(t);
    }
  }

  if (DEBUG) {
    Serial.print("AGC 1: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for the second AGC:
  // *********************************************************************
  // LOW between 2300-2600 us
  // *********************************************************************
  
  while (t < 2300 || t > 2600) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // / Waits for LOW and times it
    
    if (DEBUG) { // For finding AGC timings
      if (t > 2300 && t < 2600) Serial.println(t);
    }
  }

  if (DEBUG) {
    Serial.print("AGC 2: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for the third AGC:
  // *********************************************************************
  // HIGH between 1100-1900 us
  // *********************************************************************
  
  while (t < 1100 || t > 1900) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it

    if (DEBUG) { // For finding AGC timings
      if (t > 500 && t < 3000) Serial.println(t);
    }
  }

  if (DEBUG) {
    Serial.print("AGC 3: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 200-400 us
  // 1 = 500-800 us
  // *********************************************************************

  while (i < 41) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 500 && t < 800) { // Found 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else if (t > 200 && t < 400) { // Found 0
      command += "0";
      if (DEBUG) Serial.println("0");
      
    } else { // Unrecognized bit, finish
      if (ADDITIONAL) {
        Serial.print("INVALID TIMING: ");
        Serial.println(t);
      }
      i = 0;
      break;
    }

    i++;
  }

  // *********************************************************************
  // Done! Display results:
  // *********************************************************************  

  // Correct data bits length is 41 bits, dismiss bad captures:
  if (command.length() != 41) {
    
    if (ADDITIONAL) {
      Serial.print("Bad capture, invalid command length ");
      Serial.println(command.length());
      Serial.println();
    }
    
  } else {
    Serial.println("Successful capture, full command is: " + command);
    Serial.println("Remote control (unique) ID: " + command.substring(0, 16));
    Serial.println("Channel: " + printChannel(command.substring(16, 20)));
    Serial.println("Command: " + printCommand(command.substring(20, 24)));
    Serial.println("Remote control model: " + printRemoteModel(command.substring(24, 32)));
    
    if (ADDITIONAL) {
      Serial.print("Remote control ID (DEC): ");
      Serial.println(convertBinaryStringToInt(command.substring(0, 16)), DEC);
      Serial.print("Trailing bits: ");
      Serial.println(convertBinaryStringToInt(command.substring(32, 41)), DEC);
    }
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printChannel(String channel) {

  int c = convertBinaryStringToInt(channel);

  switch (c) {

    case 0x8: // 1000
      return "1"; // ...or remote model BF-301

    case 0x4: // 0100
      return "2"; // ...or remote model BF-101

    case 0xC: // 1100
      return "3";

    case 0x2: // 0010
      return "4";

    case 0xA: // 1010
      return "5";

    case 0xF: // 1111
      return "ALL";

  }
  return "UNKNOWN/NEW";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printCommand(String command) {

  int c = convertBinaryStringToInt(command);

  switch (c) {
    
    case 0x8: // 1000
      return "DOWN";

    case 0x3: // 0011
      return "UP";

    case 0xA: // 1010
      return "STOP";

    case 0x2: // 0010
      return "PAIR/CONFIRM (C)";

    case 0x4: // 0100
      return "PROGRAM LIMITS (L)";

    case 0x1: // 0001
      return "CHANGE DIRECTION OF ROTATION (STOP + L)";

  }
  return "UNKNOWN";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printRemoteModel(String model) {

  int c = convertBinaryStringToInt(model);

  switch (c) {

    case 0x86: // 10000110
      return "BF-305 (5 channels)";

    case 0x3: // 00000011
      return "BF-101 (single channel)";

    case 0x83: // 10000011
      return "BF-301 (single channel)";
      
  }
  return "UNKNOWN/NEW";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
long convertBinaryStringToInt(String s) {
  int c = 0;
  long b = 0;
  
  for (int i = 0; i < s.length(); i++) {
    c = convertStringToInt(s.substring(i, i + 1));
    b = b << 1;
    b += c;
  }
  
  return b;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
int convertStringToInt(String s) {
  char carray[2];
  int i = 0;
  
  s.toCharArray(carray, sizeof(carray));
  i = atoi(carray);

  return i;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
