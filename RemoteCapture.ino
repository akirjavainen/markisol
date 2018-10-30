
// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   2

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG         false
#define ADDITIONAL    false    // Display some additional information after capture


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


  // As a quick hack, add the leading 0 here,
  // as the way I wrote the code is missing it.
  // Every Markisol protocol command starts with it,
  // so function-wise it's not a big deal:
  String command = "0";


  // *********************************************************************
  // Wait for first AGC bit:
  // *********************************************************************
  
  while (t < 2489 || t > 2495) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Finds HIGH waveform spots
  }

  if (DEBUG) {
    Serial.print("AGC 1: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Wait for second AGC bit:
  // *********************************************************************
  
  while (t < 1574 || t > 1590) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Finds LOW waveform spots
  }

  if (DEBUG) {
    Serial.print("AGC 2: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************  
  // 0 = 330-360 us
  // 1 = 728-760 us
  // *********************************************************************

  while (i < 41) {
    t = pulseIn(RECEIVE_PIN, LOW, 1000000); // Reads HIGH waveform spikes
    
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }

    if (t > 560 && t < 760) { // Long spikes signal 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else if (t > 290 && t < 390) { // Short spikes signal 0
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

  // Correct command length is 41 bits, dismiss bad captures:
  if (command.length() < 41 || command.length() > 41) {
    Serial.print("Bad capture, invalid command length ");
    Serial.println(command.length());
    if (ADDITIONAL) Serial.println("Invalid command: " + command);
    Serial.println();
    
  } else {
    Serial.println("Successful capture, command is: " + command);
    Serial.println("Remote control (unique) ID: " + command.substring(0, 17));
    Serial.println("Channel: " + printChannel(command.substring(17, 21)));
    Serial.println("Command: " + printCommand(command.substring(21, 25)));
    Serial.println("Remote control model: " + printRemoteModel(command.substring(25, 33)));
    if (ADDITIONAL) Serial.println("Checksum (CRC): " + command.substring(33, 41));
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printChannel(String channel) {

  if (channel == "0111") {
      return "1"; // ...or remote model BF-301

  } else if (channel == "1011") {
      return "2"; // ...or remote model BF-101

  } else if (channel == "0011") {
      return "3";

  } else if (channel == "1101") {
      return "4";

  } else if (channel == "0101") {
      return "5";
  }

  return "UNKNOWN";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printCommand(String command) {

  if (command == "0111") {
      return "DOWN";

  } else if (command == "1100") {
      return "UP";

  } else if (command == "0101") {
      return "STOP";

  } else if (command == "1101") {
      return "PAIR/CONFIRM (C)";

  } else if (command == "1011") {
      return "PROGRAM LIMITS (L)";

  } else if (command == "1110") {
      return "CHANGE DIRECTION OF ROTATION (STOP + L)";
  }

  return "UNKNOWN";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
String printRemoteModel(String model) {

  if (model == "01111001") {
      return "BF-305 (5 channels)";
  
  } else if (model == "11111100") {
      return "BF-101 (single channel)";
      
  } else if (model == "01111100") {
      return "BF-301 (single channel)";
  }

  return "UNKNOWN/NEW";
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
