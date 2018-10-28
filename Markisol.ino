/*
******************************************************************************************************************************************************************
*
* Markisol iFit Spring Pro 433.92MHz window shades (also sold under the name Feelstyle)
* Control code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* http://www.markisolgroup.com/en/products/ifit.html
* 
* Unless I'm completely mistaken, each remote has its unique (or nearly unique) ID. However, I've included the pairing commands
* from my remotes, so you can simply pair them to your shades and take advantage of them directly. The purpose of this project was
* to get my own window shades automated, so there's more work to be done should you wish to fully reverse engineer the codes and
* generate + add new "virtual remotes". Specifically, checksum calculation needs to be figured out.
* 
* USAGE:
* 1. Set the shade into pairing mode by holding down its red P button until it shakes twice ("TA-TA") or beeps.
* 2. Send the pairing command from this code, eg. "sendMarkisolCommand(SHADE_PAIR_1);", which will shake the shade twice ("TA-TA") or beep.
* 3. Control the shade with that channel's codes, eg. sendMarkisolCommand(SHADE_DOWN_1); (or SHADE_UP_1, SHADE_STOP_1 etc.).
* 
* Setting limits is quicker and easier with the original remotes. Typically, limits are not lost even if you reset the shade
* by holding down its red P button for 8-10 seconds.
* 
* Commands were captured by a "poor man's oscillator": 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in.
* Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND).
* My translation of the waveform is tri-state encoding:
* 
* 0 = short LOW, short HIGH, short LOW (010)
* 1 = long HIGH, short LOW (110)
* 
* To view the waveform Arduino is transmitting (and debugging timings etc.), I found it easiest to connect the digital pin (13)
* from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was as clear as the original.
* 
* This protocol and remotes like BF-305 and BF-101 are used by many different vendors and products. One such brand is Bofu.
* 
* 
* Tri-state encoding is used.
* A single command is: 2 AGC bits + 41 command tribits + radio silence
* Swapping HIGH and LOW does not work with these shades.
*
* Starting (AGC) bits:
* HIGH of 107 samples / sample rate = 2426 microseconds - lag compensation
* LOW of 59 samples / sample rate = 1338 microseconds - lag compensation
* 
* Pulse length:
* SHORT: 14 samples / sample rate = 317 microseconds - lag compensation
* 
* Checksum (CRC):
* I haven't been able to figure out (yet) how checksum at the end is calculated.
*
*
* Remote 1 multi (model BF-305), controlling 5 different shades:
*
* REMOTE CONTROL      CHANNEL    COMMAND       CHECKSUM                       DECIMALS, ATTEMPT TO FIGURE OUT HOW CHECKSUM IS CALCULATED
* 01001100100010101 -- 0111 -- 011101111001 -- 10110100   = Shade 1 DOWN      39189 --  7 -- 1913 -- 180      FULL WITHOUT CRC: 2568320889
* 01001100100010101 -- 1011 -- 011101111001 -- 01110100   = Shade 2 DOWN      39189 -- 11 -- 1913 -- 116
* 01001100100010101 -- 0011 -- 011101111001 -- 11110100   = Shade 3 DOWN      39189 --  3 -- 1913 -- 244
* 01001100100010101 -- 1101 -- 011101111001 -- 00001100   = Shade 4 DOWN      39189 -- 13 -- 1913 -- 12
* 01001100100010101 -- 0101 -- 011101111001 -- 10001100   = Shade 5 DOWN      39189 --  5 -- 1913 -- 140
*
* 01001100100010101 -- 0111 -- 110001111001 -- 10111011   = Shade 1 UP
* 01001100100010101 -- 1011 -- 110001111001 -- 01111011   = Shade 2 UP
* 01001100100010101 -- 0011 -- 110001111001 -- 11111011   = Shade 3 UP
* 01001100100010101 -- 1101 -- 110001111001 -- 00000111   = Shade 4 UP
* 01001100100010101 -- 0101 -- 110001111001 -- 10000111   = Shade 5 UP
*
* 01001100100010101 -- 0111 -- 010101111001 -- 10110110   = Shade 1 STOP
* 01001100100010101 -- 1011 -- 010101111001 -- 01110110   = Shade 2 STOP
* 01001100100010101 -- 0011 -- 010101111001 -- 11110110   = Shade 3 STOP
* 01001100100010101 -- 1101 -- 010101111001 -- 00001110   = Shade 4 STOP
* 01001100100010101 -- 0101 -- 010101111001 -- 10001110   = Shade 5 STOP
*
* 01001100100010101 -- 0000 -- 011101111001 -- 11011100   = All shades DOWN
* 01001100100010101 -- 0000 -- 110001111001 -- 11010111   = All shades UP
* 01001100100010101 -- 0000 -- 010101111001 -- 11011110   = All shades STOP
* 
* 01001100100010101 -- 0111 -- 110101111001 -- 10111010   = Pair channel 1
* 01001100100010101 -- 1011 -- 110101111001 -- 01111010   = Pair channel 2
* 01001100100010101 -- 0011 -- 110101111001 -- 11111010   = Pair channel 3
* 01001100100010101 -- 1101 -- 110101111001 -- 00000110   = Pair channel 4
* 01001100100010101 -- 0101 -- 110101111001 -- 10000110   = Pair channel 5
* 
* 01001100100010101 -- 0111 -- 101101111001 -- 10111100   = Set limits channel 1 
* 01001100100010101 -- 1011 -- 101101111001 -- 01111100   = Set limits channel 2
* 01001100100010101 -- 0011 -- 101101111001 -- 11111100   = Set limits channel 3
* 01001100100010101 -- 1101 -- 101101111001 -- 00000010   = Set limits channel 4
* 01001100100010101 -- 0101 -- 101101111001 -- 10000010   = Set limits channel 5
* 
* 01001100100010101 -- 0111 -- 111001111001 -- 10111001   = Change rotation direction 1
* 01001100100010101 -- 1011 -- 111001111001 -- 01111001   = Change rotation direction 2
* 01001100100010101 -- 0011 -- 111001111001 -- 11111001   = Change rotation direction 3
* 01001100100010101 -- 1101 -- 111001111001 -- 00000101   = Change rotation direction 4
* 01001100100010101 -- 0101 -- 111001111001 -- 10000101   = Change rotation direction 5
* 
* 
* Remote 2 (model BF-101), single UP/STOP/DOWN, programmed for shades 1, 2 and 3 downstairs:
* 00100000000111011 -- 1011 -- 011111111100 -- 00001111   = Shades DOWN
* 00100000000111011 -- 1011 -- 110011111100 -- 00000101   = Shades UP
* 00100000000111011 -- 1011 -- 010111111100 -- 00001100   = Shades STOP
* 00100000000111011 -- 1011 -- 110111111100 -- 00000100   = Pair
* 00100000000111011 -- 1011 -- 101111111100 -- 00000000   = Set limits          16443 -- 11 -- 3068 -- 0      FULL WITHOUT CRC: 1077656572
* 00100000000111011 -- 1011 -- 111011111100 -- 00000110   = Change rotation direction
* 
*
* Remote 3 (model BF-101), single UP/STOP/DOWN, programmed for shades 4 and 5 upstairs:
* 00101011001101011 -- 1011 -- 011111111100 -- 01110001   = Shades DOWN
* 00101011001101011 -- 1011 -- 110011111100 -- 01111100   = Shades UP
* 00101011001101011 -- 1011 -- 010111111100 -- 01110011   = Shades STOP
* 00101011001101011 -- 1011 -- 110111111100 -- 01111101   = Pair
* 00101011001101011 -- 1011 -- 101111111100 -- 01111001   = Set limits
* 00101011001101011 -- 1011 -- 111011111100 -- 01111111   = Change rotation direction
*
*
* End with LOW radio silence of 223 samples / sample rate = 5057 microseconds
* 
******************************************************************************************************************************************************************
*/


#define SHADE_DOWN_1              "01001100100010101011101110111100110110100"
#define SHADE_DOWN_2              "01001100100010101101101110111100101110100"
#define SHADE_DOWN_3              "01001100100010101001101110111100111110100"
#define SHADE_DOWN_4              "01001100100010101110101110111100100001100"
#define SHADE_DOWN_5              "01001100100010101010101110111100110001100"

#define SHADE_DOWN_DOWNSTAIRS     "00100000000111011101101111111110000001111"
#define SHADE_DOWN_UPSTAIRS       "00101011001101011101101111111110001110001"
#define SHADE_DOWN_ALL            "01001100100010101000001110111100111011100"

#define SHADE_UP_1                "01001100100010101011111000111100110111011"
#define SHADE_UP_2                "01001100100010101101111000111100101111011"
#define SHADE_UP_3                "01001100100010101001111000111100111111011"
#define SHADE_UP_4                "01001100100010101110111000111100100000111"
#define SHADE_UP_5                "01001100100010101010111000111100110000111"

#define SHADE_UP_DOWNSTAIRS       "00100000000111011101111001111110000000101"
#define SHADE_UP_UPSTAIRS         "00101011001101011101111001111110001111100"
#define SHADE_UP_ALL              "01001100100010101000011000111100111010111"

#define SHADE_STOP_1              "01001100100010101011101010111100110110110"
#define SHADE_STOP_2              "01001100100010101101101010111100101110110"
#define SHADE_STOP_3              "01001100100010101001101010111100111110110"
#define SHADE_STOP_4              "01001100100010101110101010111100100001110"
#define SHADE_STOP_5              "01001100100010101010101010111100110001110"

#define SHADE_STOP_DOWNSTAIRS     "00100000000111011101101011111110000001100"
#define SHADE_STOP_UPSTAIRS       "00101011001101011101101011111110001110011"
#define SHADE_STOP_ALL            "01001100100010101000001010111100111011110"

#define SHADE_PAIR_1              "01001100100010101011111010111100110111010"
#define SHADE_PAIR_2              "01001100100010101101111010111100101111010"
#define SHADE_PAIR_3              "01001100100010101001111010111100111111010"
#define SHADE_PAIR_4              "01001100100010101110111010111100100000110"
#define SHADE_PAIR_5              "01001100100010101010111010111100110000110"

#define SHADE_PAIR_DOWNSTAIRS     "00100000000111011101111011111110000000100"
#define SHADE_PAIR_UPSTAIRS       "00101011001101011101111011111110001111101"

#define SHADE_LIMIT_1             "01001100100010101011110110111100110111100"
#define SHADE_LIMIT_2             "01001100100010101101110110111100101111100"
#define SHADE_LIMIT 3             "01001100100010101001110110111100111111100"
#define SHADE_LIMIT_4             "01001100100010101110110110111100100000010"
#define SHADE_LIMIT_5             "01001100100010101010110110111100110000010"

#define SHADE_LIMIT_DOWNSTAIRS    "00100000000111011101110111111110000000000"
#define SHADE_LIMIT_UPSTAIRS      "00101011001101011101110111111110001111001"

#define SHADE_CHANGE_DIRECTION_1  "01001100100010101011111100111100110111001"
#define SHADE_CHANGE_DIRECTION_2  "01001100100010101101111100111100101111001"
#define SHADE_CHANGE_DIRECTION_3  "01001100100010101001111100111100111111001"
#define SHADE_CHANGE_DIRECTION_4  "01001100100010101110111100111100100000101"
#define SHADE_CHANGE_DIRECTION_5  "01001100100010101010111100111100110000101"

#define SHADE_CHANGE_DIRECTION_DOWNSTAIRS   "00100000000111011101111101111110000000110"
#define SHADE_CHANGE_DIRECTION_UPSTAIRS     "00101011001101011101111101111110001111111"


#define TRANSMIT_PIN          13      // We'll use digital 13 for transmitting
#define REPEAT_COMMAND        8       // How many times to repeat the same command: original remotes repeat 8 (multi) or 10 (single) times by default
#define DEBUG                 false   // Do note that if you add serial output during transmit, it will cause delay and commands may fail

// If you wish to use PORTB commands instead of digitalWrite, these are for Arduino Uno digital 13:
#define D13high | 0x20; 
#define D13low  & 0xDF; 

// Timings in microseconds. Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
#define MARKISOL_AGC1_PULSE                   2410  // 107 samples
#define MARKISOL_AGC2_PULSE                   1320  // 59 samples
#define MARKISOL_RADIO_SILENCE                5045  // 223 samples

#define MARKISOL_PULSE_SHORT                  300   // 13-16 samples
#define MARKISOL_PULSE_LONG                   680   // 26-32 samples

#define MARKISOL_COMMAND_BIT_ARRAY_SIZE       41    // Command bit count



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600); // Used for error messages even with DEBUG set to false
      
  if (DEBUG) Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:

  // Pair a shade (first set the shade to pairing mode by holding
  // down its red P button until the "TA-TA"):
  //sendMarkisolCommand(SHADE_PAIR_1);
  //while (true) {} // Stop after pairing, you can use UP/STOP/DOWN commands afterwards

  // Send the command:
  //sendMarkisolCommand(SHADE_DOWN_1);
  //sendMarkisolCommand(SHADE_DOWN_2);
  //sendMarkisolCommand(SHADE_DOWN_3);
  delay(3000);
  
  //sendMarkisolCommand(SHADE_STOP_1);
  //sendMarkisolCommand(SHADE_STOP_2);
  //sendMarkisolCommand(SHADE_STOP_3);
  delay(3000);
  
  //sendMarkisolCommand(SHADE_UP_1);
  //sendMarkisolCommand(SHADE_UP_2);
  //sendMarkisolCommand(SHADE_UP_3);
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

      if (c == "0" || c == "1" || c == "2" || c == "3") { // 2 and 3 are allowed for doManchesterSend() (not used by this code)
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
