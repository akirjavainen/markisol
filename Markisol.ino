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
* window shades automated, so there's a bit more work to be done, to reverse engineer the final 8 bits of the commands.
* I don't know how they're formed, but the motors just seem to ignore them anyway. You could zero them out (as well as
* the previous 8 bits for the remote model) and command the shades with 25 bit commands. If you only have single channel
* remotes (BF-301 for example), you can zero out the 4 bits for channel as well. In fact, only the 17 bit remote ID and
* the 4 bit command are actually required in most cases.
* 
* 
* HOW TO USE
* 
* Capture your remote controls with RemoteCapture.ino and copy paste the 41 bit commands to Markisol.ino for sendMarkisolCommand().
* More info about this provided in RemoteCapture.ino.
* 
* If you have no multichannel remotes like the BF-305, you can also call the sendShortMarkisolCommand() function with
* your 17 bit remote ID and a command (see setup() below for more info).
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
* A single command is: 2 AGC bits + 41 command tribits + radio silence
* Swapping HIGH and LOW does not work with these shades.
*
* All sample counts below listed with a sample rate of 44100 Hz (sample count / 44100 = microseconds).
*
* Starting (AGC) bits:
* HIGH of approx. 107 samples = 2426 us
* LOW of approx. 59 samples = 1338 us
* 
* Pulse length:
* SHORT: approx. 14 samples = 317 us
* LONG: approx. 30 samples = 680 us
* 
* Data bits:
* Data 0 = short LOW, short HIGH, short LOW (wire 010)
* Data 1 = long HIGH, short LOW (wire 110)
* 
* Command is as follows:
* 17 bits for (unique) remote control ID, hard coded in remotes
* 4 bits for channel ID: 1 = 0111 (also used by BF-301), 2 = 1011 (also used by BF-101), 3 = 0011, 4 = 1101, 5 = 0101, ALL = 0000
* 4 bits for command: DOWN = 0111, UP = 1100, STOP = 0101, CONFIRM/PAIR = 1101, LIMITS = 1011, ROTATION DIRECTION = 1110
* 8 bits for remote control model: BF-305 multi = 01111001, BF-101 single = 11111100, BF-301 single = 01111100
* 8 bits for something? I have yet to figure out how this is formed, but no motor seems to care if I simply zero them out
* = 41 bits in total
*
* There is a short HIGH spike of 80 us at the end, but it is not necessary to replicate.
* End with LOW radio silence of (minimum) 223 samples = 5057 us
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
* Note that the waveform captured by Audacity is basically "upside down". This code was written without using Audacity's Effects -> Invert
* (LOW/HIGH are inverted in the functions below).
* 
******************************************************************************************************************************************************************
*/



// Example commands to try (or just capture your own remotes with RemoteCapture.ino):
#define SHADE_PAIR_EXAMPLE                 "01010001110101100011111010111110000100000" // C button
#define SHADE_DOWN_EXAMPLE                 "01010001110101100011101110111110000101011" // DOWN button
#define SHADE_STOP_EXAMPLE                 "01010001110101100011101010111110000101000" // STOP button
#define SHADE_UP_EXAMPLE                   "01010001110101100011111000111110000100001" // UP button
#define SHADE_LIMIT_EXAMPLE                "01010001110101100011110110111110000100111" // L button
#define SHADE_CHANGE_DIRECTION_EXAMPLE     "01010001110101100011111100111110000100010" // STOP + L buttons

#define TRANSMIT_PIN                       13   // We'll use digital 13 for transmitting
#define REPEAT_COMMAND                      8   // How many times to repeat the same command: original remotes repeat 8 (multi) or 10 (single) times by default
#define DEBUG                           false   // Do note that if you add serial output during transmit, it will cause delay and commands may fail

// If you wish to use PORTB commands instead of digitalWrite, these are for Arduino Uno digital 13:
#define D13high | 0x20; 
#define D13low  & 0xDF; 

// For sendShortMarkisolCommand():
//#define MY_REMOTE_ID_1                    "00000000000000000"   // Enter your 17 bit remote ID here or make up a binary number and confirm/pair it with the motor
#define DEFAULT_CHANNEL                     "0000"                // Channel information is only required for multichannel remotes like the BF-305
#define DEFAULT_REMOTE_MODEL                "01111100"            // We default to BF-301, but this is actually ignored by motors and could be plain zeroes
#define DEFAULT_TRAILING_BITS               "00000000"            // Last 8 bits of the command. What do they mean? No idea. Again, ignored by motors.
#define COMMAND_DOWN                        "0111"                // Remote button DOWN
#define COMMAND_UP                          "1100"                // Remote button UP
#define COMMAND_STOP                        "0101"                // Remote button STOP
#define COMMAND_PAIR                        "1101"                // Remote button C
#define COMMAND_PROGRAM_LIMITS              "1011"                // Remote button L
#define COMMAND_CHANGE_ROTATION_DIRECTION   "1110"                // Remote buttons STOP + L

// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define MARKISOL_AGC1_PULSE                   2410  // 107 samples
#define MARKISOL_AGC2_PULSE                   1320  // 59 samples
#define MARKISOL_RADIO_SILENCE                5045  // 223 samples

#define MARKISOL_PULSE_SHORT                  300   // 13-16 samples
#define MARKISOL_PULSE_LONG                   680   // 26-32 samples

#define MARKISOL_COMMAND_BIT_ARRAY_SIZE       41    // Command bit count



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
  // pairing mode and transmit COMMAND_PAIR first:

  //sendShortMarkisolCommand(MY_REMOTE_ID_1, COMMAND_PAIR);
  //while (true) {} // Stop after pairing
  // ---
  
  //sendShortMarkisolCommand(MY_REMOTE_ID_1, COMMAND_DOWN);
  //delay(3000);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendMarkisolCommand(String command) {
  
  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  if (command.length() < MARKISOL_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendMarkisolCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (command.length() > MARKISOL_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendMarkisolCommand(): Invalid command (too long), cannot continue.");
    return;
  }

  // Declare the array (int) of command bits
  int command_array[MARKISOL_COMMAND_BIT_ARRAY_SIZE];

  // Processing a string during transmit is just too slow,
  // let's convert it to an array of int first:
  convertStringToArrayOfInt(command, command_array, MARKISOL_COMMAND_BIT_ARRAY_SIZE);
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doMarkisolTribitSend(command_array);
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // disrupting most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendShortMarkisolCommand(String remote_id, String command) {
  
  if (remote_id.length() != 17) {
    errorLog("sendShortMarkisolCommand(): Correct remote ID length is 17 bits. Cannot continue.");
    return;
  }
  if (command.length() != 4) {
    errorLog("sendShortMarkisolCommand(): Correct command length is 4 bits. Cannot continue.");
    return;
  }

  // Let's form and transmit the full command:
  sendMarkisolCommand(remote_id + DEFAULT_CHANNEL + command + DEFAULT_REMOTE_MODEL + DEFAULT_TRAILING_BITS);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doMarkisolTribitSend(int *command_array) {

  if (command_array == NULL) {
    errorLog("doMarkisolTribitSend(): Array pointer was NULL, cannot continue.");
    return;
  }

  // Starting (AGC) bits:
  transmitWaveformHigh(MARKISOL_AGC1_PULSE);
  transmitWaveformLow(MARKISOL_AGC2_PULSE);

  // Transmit command:
  for (int i = 0; i < MARKISOL_COMMAND_BIT_ARRAY_SIZE; i++) {

      // If current bit is 0, transmit LOW-HIGH-LOW:
      if (command_array[i] == 0) {
        transmitWaveformLow(MARKISOL_PULSE_SHORT);
        transmitWaveformHigh(MARKISOL_PULSE_SHORT);
        transmitWaveformLow(MARKISOL_PULSE_SHORT);
      }

      // If current bit is 1, transmit HIGH-HIGH-LOW:
      if (command_array[i] == 1) {
        transmitWaveformHigh(MARKISOL_PULSE_LONG);
        transmitWaveformLow(MARKISOL_PULSE_SHORT);
      }   
   }

  // Radio silence at the end.
  // It's better to go a bit over than under minimum required length:
  transmitWaveformLow(MARKISOL_RADIO_SILENCE);
  
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
void transmitWaveformHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW); // Digital pin low transmits a high waveform
  //PORTB = PORTB D13low; // If you wish to use faster PORTB commands instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitWaveformLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH); // Digital pin high transmits a low waveform
  //PORTB = PORTB D13high; // If you wish to use faster PORTB commands instead
  delayMicroseconds(delay_microseconds);
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

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void convertStringToArrayOfInt(String command, int *int_array, int command_array_size) {
  String c = "";

  if (int_array == NULL) {
    errorLog("convertStringToArrayOfInt(): Array pointer was NULL, cannot continue.");
    return;
  }
 
  for (int i = 0; i < command_array_size; i++) {
      c = command.substring(i, i + 1);

      if (c == "0" || c == "1") {
        int_array[i] = convertStringToInt(c);
      } else {
        errorLog("convertStringToArrayOfInt(): Invalid character " + c + " in command.");
        return;
      }
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
