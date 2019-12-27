/*
******************************************************************************************************************************************************************
*
* Markisol iFit Spring Pro 433.92MHz window shades
* Also sold under the name Feelstyle and various Chinese brands like Bofu
* Compatible with remotes like BF-101, BF-301, BF-305, possibly others
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* http://www.markisolgroup.com/en/products/ifit.html
* 
* Unless I'm completely mistaken, each remote has its unique, hard coded ID. I've included all commands from one as an
* example, but you can use RemoteCapture.ino to capture your own remotes. The purpose of this project was to get my own
* window shades automated, so there's a bit more work to be done, to reverse engineer the final 9 trailing bits of the
* commands. I don't (yet) know how they're formed, but most motors simply seem to ignore them. However, some require
* them to be correct and sendShortMarkisolCommand() will not work with such motors (use the full 41 bit command with
* sendMarkisolCommand() in that case).
* 
* Special thanks to Chris Teel for testing timings with some Rollerhouse products to achieve better compatibility.
* 
* 
* HOW TO USE
* 
* Capture your remote controls with RemoteCapture.ino and copy paste the 41 bit commands to Markisol.ino for sendMarkisolCommand().
* More info about this provided in RemoteCapture.ino.
* 
* If you have no multichannel remotes like the BF-305, you can also try the sendShortMarkisolCommand() function with
* your 16 bit remote ID and a command (see setup() below for more info). This function will not work with every motor,
* so first try the full 41 bit commands.
* 
* 
* HOW TO USE WITH EXAMPLE COMMANDS
* 
* 1. Set the shade into pairing mode by holding down its P/SETTING button until it shakes twice ("TA-TA") or beeps.
* 2. Send the pairing command, eg. "sendMarkisolCommand(SHADE_PAIR_EXAMPLE);", which will shake the shade twice ("TA-TA") or beep.
* 3. Now you can control the shade, eg. sendMarkisolCommand(SHADE_DOWN_EXAMPLE); (or SHADE_UP_EXAMPLE, SHADE_STOP_EXAMPLE etc.).
* 
* Setting limits is quicker with the remotes, although you can use your Arduino for that as well. Some motors do not erase the
* the limits even if you reset them by holding down the P/SETTING button for 8-10 seconds. They just reset the list of registered
* remote controls.
* 
* 
* PROTOCOL DESCRIPTION
* 
* Tri-state bits are used.
* A single command is: 3 AGC bits + 41 command tribits + radio silence
*
* All sample counts below listed with a sample rate of 44100 Hz (sample count / 44100 = microseconds).
*
* Starting (AGC) bits:
* HIGH of approx. 216 samples = 4885 us
* LOW of approx. 108 samples = 2450 us
* HIGH of approx. 75 samples = 1700 us
* 
* Pulse length:
* SHORT: approx. 15 samples = 340 us
* LONG: approx. 30 samples = 680 us
* 
* Data bits:
* Data 0 = short LOW, short HIGH, short LOW (wire 010)
* Data 1 = short LOW, long HIGH (wire 011)
* 
* Command is as follows:
* 16 bits for (unique) remote control ID, hard coded in remotes
* 4 bits for channel ID: 1 = 1000 (also used by BF-301), 2 = 0100 (also used by BF-101), 3 = 1100, 4 = 0010, 5 = 1010, ALL = 1111
* 4 bits for command: DOWN = 1000, UP = 0011, STOP = 1010, CONFIRM/PAIR = 0010, LIMITS = 0100, ROTATION DIRECTION = 0001
* 8 bits for remote control model: BF-305 multi = 10000110, BF-101 single = 00000011, BF-301 single = 10000011
* 9 bits for something? I have yet to figure out how this is formed, but most motors simply do not seem to care (others do)
* 
* = 41 bits in total
*
* There is a short LOW drop of 80 us at the end of each command, before the next AGC (or radio silence at the end of last command).
* End the last command in sequence with LOW radio silence of 223 samples = 5057 us
* 
* 
* HOW THIS WAS STARTED
* 
* Project started with a "poor man's oscilloscope": 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in.
* Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND).
*
* To view the waveform Arduino is transmitting (and debugging timings etc.), I found it easiest to directly connect the digital pin (13)
* from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was very clear.
* 
* Note that a PC sound cards may capture the waveform "upside down" (phase inverted). You may need to apply Audacity's Effects -> Invert
* to get the HIGHs and LOWs correctly.
* 
******************************************************************************************************************************************************************
*/



// Example commands to try (or just capture your own remotes with RemoteCapture.ino):
#define SHADE_PAIR_EXAMPLE                 "10111011111011111000001010000011110101001" // C button
#define SHADE_DOWN_EXAMPLE                 "10111011111011111000100010000011110110101" // DOWN button
#define SHADE_STOP_EXAMPLE                 "10111011111011111000101010000011110110001" // STOP button
#define SHADE_UP_EXAMPLE                   "10111011111011111000001110000011110101011" // UP button
#define SHADE_LIMIT_EXAMPLE                "10111011111011111000010010000011110100101" // L button
#define SHADE_CHANGE_DIRECTION_EXAMPLE     "10111011111011111000000110000011110101111" // STOP + L buttons

#define TRANSMIT_PIN                       13   // We'll use digital 13 for transmitting
#define REPEAT_COMMAND                      8   // How many times to repeat the same command: original remotes repeat 8 (multi) or 10 (single) times by default
#define DEBUG                           false   // Do note that if you print serial output during transmit, it will cause delay and commands may fail

// If you wish to use PORTB commands instead of digitalWrite, these are for Arduino Uno digital 13:
#define D13high | 0x20; 
#define D13low  & 0xDF; 

// For sendShortMarkisolCommand():
//#define MY_REMOTE_ID_1                    "0000000000000000"    // Enter your 16 bit remote ID here or make up a binary number and confirm/pair it with the motor
#define DEFAULT_CHANNEL                     "1000"                // Channel information is only required for multichannel remotes like the BF-305
#define DEFAULT_REMOTE_MODEL                "10000011"            // We default to BF-301, but this is actually ignored by motors and could be plain zeroes
#define DEFAULT_TRAILING_BITS               "000000000"           // Last 9 bits of the command. What do they mean? No idea. Again, ignored by motors.
#define COMMAND_DOWN                        "1000"                // Remote button DOWN
#define COMMAND_UP                          "0011"                // Remote button UP
#define COMMAND_STOP                        "1010"                // Remote button STOP
#define COMMAND_PAIR                        "0010"                // Remote button C
#define COMMAND_PROGRAM_LIMITS              "0100"                // Remote button L
#define COMMAND_CHANGE_ROTATION_DIRECTION   "0001"                // Remote buttons STOP + L

// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define MARKISOL_AGC1_PULSE                   4885  // 216 samples
#define MARKISOL_AGC2_PULSE                   2450  // 108 samples
#define MARKISOL_AGC3_PULSE                   1700  // 75 samples
#define MARKISOL_RADIO_SILENCE                5057  // 223 samples

#define MARKISOL_PULSE_SHORT                  340   // 15 samples
#define MARKISOL_PULSE_LONG                   680   // 30 samples

#define MARKISOL_COMMAND_BIT_ARRAY_SIZE       41    // Command bit count


// NOTE: If you're having issues getting the motors to respond, try these previous defaults:
//#define MARKISOL_AGC2_PULSE                   2410  // 107 samples
//#define MARKISOL_AGC3_PULSE                   1320  // 59 samples
//#define MARKISOL_PULSE_SHORT                  300   // 13 samples



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  Serial.begin(9600); // Used for error messages even with DEBUG set to false
  if (DEBUG) Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {

  // Pair a shade (first set the shade to pairing mode by holding
  // down its P/SETTING button until the "TA-TA" or a beep):
  //sendMarkisolCommand(SHADE_PAIR_EXAMPLE);
 // sendMarkisolCommand("01100110111010100100001110000110100001001");
  //while (true) {} // Stop after pairing, you can use UP/STOP/DOWN commands afterwards
  // ---


  // Send DOWN/STOP/UP commands after pairing:
  //sendMarkisolCommand(SHADE_DOWN_EXAMPLE);
  //delay(3000);
  
  //sendMarkisolCommand(SHADE_STOP_EXAMPLE);
  //delay(3000);
  
  //sendMarkisolCommand(SHADE_UP_EXAMPLE);
  //delay(3000);


  // --- Short commands ---
  // If your remote is single channel (like the BF-301), you only need the remote ID.
  // You can make up your own "virtual" ID, too. In that case, just set the motor to
  // pairing mode and transmit COMMAND_PAIR first.

  // Do note that short commands may not work with every motor. The full 41 bit
  // binary command has bits and pieces that some models require, others don't.

  //sendShortMarkisolCommand(MY_REMOTE_ID_1, COMMAND_PAIR);
  //while (true) {} // Stop after pairing
  // ---
  
  //sendShortMarkisolCommand(MY_REMOTE_ID_1, COMMAND_DOWN);
  delay(3000);

}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendMarkisolCommand(char* command) {

  if (command == NULL) {
    errorLog("sendMarkisolCommand(): Command array pointer was NULL, cannot continue.");
    return;
  }

  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  if (strlen(command) < MARKISOL_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendMarkisolCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (strlen(command) > MARKISOL_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendMarkisolCommand(): Invalid command (too long), cannot continue.");
    return;
  }
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doMarkisolTribitSend(command);
  }

  // Radio silence at the end of last command.
  // It's better to go a bit over than under minimum required length:
  transmitLow(MARKISOL_RADIO_SILENCE);


  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // disrupting most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendShortMarkisolCommand(char* remote_id, char* command) {

  if (strlen(remote_id) != 16) {
    errorLog("sendShortMarkisolCommand(): Correct remote ID length is 16 bits. Cannot continue.");
    return;
  }
  if (strlen(command) != 4) {
    errorLog("sendShortMarkisolCommand(): Correct command length is 4 bits. Cannot continue.");
    return;
  }

  // Let's form and transmit the full command:
  char* full_command = new char[MARKISOL_COMMAND_BIT_ARRAY_SIZE];

  full_command[0] = '\0';
  strcat(full_command, remote_id);
  strcat(full_command, DEFAULT_CHANNEL);
  strcat(full_command, command);
  strcat(full_command, DEFAULT_REMOTE_MODEL);
  strcat(full_command, DEFAULT_TRAILING_BITS);

  sendMarkisolCommand(full_command);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doMarkisolTribitSend(char* command) {

  // Starting (AGC) bits:
  transmitHigh(MARKISOL_AGC1_PULSE);
  transmitLow(MARKISOL_AGC2_PULSE);
  transmitHigh(MARKISOL_AGC3_PULSE);

  // Transmit command:
  for (int i = 0; i < MARKISOL_COMMAND_BIT_ARRAY_SIZE; i++) {

      // If current bit is 0, transmit LOW-HIGH-LOW (010):
      if (command[i] == '0') {
        transmitLow(MARKISOL_PULSE_SHORT);
        transmitHigh(MARKISOL_PULSE_SHORT);
        transmitLow(MARKISOL_PULSE_SHORT);
      }

      // If current bit is 1, transmit LOW-HIGH-HIGH (011):
      if (command[i] == '1') {
        transmitLow(MARKISOL_PULSE_SHORT);
        transmitHigh(MARKISOL_PULSE_LONG);
      }   
   }

  if (DEBUG) {
    Serial.println();
    Serial.print("Transmitted ");
    Serial.print(MARKISOL_COMMAND_BIT_ARRAY_SIZE);
    Serial.println(" bits.");
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH);
  //PORTB = PORTB D13high; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW);
  //PORTB = PORTB D13low; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
