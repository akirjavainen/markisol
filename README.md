# Control Markisol protocol 433.92MHz motorized window shades from Arduino and Raspberry Pi
Compatible with iFit Spring Pro, Chinese brands like Bofu and remotes like BF-101, BF-301, BF-305, possibly others.

http://www.markisolgroup.com/en/products/ifit.html

I've automated a lot in my house, but there was that one thing that kept bothering me: the window shades. I've had motorized shades for a long time, but they would only work with their own remotes. I have different types of remote controls: BF-305 can control 5 different channels. Others, like BF-101 and BF-301 would normally control a single shade, but you can pair them with as many as you like.

There are many reasons to control window shades from a program. For starters, automating them from Arduino/Raspberry Pi (scheduling, based on outside temperature and/or sunlight, based on home alarm Home/Away mode etc.) and voice commands from Alexa via TRIGGERcmd. I also control them remotely via SSH & HTTPS and wrote a PHP script for Raspberry Pi, enabling control from browsers and mobile phones.

Markisol products have been sold by The Home Depot, IKEA, Rollerhouse and also under the name Feelstyle, so this protocol works with quite many products, identifiable by the remote control models. I have motors from a Chinese brand Bofu that utilize the same remotes. Unless I'm completely mistaken, each remote has its unique (or nearly unique), hard coded ID. Commands can be captured by RemoteCapture.ino and I've also included example commands (including PAIR/CONFIRM) from one remote.


# How to use
1. Load up RemoteCapture.ino and plug a 433.92MHz receiver to digital pin 2.
2. Open up Tools -> Serial Monitor in Arduino IDE and start pressing buttons from your original remotes.
3. Copy paste the 41 bit commands to Markisol.ino for sendMarkisolCommand(). I recommend using #define preprocessor directives to conserve memory. In case of single channel remotes like the BF-301, you could simply call the sendShortMarkisolCommand() function with the 16 bit remote ID and COMMAND_DOWN (for example).


# How to use with example commands
1. Set a shade into pairing mode by holding down its P/SETTING button until it shakes twice ("TA-TA") or beeps.
2. Send the pairing command, eg. "sendMarkisolCommand(SHADE_PAIR_EXAMPLE);", which will shake the shade twice ("TA-TA") or beep.
3. Now you can control the shade, eg. sendMarkisolCommand(SHADE_DOWN_EXAMPLE); (or SHADE_UP_EXAMPLE, SHADE_STOP_EXAMPLE etc.).
 
Setting limits is quicker with the remotes, although you can use your Arduino for that as well. Some motors do not erase the the limits even if you reset them by holding down the P/SETTING button for 8-10 seconds. They just reset the list of registered remote controls.
