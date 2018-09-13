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
* to get my own window shades automated, so there's a lot more work to be done should you wish to fully reverse engineer the codes
* and generate + add new "virtual remotes".
* 
* USAGE:
* 1. Set the shade into pairing mode by holding down its red P button until it shakes twice ("TA-TA").
* 2. Send the pairing command from this code, eg. "sendMarkisolCommand(SHADE_PAIR_1);", which will shake the shade twice ("TA-TA").
* 3. Control the shade with that channel's codes, eg. sendMarkisolCommand(SHADE_DOWN_1); (or SHADE_UP_1, SHADE_STOP_1 etc.).
* 
* Setting limits is quicker and easier with the original remotes. Typically, limits are not lost even if you reset the shade
* by holding down its red P button for 8-10 seconds.
* 
* Commands were captured by a "poor man's oscillator": 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in.
* Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND).
* My translation of the waveform is simple: high = 1, low = 0. Two consecutive highs are 11 and two consecutive lows 00.
* 
* To view the waveform Arduino is transmitting (and debugging timings etc.), I found it easiest to connect the digital pin (13)
* from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was as clear as the original.
* 
* BF-305 and BF-101 seem like some kind of generic remotes, used by many different vendors and products. Are they all based on
* this format? No idea. It's possible each vendor reprograms them for their products.
* 
* 
* On-Off Keying (OOK) is used.
* A single command is: 2 AGC bits + 123 command bits + radio silence
* Swapping HIGH and LOW does not work with these shades.
*
* Starting (AGC) bits:
* HIGH of 107 samples / 44100 = 2426 microseconds - lag compensation
* LOW of 73 samples / 44100 = 1655 microseconds - lag compensation
*
*
* Remote 1 multi (model BF-305), controlling 5 different shades:
*
*                  REMOTE CONTROL ID                    CHANNEL ID?       COMMAND           REMOTE CONTROL ID       CHANNEL ID?      ?      COMMAND?
* 10110010010110110010010110010010010110010110010110 -- 010110110110 -- 0101101101 -- 10010110110110110010010110 -- 110010110110 -- 0101 -- 100100100   = Shade 1 DOWN
* 10110010010110110010010110010010010110010110010110 -- 110010110110 -- 0101101101 -- 10010110110110110010010110 -- 010110110110 -- 0101 -- 100100100   = Shade 2 DOWN
* 10110010010110110010010110010010010110010110010110 -- 010010110110 -- 0101101101 -- 10010110110110110010010110 -- 110110110110 -- 0101 -- 100100100   = Shade 3 DOWN
* 10110010010110110010010110010010010110010110010110 -- 110110010110 -- 0101101101 -- 10010110110110110010010110 -- 010010010010 -- 1101 -- 100100100   = Shade 4 DOWN
* 10110010010110110010010110010010010110010110010110 -- 010110010110 -- 0101101101 -- 10010110110110110010010110 -- 110010010010 -- 1101 -- 100100100   = Shade 5 DOWN
*
* 10110010010110110010010110010010010110010110010110 -- 010110110110 -- 1101100100 -- 10010110110110110010010110 -- 110010110110 -- 1100 -- 101101100   = Shade 1 UP
* 10110010010110110010010110010010010110010110010110 -- 110010110110 -- 1101100100 -- 10010110110110110010010110 -- 010110110110 -- 1100 -- 101101100   = Shade 2 UP
* 10110010010110110010010110010010010110010110010110 -- 010010110110 -- 1101100100 -- 10010110110110110010010110 -- 110110110110 -- 1100 -- 101101100   = Shade 3 UP
* 10110010010110110010010110010010010110010110010110 -- 110110010110 -- 1101100100 -- 10010110110110110010010110 -- 010010010010 -- 0101 -- 101101100   = Shade 4 UP
* 10110010010110110010010110010010010110010110010110 -- 010110010110 -- 1101100100 -- 10010110110110110010010110 -- 110010010010 -- 0101 -- 101101100   = Shade 5 UP
*
* 10110010010110110010010110010010010110010110010110 -- 010110110110 -- 0101100101 -- 10010110110110110010010110 -- 110010110110 -- 0101 -- 101100100   = Shade 1 STOP
* 10110010010110110010010110010010010110010110010110 -- 110010110110 -- 0101100101 -- 10010110110110110010010110 -- 010110110110 -- 0101 -- 101100100   = Shade 2 STOP
* 10110010010110110010010110010010010110010110010110 -- 010010110110 -- 0101100101 -- 10010110110110110010010110 -- 110110110110 -- 0101 -- 101100100   = Shade 3 STOP
* 10110010010110110010010110010010010110010110010110 -- 110110010110 -- 0101100101 -- 10010110110110110010010110 -- 010010010010 -- 1101 -- 101100100   = Shade 4 STOP
* 10110010010110110010010110010010010110010110010110 -- 010110010110 -- 0101100101 -- 10010110110110110010010110 -- 110010010010 -- 1101 -- 101100100   = Shade 5 STOP
*
* 10110010010110110010010110010010010110010110010110 -- 010010010010 -- 0101101101 -- 10010110110110110010010110 -- 110110010110 -- 1101 -- 100100100   = All shades DOWN
* 10110010010110110010010110010010010110010110010110 -- 010010010010 -- 1101100100 -- 10010110110110110010010110 -- 110110010110 -- 0101 -- 101101100   = All shades UP
* 10110010010110110010010110010010010110010110010110 -- 010010010010 -- 0101100101 -- 10010110110110110010010110 -- 110110010110 -- 1101 -- 101100100   = All shades STOP
* 
* 10110010010110110010010110010010010110010110010110 -- 010110110110 -- 1101100101 -- 10010110110110110010010110 -- 110010110110 -- 1100 -- 101100100   = Pair channel 1
* 10110010010110110010010110010010010110010110010110 -- 110010110110 -- 1101100101 -- 10010110110110110010010110 -- 010110110110 -- 1100 -- 101100100   = Pair channel 2
* 10110010010110110010010110010010010110010110010110 -- 010010110110 -- 1101100101 -- 10010110110110110010010110 -- 110110110110 -- 1100 -- 101100100   = Pair channel 3
* 10110010010110110010010110010010010110010110010110 -- 110110010110 -- 1101100101 -- 10010110110110110010010110 -- 010010010010 -- 0101 -- 101100100   = Pair channel 4
* 10110010010110110010010110010010010110010110010110 -- 010110010110 -- 1101100101 -- 10010110110110110010010110 -- 110010010010 -- 0101 -- 101100100   = Pair channel 5
* 
* 10110010010110110010010110010010010110010110010110 -- 010110110110 -- 1100101101 -- 10010110110110110010010110 -- 110010110110 -- 1101 -- 100100100   = Set limits channel 1 
* 10110010010110110010010110010010010110010110010110 -- 110010110110 -- 1100101101 -- 10010110110110110010010110 -- 010110110110 -- 1101 -- 100100100   = Set limits channel 2
* 10110010010110110010010110010010010110010110010110 -- 010010110110 -- 1100101101 -- 10010110110110110010010110 -- 110110110110 -- 1101 -- 100100100   = Set limits channel 3
* 10110010010110110010010110010010010110010110010110 -- 110110010110 -- 1100101101 -- 10010110110110110010010110 -- 010010010010 -- 0100 -- 101100100   = Set limits channel 4
* 10110010010110110010010110010010010110010110010110 -- 010110010110 -- 1100101101 -- 10010110110110110010010110 -- 110010010010 -- 0100 -- 101100100   = Set limits channel 5
* 
* 
* Remote 2 (model BF-101), single UP/STOP/DOWN, programmed for shades 1, 2 and 3 downstairs:
* 10010110010010010010010010010010110110110010110110 -- 110010110110 -- 0101101101 -- 10110110110110110110010010 -- 010010010010 -- 1101 -- 101101100   = Shades DOWN
* 10010110010010010010010010010010110110110010110110 -- 110010110110 -- 1101100100 -- 10110110110110110110010010 -- 010010010010 -- 0101 -- 100101100   = Shades UP
* 10010110010010010010010010010010110110110010110110 -- 110010110110 -- 0101100101 -- 10110110110110110110010010 -- 010010010010 -- 1101 -- 100100100   = Shades STOP
* 10010110010010010010010010010010110110110010110110 -- 110010110110 -- 1101100101 -- 10110110110110110110010010 -- 010010010010 -- 0101 -- 100100100   = Pair
* 10010110010010010010010010010010110110110010110110 -- 110010110110 -- 1100101101 -- 10110110110110110110010010 -- 010010010010 -- 0100 -- 100100100   = Set limits
* 
*
* Remote 3 (model BF-101), single UP/STOP/DOWN, programmed for shades 4 and 5 upstairs:
* 10010110010110010110110010010110110010110010110110 -- 110010110110 -- 0101101101 -- 10110110110110110110010010 -- 010110110110 -- 0100 -- 100101100   = Shades DOWN
* 10010110010110010110110010010110110010110010110110 -- 110010110110 -- 1101100100 -- 10110110110110110110010010 -- 010110110110 -- 1101 -- 100100100   = Shades UP
* 10010110010110010110110010010110110010110010110110 -- 110010110110 -- 0101100101 -- 10110110110110110110010010 -- 010110110110 -- 0100 -- 101101100   = Shades STOP
* 10010110010110010110110010010110110010110010110110 -- 110010110110 -- 1101100101 -- 10110110110110110110010010 -- 010110110110 -- 1101 -- 100101100   = Pair
* 10010110010110010110110010010110110010110010110110 -- 110010110110 -- 1100101101 -- 10110110110110110110010010 -- 010110110110 -- 1100 -- 100101100   = Set limits
*
*
* End with LOW radio silence of 209 samples / sample rate = 4739 microseconds
* 
******************************************************************************************************************************************************************
*/


#define SHADE_DOWN_1              "101100100101101100100101100100100101100101100101100101101101100101101101100101101101101100100101101100101101100101100100100"
#define SHADE_DOWN_2              "101100100101101100100101100100100101100101100101101100101101100101101101100101101101101100100101100101101101100101100100100"
#define SHADE_DOWN_3              "101100100101101100100101100100100101100101100101100100101101100101101101100101101101101100100101101101101101100101100100100"
#define SHADE_DOWN_4              "101100100101101100100101100100100101100101100101101101100101100101101101100101101101101100100101100100100100101101100100100"
#define SHADE_DOWN_5              "101100100101101100100101100100100101100101100101100101100101100101101101100101101101101100100101101100100100101101100100100"

#define SHADE_DOWN_DOWNSTAIRS     "100101100100100100100100100100101101101100101101101100101101100101101101101101101101101101100100100100100100101101101101100"
#define SHADE_DOWN_UPSTAIRS       "100101100101100101101100100101101100101100101101101100101101100101101101101101101101101101100100100101101101100100100101100"
#define SHADE_DOWN_ALL            "101100100101101100100101100100100101100101100101100100100100100101101101100101101101101100100101101101100101101101100100100"

#define SHADE_UP_1                "101100100101101100100101100100100101100101100101100101101101101101100100100101101101101100100101101100101101101100101101100"
#define SHADE_UP_2                "101100100101101100100101100100100101100101100101101100101101101101100100100101101101101100100101100101101101101100101101100"
#define SHADE_UP_3                "101100100101101100100101100100100101100101100101100100101101101101100100100101101101101100100101101101101101101100101101100"
#define SHADE_UP_4                "101100100101101100100101100100100101100101100101101101100101101101100100100101101101101100100101100100100100100101101101100"
#define SHADE_UP_5                "101100100101101100100101100100100101100101100101100101100101101101100100100101101101101100100101101100100100100101101101100"

#define SHADE_UP_DOWNSTAIRS       "100101100100100100100100100100101101101100101101101100101101101101100100101101101101101101100100100100100100100101100101100"
#define SHADE_UP_UPSTAIRS         "100101100101100101101100100101101100101100101101101100101101101101100100101101101101101101100100100101101101101101100100100"
#define SHADE_UP_ALL              "101100100101101100100101100100100101100101100101100100100100101101100100100101101101101100100101101101100101100101101101100"

#define SHADE_STOP_1              "101100100101101100100101100100100101100101100101100101101101100101100101100101101101101100100101101100101101100101101100100"
#define SHADE_STOP_2              "101100100101101100100101100100100101100101100101101100101101100101100101100101101101101100100101100101101101100101101100100"
#define SHADE_STOP_3              "101100100101101100100101100100100101100101100101100100101101100101100101100101101101101100100101101101101101100101101100100"
#define SHADE_STOP_4              "101100100101101100100101100100100101100101100101101101100101100101100101100101101101101100100101100100100100101101101100100"
#define SHADE_STOP_5              "101100100101101100100101100100100101100101100101100101100101100101100101100101101101101100100101101100100100101101101100100"

#define SHADE_STOP_DOWNSTAIRS     "100101100100100100100100100100101101101100101101101100101101100101100101101101101101101101100100100100100100101101100100100"
#define SHADE_STOP_UPSTAIRS       "100101100101100101101100100101101100101100101101101100101101100101100101101101101101101101100100100101101101100100101101100"
#define SHADE_STOP_ALL            "101100100101101100100101100100100101100101100101100100100100100101100101100101101101101100100101101101100101101101101100100"

#define SHADE_PAIR_1              "101100100101101100100101100100100101100101100101100101101101101101100101100101101101101100100101101100101101101100101100100"
#define SHADE_PAIR_2              "101100100101101100100101100100100101100101100101101100101101101101100101100101101101101100100101100101101101101100101100100"
#define SHADE_PAIR_3              "101100100101101100100101100100100101100101100101100100101101101101100101100101101101101100100101101101101101101100101100100"
#define SHADE_PAIR_4              "101100100101101100100101100100100101100101100101101101100101101101100101100101101101101100100101100100100100100101101100100"
#define SHADE_PAIR_5              "101100100101101100100101100100100101100101100101100101100101101101100101100101101101101100100101101100100100100101101100100"

#define SHADE_PAIR_DOWNSTAIRS     "100101100100100100100100100100101101101100101101101100101101101101100101101101101101101101100100100100100100100101100100100"
#define SHADE_PAIR_UPSTAIRS       "100101100101100101101100100101101100101100101101101100101101101101100101101101101101101101100100100101101101101101100101100"

#define SHADE_LIMIT_1             "101100100101101100100101100100100101100101100101100101101101101100101101100101101101101100100101101100101101101101100100100"
#define SHADE_LIMIT_2             "101100100101101100100101100100100101100101100101101100101101101100101101100101101101101100100101100101101101101101100100100"
#define SHADE_LIMIT 3             "101100100101101100100101100100100101100101100101100100101101101100101101100101101101101100100101101101101101101101100100100"
#define SHADE_LIMIT_4             "101100100101101100100101100100100101100101100101101101100101101100101101100101101101101100100101100100100100100100101100100"
#define SHADE_LIMIT_5             "101100100101101100100101100100100101100101100101100101100101101100101101100101101101101100100101101100100100100100101100100"

#define SHADE_LIMIT_DOWNSTAIRS    "100101100100100100100100100100101101101100101101101100101101101100101101101101101101101101100100100100100100100100100100100"
#define SHADE_LIMIT_UPSTAIRS      "100101100101100101101100100101101100101100101101101100101101101100101101101101101101101101100100100101101101101100100101100"


#define TRANSMIT_PIN 13 // We'll use digital 13 for transmitting
#define REPEAT_COMMAND 8 // How many times to repeat the same command: original remotes repeat 8 (multi) or 10 (single) times by default
#define DEBUG false // Disable serial output in actual use, as it WILL delay transmitting (thus causing commands to fail)

// We'll use digital 13 for transmitting.
// These are for Arduino Uno and may require changing for other models (or try using digitalWrite() instead):
#define D13high | 0x20; 
#define D13low  & 0xDF; 

// Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
#define PULSE_SHORT_MICRO_SECONDS 300 // 13-16 samples
#define PULSE_ADDITIONAL_LONG_MICRO_SECONDS 380 // 26-32 samples: sendMarkisolCommand() delays this + PULSE_SHORT_MICRO_SECONDS = 680 microseconds all in all
#define COMMAND_BIT_ARRAY_SIZE 123 // Command bit count



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  // put your setup code here, to run once:

  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
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
  
  if (command.length() < COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendMarkisolCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (command.length() > COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendMarkisolCommand(): Invalid command (too long), cannot continue.");
    return;
  }

  // Declare the array (int) of command bits
  int command_array[COMMAND_BIT_ARRAY_SIZE];

  // Processing a string during transmit is just too slow,
  // let's convert it to an array of int first:
  convertStringToArrayOfInt(command, command_array);
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doSend(command_array);
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doSend(int *command_array) {
  int previous = 2;

  if (command_array == NULL) {
    errorLog("doSend(): Array pointer was NULL, cannot continue.");
    return;
  }

  // Starting (AGC) bits:
  transmitWaveformHigh(2410); // 107 samples
  transmitWaveformLow(1640); // 73 samples

  // Transmit command:
  for (int i = 0; i < COMMAND_BIT_ARRAY_SIZE; i++) {

    // If current bit is 0:
    if (command_array[i] == 0) {
      if (previous != 0) { // If previous bit was 1, change pin state
        transmitWaveformLow(PULSE_SHORT_MICRO_SECONDS);
      } else { // If previous bit was already 0, only extend the delay
        additionalDelay(PULSE_ADDITIONAL_LONG_MICRO_SECONDS);
      }
    }

    // If current bit is 1:
    if (command_array[i] == 1) {
      if (previous != 1) { // If previous bit was 0, change pin state
        transmitWaveformHigh(PULSE_SHORT_MICRO_SECONDS);
      } else { // If previous bit was already 1, only extend the delay
        additionalDelay(PULSE_ADDITIONAL_LONG_MICRO_SECONDS);
      }
    }

    previous = command_array[i];
   }

  // Radio silence. Length is 209 samples.
  // It's better to rather go a bit over than under that.
  transmitWaveformLow(4780);
  
  if (DEBUG) {
    Serial.println();
    Serial.print("Transmitted ");
    Serial.print(COMMAND_BIT_ARRAY_SIZE);
    Serial.println(" bits.");
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitWaveformHigh(int delay_microseconds) {
  PORTB = PORTB D13low; // Digital pin low transmits a high waveform
  //digitalWrite(TRANSMIT_PIN, LOW); // Try this if the PORTB command fails (a different Arduino model etc.)
  delayMicroseconds(delay_microseconds);

  if (DEBUG) {
    Serial.print("1(");
    Serial.print(delay_microseconds);
    Serial.print(")-");
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitWaveformLow(int delay_microseconds) {
  PORTB = PORTB D13high; // Digital pin high transmits a low waveform
  //digitalWrite(TRANSMIT_PIN, HIGH); // Try this if the PORTB command fails (a different Arduino model etc.)
  delayMicroseconds(delay_microseconds);

  if (DEBUG) {
    Serial.print("0(");
    Serial.print(delay_microseconds);
    Serial.print(")-");
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void additionalDelay(int delay_microseconds) {
  delayMicroseconds(delay_microseconds);
  
  if (DEBUG) {
    Serial.print("(L)-");
  }
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
void convertStringToArrayOfInt(String command, int *int_array) {
  String c = "";

  if (int_array == NULL) {
    errorLog("convertStringToArrayOfInt(): Array pointer was NULL, cannot continue.");
    return;
  }
 
  for (int i = 0; i < COMMAND_BIT_ARRAY_SIZE; i++) {
      c = command.substring(i, i + 1);

      if (c == "0" || c == "1") {
        int_array[i] = convertStringToInt(c);
      } else {
        errorLog("convertStringToArrayOfInt(): Invalid character " + c + " in command, only 1 and 0 are accepted.");
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
