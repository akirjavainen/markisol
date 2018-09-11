# Control Markisol iFit Spring Pro 433.92MHz window shades from Arduino Uno Rev3
http://www.markisolgroup.com/en/products/ifit.html

I've automated a lot in my house, but there was still one thing that kept bothering me: the window shades. I've had motorized shades for a long time, but they would only work with their own remotes. I have 3 remote controls: one (model BF-305) can control all 5 separately. Two of the remotes (model BF-101) would normally control a single shade, but they're easy to pair with more shades (so 2-3 shades roll down at the same time).

There are many reasons to control window shades from a program. For starters, automating them from Arduino/Raspberry Pi (scheduling, based on outside temperature and/or sunlight, based on home alarm Home/Away mode etc.) and voice commands from Alexa via TRIGGERcmd. I also control them remotely via SSH & HTTPS and wrote a PHP script for Raspberry Pi.

Markisol products have been sold by IKEA and also under the name Feelstyle, so this protocol may or may not work with other products. Unless I'm completely mistaken, each remote has its unique (or nearly unique) ID. However, I've included the pairing commands from my remotes, so you can simply pair them to your shades and take advantage of them directly. The purpose of this project was to get my own window shades automated, so there's a lot more work to be done should you wish to fully reverse engineer the codes and generate + add new "virtual remotes".


# Usage
1. Set the shade into pairing mode by holding down its red P button until it shakes twice ("TA-TA").
2. Send the pairing command from this code, eg. "sendMarkisolCommand(SHADE_PAIR_1);", which will shake the shade twice ("TA-TA").
3. Control the shade with that channel's codes, eg. sendMarkisolCommand(SHADE_DOWN_1); (or SHADE_UP_1, SHADE_STOP_1 etc.).

Setting limits is quicker and easier with the original remotes. Typically, limits are not lost even if you reset the shade by holding down its red P button for 8-10 seconds.


# Poor man's oscillator
Commands were captured by a "poor man's oscillator": plugging a 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in. Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND). My translation of the waveform is simple: high = 1, low = 0. Two consecutive highs are 11 and two consecutive lows 00.

To view the waveform Arduino is transmitting, I found it easiest to connect the digital pin (13) directly from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was as clear as the original and debugging became easy.


# More information and work required
BF-305 and BF-101 seem like some kind of generic remotes, used by many different vendors and products. Are they all based on this format? No idea. It's possible each vendor reprograms them for their products. Commands haven't really been reverse engineered (not yet!).
