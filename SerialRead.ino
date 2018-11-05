/*
******************************************************************************************************************************************************************
*
* Arduino serial read example
* Receiving commands from serial, e.g. connecting Raspberry Pi or
* other systems with Arduino
*
* HOW TO USE
* Create a bash script called "433trigger.sh" to simplify use:


#!/bin/bash
export DEVICE=/dev/ttyACM0 # Could also be /dev/ttyUSB*

if [ ! -n "$1" ]; then echo "Usage: $0 [command]"; exit; fi

echo "<PREPARE>" >$DEVICE # Send some extra data to make sure serial reacts
echo "<$1>" >$DEVICE


* Then trigger with "433trigger.sh MY_OWN_COMMAND".
* Add your commands and actions to show_new_data_received().
*
* NOTE: If your Arduino shows up as ttyUSB* (eg. ttyUSB0), you may need to
* "cat /dev/ttyUSB0" on another terminal for the serial interface to react.
*
******************************************************************************************************************************************************************
*/



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
const byte char_count = 32;

char received_data[char_count];
boolean new_data_received = false;
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  receive_new_data();
  show_new_data_received(); // Required here and in receive_new_data()
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void show_new_data_received() {
  
    if (new_data_received) {
      new_data_received = false;

      Serial.println(received_data);

      // *********************************************************************
      // Add your own commands here and what to do with them (without the headers <>):
      // *********************************************************************
      if (char_array_contains(received_data, "MY_OWN_COMMAND_1")) {
        Serial.println("My own command received.");
      }
      // *********************************************************************
      if (char_array_contains(received_data, "MY_OWN_COMMAND_2")) {
        Serial.println("My own command received.");
      }
      // *********************************************************************
    } 
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void receive_new_data() {
    char marker_header = '<';
    char marker_finish = '>';
    char current_char;
    static byte b = 0;
    static boolean receive_in_progress = false;
 
     while (Serial.available() > 0 && new_data_received == false) {
        current_char = Serial.read();

        if (receive_in_progress == true) {
            if (current_char != marker_finish) {
                received_data[b] = current_char;
                b++;

                if (b >= char_count) b = char_count - 1;

            } else {
                received_data[b] = '\0'; // String termination
                receive_in_progress = false;
                b = 0;
                new_data_received = true;
                show_new_data_received(); // Required here and in loop()
            }

        } else if (current_char == marker_header) {
            receive_in_progress = true;
        }
    }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
boolean char_array_contains(char *str, char *str_find) {
  char i = 0;
  char found = 0;
  char length = strlen(str);
    
  if (strlen(str_find) > length) return false;

  while (i < length) {
    if (str[i] == str_find[found]) {
      found++;       
      if (strlen(str_find) == found) return true;

    } else {
      found = 0;
    }

    i++;
  }
  return false;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
