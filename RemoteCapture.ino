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
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  int i = 0;
  unsigned long t = 0;


  // As a quick hack, we add the leading 0 here.
  // The way I wrote the code misses the first data bit.
  // However, every Markisol protocol command starts with
  // it, so function-wise it's not a big deal:
  String command = "0";


  // *********************************************************************
  // Wait for the first AGC bit:
  // *********************************************************************
  // HIGH between 2430-2495 us
  // *********************************************************************
  
  while (t < 2430 || t > 2495) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Waits for a HIGH waveform spike (low-HIGH-low)

    if (DEBUG) { // For finding AGC timings
      if (t > 2000 && t < 3000) Serial.println(t);
    }
  }

  if (DEBUG) {
    Serial.print("AGC 1: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for second AGC bit:
  // *********************************************************************
  // LOW between 1290-1640 us
  // *********************************************************************
  
  while (t < 1290 || t > 1640) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for a LOW waveform spike (high-LOW-high)

    if (DEBUG) { // For finding AGC timings
      if (t > 1000 && t < 2000) Serial.println(t);
    }
  }

  if (DEBUG) {
    Serial.print("AGC 2: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 290-390 us
  // 1 = 560-760 us
  // *********************************************************************

  while (i < 41) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Waits for a HIGH waveform spike (low-HIGH-low)
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 560 && t < 760) { // Found 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else if (t > 290 && t < 390) { // Found 0
      command += "0";
      if (DEBUG) Serial.println("0");
      
    } else if (t > 70 && t < 85) { // End spike, finish
      if (DEBUG) Serial.println("END FOUND");
      i = 0;
      break;
      
    } else { // Unrecognized bit, finish
      if (DEBUG) Serial.println("INVALID BIT");
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
    Serial.print("Bad capture, invalid command length ");
    Serial.println(command.length());
    if (ADDITIONAL) Serial.println("Invalid command: " + command);
    Serial.println();
    
  } else {
    Serial.println("Successful capture, command is: " + command);
    Serial.print("Remote control (unique) ID: ");
    Serial.println(convertBinaryStringToInt(command.substring(0, 17)), DEC);
    Serial.println("Channel: " + printChannel(command.substring(17, 21)));
    Serial.println("Command: " + printCommand(command.substring(21, 25)));
    Serial.println("Remote control model: " + printRemoteModel(command.substring(25, 33)));
    
    if (ADDITIONAL) {
      Serial.print("Checksum (CRC): ");
      Serial.println(convertBinaryStringToInt(command.substring(33, 41)), DEC);
    }
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printChannel(String channel) {

  int c = convertBinaryStringToInt(channel);

  switch (c) {

    case 0x7: // 0111
      return "1"; // ...or remote model BF-301

    case 0xB: // 1011
      return "2"; // ...or remote model BF-101

    case 0x3: // 0011
      return "3";

    case 0xD: // 1101
      return "4";

    case 0x5: // 0101
      return "5";

    case 0x0: // 0000
      return "ALL";

  }
  return "UNKNOWN/NEW";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printCommand(String command) {

  int c = convertBinaryStringToInt(command);

  switch (c) {
    
    case 0x7: // 0111
      return "DOWN";

    case 0xC: // 1100
      return "UP";

    case 0x5: // 0101
      return "STOP";

    case 0xD: // 1101
      return "PAIR/CONFIRM (C)";

    case 0xB: // 1011
      return "PROGRAM LIMITS (L)";

    case 0xE: // 1110
      return "CHANGE DIRECTION OF ROTATION (STOP + L)";

  }
  return "UNKNOWN";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printRemoteModel(String model) {

  int c = convertBinaryStringToInt(model);

  switch (c) {

    case 0x79: // 01111001
      return "BF-305 (5 channels)";

    case 0xFC: // 11111100
      return "BF-101 (single channel)";

    case 0x7C: // 01111100
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
