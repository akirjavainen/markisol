/*
******************************************************************************************************************************************************************
*
* Markisol iFit Spring Pro 433.92MHz window shades
* Also sold under the name Feelstyle and various Chinese brands like Bofu
* Compatible with remotes like BF-101, BF-301, BF-305, possibly others
* 
* Protocol code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* http://www.markisolgroup.com/en/products/ifit.html
* 
* Unless I'm completely mistaken, each remote has its unique, hard coded ID. I've included all commands from one remote as an
* example, but you can also use RemoteCapture.ino to capture your own remotes. The purpose of this project was to get my own
* window shades automated, so there's more work to be done should you wish to fully reverse engineer the codes and generate
* + add new "virtual remotes". Specifically, checksum calculation needs to be figured out.
* 
* 
* HOW TO USE
* 
* Capture your remote controls with RemoteCapture.ino and copy paste the 41 bit commands to Markisol.ino for sendMarkisolCommand().
* More info about this provided in RemoteCapture.ino.
* 
* 
* HOW TO USE WITH EXAMPLE COMMANDS
* 
* 1. Set the shade into pairing mode by holding down its red P button until it shakes twice ("TA-TA") or beeps.
* 2. Send the pairing command, eg. "sendMarkisolCommand(SHADE_PAIR_EXAMPLE);", which will shake the shade twice ("TA-TA") or beep.
* 3. Now you can control the shade, eg. sendMarkisolCommand(SHADE_DOWN_EXAMPLE); (or SHADE_UP_EXAMPLE, SHADE_STOP_EXAMPLE etc.).
* 
* Setting limits is quicker with the remotes, although you can use your Arduino for that as well. Some motors do not erase the
* the limits even if you reset them by holding down the red P button for 8-10 seconds.
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
* 4 bits for channel ID: 1 = 0111 (also used by BF-301), 2 = 1011 (also used by BF-101), 3 = 0011, 4 = 1101, 5 = 0101
* 4 bits for command: DOWN = 0111, UP = 1100, STOP = 0101, CONFIRM/PAIR = 1101, LIMITS = 1011, ROTATION DIRECTION = 1110
* 8 bits for remote control model: BF-305 multi = 01111001, BF-101 single = 11111100, BF-301 single = 01111100
* 8 bits for checksum (CRC): I have yet to figure out how this is formed
* = 41 bits in total
*
* There is a short HIGH spike of 80 us at the end, but it is not necessary to replicate.
* End with LOW radio silence of (minimum) 223 samples = 5057 us
* 
* 
* HOW THIS WAS STARTED
* 
* Commands were captured by a "poor man's oscillator": 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in.
* Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND).
*
* To view the waveform Arduino is transmitting (and debugging timings etc.), I found it easiest to connect the digital pin (13)
* from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was as clear as the original.
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

#define TRANSMIT_PIN          13      // We'll use digital 13 for transmitting
#define REPEAT_COMMAND        8       // How many times to repeat the same command: original remotes repeat 8 (multi) or 10 (single) times by default
#define DEBUG                 false   // Do note that if you add serial output during transmit, it will cause delay and commands may fail

// If you wish to use PORTB commands instead of digitalWrite, these are for Arduino Uno digital 13:
#define D13high | 0x20; 
#define D13low  & 0xDF; 

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
  // down its red P button until the "TA-TA"):
  //sendMarkisolCommand(SHADE_PAIR_EXAMPLE);
  //while (true) {} // Stop after pairing, you can use UP/STOP/DOWN commands afterwards
  // ---


  // Send DOWN/STOP/UP commands after pairing:
  //sendMarkisolCommand(SHADE_DOWN_EXAMPLE);
  delay(3000);
  
  //sendMarkisolCommand(SHADE_STOP_EXAMPLE);
  delay(3000);
  
  //sendMarkisolCommand(SHADE_UP_EXAMPLE);
  delay(3000);
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
    doMarkisolTribitSend(command_array, MARKISOL_COMMAND_BIT_ARRAY_SIZE, MARKISOL_AGC1_PULSE, MARKISOL_AGC2_PULSE, MARKISOL_RADIO_SILENCE, MARKISOL_PULSE_SHORT, MARKISOL_PULSE_LONG);
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // which will disrupt most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doMarkisolTribitSend(int *command_array, int command_array_size, int pulse_agc1, int pulse_agc2, int pulse_radio_silence, int pulse_short, int pulse_long) {

  if (command_array == NULL) {
    errorLog("doMarkisolTribitSend(): Array pointer was NULL, cannot continue.");
    return;
  }

  // Starting (AGC) bits:
  transmitWaveformHigh(pulse_agc1);
  transmitWaveformLow(pulse_agc2);

  // Transmit command:
  for (int i = 0; i < command_array_size; i++) {

      // If current bit is 0, transmit LOW-HIGH-LOW:
      if (command_array[i] == 0) {
        transmitWaveformLow(pulse_short);
        transmitWaveformHigh(pulse_short);
        transmitWaveformLow(pulse_short);
      }

      // If current bit is 1, transmit HIGH-HIGH-LOW:
      if (command_array[i] == 1) {
        transmitWaveformHigh(pulse_long);
        transmitWaveformLow(pulse_short);
      }   
   }

  // Radio silence at the end.
  // It's better to rather go a bit over than under required length.
  transmitWaveformLow(pulse_radio_silence);
  
  if (DEBUG) {
    Serial.println();
    Serial.print("Transmitted ");
    Serial.print(command_array_size);
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

      if (c == "0" || c == "1" || c == "2" || c == "3") { // 2 and 3 are allowed for other projects of mine (not used by this code)
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
