# Arduino control for Markisol iFit Spring Pro 433.92MHz window shades
I've automated a lot in my home, but there was still one thing that kept bothering me: the window shades. I've had motorized shades for a long time, but they would only work with their own remotes. I have 3 remote controls: one can control all 5 separately. Two of the remotes would normally control a single shade, but they're easy to pair with more shades (so 2-3 shades roll down at the same time).

There are many reasons to control window shades from a program. For starters, automating them from Arduino/Raspberry Pi (scheduling, based on outside temperature and/or sunlight, based on home alarm Home/Away mode etc.) and voice commands from Alexa via TRIGGERcmd. I also control them remotely via SSH & HTTPS and wrote a PHP script for Raspberry Pi.

Markisol products have been sold by IKEA, so this protocol may or may not work with other products. The biggest problem could be the pairing process: since I don't have extra shades to reverse engineer, at the moment it's pretty much impossible to tell how the ID's are generated or if they even change during pairing. Try the commands from my remotes first (defined in the code) and if none are working for you, simply use an oscillator to copy the code your own remote is transmitting.



# Poor man's oscillator
I built a "poor man's oscillator" by plugging a 433.92MHz receiver unit data pin -> 10K Ohm resistor -> USB sound card line-in. Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND). My translation of the waveform is simple: high = 1, low = 0. Two consecutive highs are 11 and two consecutive lows 00.

To view the waveform Arduino is transmitting, I found it easiest to connect the digital pin (13) directly from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was as clear as the original and debugging became easy.



# More work needed
How are device ID's generated for remote controls and shades? More work, remotes and shades are needed. So if you own any of these shades, it would be great if you could send me either the WAV files or deciphers (as 1's and 0's) of the codes your remotes send. You can even snail mail me the remotes themselves, but that would be at your own expense, as well as returning them. :)



(c) Antti Kirjavainen 2018
