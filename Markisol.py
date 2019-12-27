#!/usr/bin/python

"""
* Markisol iFit Spring Pro 433.92MHz window shades
* Also sold under the name Feelstyle and various Chinese brands like Bofu
* Compatible with remotes like BF-101, BF-301, BF-305, possibly others
*
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
*
* This is a Python implementation of the Markisol protocol, for
* the Raspberry Pi. Plug your transmitter to BOARD PIN 16 (BCM/GPIO23).
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


TRANSMIT_PIN = 16  # BCM PIN 23 (GPIO23, BOARD PIN 16)
REPEAT_COMMAND = 8


# Microseconds (us) converted to seconds for time.sleep() function:
MARKISOL_AGC1_PULSE = 0.004885
MARKISOL_AGC2_PULSE = 0.00245
MARKISOL_AGC3_PULSE = 0.0017
MARKISOL_RADIO_SILENCE = 0.005057

MARKISOL_PULSE_SHORT = 0.00034
MARKISOL_PULSE_LONG = 0.00068

MARKISOL_COMMAND_BIT_ARRAY_SIZE = 41


# ------------------------------------------------------------------
def sendMarkisolCommand(command):

    if len(str(command)) is not MARKISOL_COMMAND_BIT_ARRAY_SIZE:
        print("Your (invalid) command was", len(str(command)), "bits long.")
        print
        printUsage()

    # Prepare:
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(TRANSMIT_PIN, GPIO.OUT)

    # Send command:
    for t in range(REPEAT_COMMAND):
        doMarkisolTribitSend(command)

    # Radio silence at the end of last command:
    transmitLow(MARKISOL_RADIO_SILENCE)

    # Disable output to transmitter and clean up:
    exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def doMarkisolTribitSend(command):

    # AGC bits:
    transmitHigh(MARKISOL_AGC1_PULSE)  # AGC 1
    transmitLow(MARKISOL_AGC2_PULSE)  # AGC 2
    transmitHigh(MARKISOL_AGC3_PULSE)  # AGC 3

    for i in command:

        if i == '0':  # LOW-HIGH-LOW
            transmitLow(MARKISOL_PULSE_SHORT)
            transmitHigh(MARKISOL_PULSE_SHORT)
            transmitLow(MARKISOL_PULSE_SHORT)

        elif i == '1':  # LOW-HIGH-HIGH
            transmitLow(MARKISOL_PULSE_SHORT)
            transmitHigh(MARKISOL_PULSE_LONG)

        else:
            print("Invalid character", i, "in command! Exiting...")
            exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitHigh(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.HIGH)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitLow(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def printUsage():
    print("Usage:")
    print(os.path.basename(sys.argv[0]), "[command_string]")
    print
    print("Correct command length is", MARKISOL_COMMAND_BIT_ARRAY_SIZE, "bits.")
    print
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def exitProgram():
    # Disable output to transmitter and clean up:
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    GPIO.cleanup()
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
# Main program:
# ------------------------------------------------------------------
if len(sys.argv) < 2:
    printUsage()

sendMarkisolCommand(sys.argv[1])
