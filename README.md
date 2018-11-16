# Control Markisol protocol 433.92MHz motorized window shades from Arduino
Compatible with iFit Spring Pro, Chinese brands like Bofu and remotes like BF-101, BF-301, BF-305, possibly others.

http://www.markisolgroup.com/en/products/ifit.html

I've automated a lot in my house, but there was that one thing that kept bothering me: the window shades. I've had motorized shades for a long time, but they would only work with their own remotes. I have 5 remote controls: one (model BF-305) can control all 5 separately. Four of the remotes (models BF-101 and BF-301) would normally control a single shade, but they're easy to pair with more (so 2-3 shades roll down at the same time).

There are many reasons to control window shades from a program. For starters, automating them from Arduino/Raspberry Pi (scheduling, based on outside temperature and/or sunlight, based on home alarm Home/Away mode etc.) and voice commands from Alexa via TRIGGERcmd. I also control them remotely via SSH & HTTPS and wrote a PHP script for Raspberry Pi, enabling control from browsers and mobile phones.

Markisol products have been sold by The Home Depot, IKEA and also under the name Feelstyle, so this protocol works with quite many products, identifiable by the remote control models. For example, I have motors from a Chinese brand Bofu, who utilize the same remotes. Unless I'm completely mistaken, each remote has its unique (or nearly unique), hard coded ID. Commands can be captured by RemoteCapture.ino and I've also included example commands (including PAIR/CONFIRM) from one remote. The purpose of this project was to get my own window shades automated, so there's more work to be done should you wish to fully reverse engineer the protocol and generate + add new "virtual remotes". Specifically, checksum calculation needs to be figured out.


# How to use
1. Load up RemoteCapture.ino and plug a 433.92MHz receiver to digital pin 2.
2. Open up Tools -> Serial Monitor in Arduino IDE and start pressing buttons from your original remotes.
3. Copy paste the 41 bit commands to Markisol.ino for sendMarkisolCommand(). I recommend using #define preprocessor directives to conserve memory.


# How to use with example commands
1. Set a shade into pairing mode by holding down its P/SETTING button until it shakes twice ("TA-TA") or beeps.
2. Send the pairing command, eg. "sendMarkisolCommand(SHADE_PAIR_EXAMPLE);", which will shake the shade twice ("TA-TA") or beep.
3. Now you can control the shade, eg. sendMarkisolCommand(SHADE_DOWN_EXAMPLE); (or SHADE_UP_EXAMPLE, SHADE_STOP_EXAMPLE etc.).
 
Setting limits is quicker with the remotes, although you can use your Arduino for that as well. Some motors do not erase the the limits even if you reset them by holding down the P/SETTING button for 8-10 seconds.
