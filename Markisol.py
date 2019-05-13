#!/usr/bin/python

"""
* Markisol iFit Spring Pro 433.92MHz window shades
* Also sold under the name Feelstyle and various Chinese brands like Bofu
* Compatible with remotes like BF-101, BF-301, BF-305, possibly others
*
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
*
* This is a Python implementation of the Markisol protocol, for
* the Raspberry Pi. Note that I have never tested this code. I've
* written it based on the (working) code for Arduino. The code may
* or may not work at all. It's possible that timings need to be
* modified for the Pi or that there are other issues.
*
* HOW TO USE
* ./Markisol.py [41-bit_binary_string]
*
* More info on the protocol in Markisol.ino and RemoteCapture.ino here:
* https://github.com/akirjavainen/markisol
*
"""

import time
import sys
import os
import RPi.GPIO as GPIO


TRANSMIT_PIN = 23
REPEAT_COMMAND = 8

MARKISOL_AGC1_PULSE = 0.02410
MARKISOL_AGC2_PULSE = 0.01320
MARKISOL_RADIO_SILENCE = 0.05045

MARKISOL_PULSE_SHORT = 0.00300
MARKISOL_PULSE_LONG = 0.00680

MARKISOL_COMMAND_BIT_ARRAY_SIZE = 41


# ------------------------------------------------------------------
def sendMarkisolCommand(command):

    if len(str(command)) is not MARKISOL_COMMAND_BIT_ARRAY_SIZE:
        print "Your (invalid) command was", len(str(command)), "bits long."
        print ""
        printUsage()

    # Prepare:
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(TRANSMIT_PIN, GPIO.OUT)

    # Send command:
    for t in range(REPEAT_COMMAND):
        doMarkisolTribitSend(command)

    # Disable output to transmitter and clean up:
    exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def doMarkisolTribitSend(command):

    # AGC bits:
    transmitWaveformHigh(MARKISOL_AGC1_PULSE)  # AGC 1
    transmitWaveformLow(MARKISOL_AGC2_PULSE)  # AGC 2

    for i in command:

        if i == '0':  # LOW-HIGH-LOW
            transmitWaveformLow(MARKISOL_PULSE_SHORT)
            transmitWaveformHigh(MARKISOL_PULSE_SHORT)
            transmitWaveformLow(MARKISOL_PULSE_SHORT)

        elif i == '1':  # HIGH-HIGH-LOW
            transmitWaveformHigh(MARKISOL_PULSE_LONG)
            transmitWaveformLow(MARKISOL_PULSE_SHORT)

        else:
            print "Invalid character", i, "in command! Exiting..."
            exitProgram()

    # Radio silence:
    transmitWaveformLow(MARKISOL_RADIO_SILENCE)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitWaveformHigh(delay):
    GPIO.output(TRANSMIT_PIN, 0)  # Invert to 1 if required
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitWaveformLow(delay):
    GPIO.output(TRANSMIT_PIN, 1)  # Invert to 0 if required
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def printUsage():
    print "Usage:"
    print os.path.basename(sys.argv[0]), "[command_string]"
    print
    print "Correct command length is", MARKISOL_COMMAND_BIT_ARRAY_SIZE, "bits."
    print
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def exitProgram():
    # Disable output to transmitter and clean up:
    GPIO.output(TRANSMIT_PIN, 0)
    GPIO.cleanup()
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
# Main program:
# ------------------------------------------------------------------
if len(sys.argv) < 2:
    printUsage()

sendMarkisolCommand(sys.argv[1])
