
// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   2

// Enable if there's no serial output or if you're modifying this code for another protocol/device:
#define DEBUG         false   


void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
}


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

    if (t > 728 && t < 760) { // Long spikes signal 1
      command += "1";
      if (DEBUG) Serial.println("1");
      
    } else if (t > 330 && t < 360) { // Short spikes signal 0
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
    Serial.println("Bad capture, invalid command length.");
  } else {
    Serial.print("Successful capture, command is: ");
    Serial.println(command);
  }
}
