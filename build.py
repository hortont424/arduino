#!/usr/bin/env python

import os
import json
import serial
import time
import sys
from util import *

class ConfigurationException(Exception):
    def __init__(self, problem):
        self.problem = problem

    def __str(self):
        return "ConfigurationError: {0}".format(self.problem)

class BuildException(Exception):
    def __init__(self, filename):
        self.filename = filename

    def __str(self):
        return "BuildException: {0}".format(self.filename)

def pulseDTR(port):
    ser = serial.Serial(port)
    ser.setDTR(1)
    time.sleep(0.5)
    ser.setDTR(0)
    ser.close()

def loadConfiguration():
    def findArduino():
        paths = ["/Applications/Arduino.app/Contents/Resources/Java"]

        for path in paths:
            if os.path.exists(path):
                return path

        raise ConfigurationException("Couldn't find Arduino IDE installation.")

    try:
        config = json.loads(readFile("global-config.js"))
    except e:
        raise ConfigurationException("Couldn't parse global-config.js: {0}".format(repr(e)))

    config["ARDUINO_PATH"] = findArduino()

    return config

def buildProject(progSources, config, libraries=None):
    cSources = []
    cppSources = []

    progSources.append("support.cpp")

    corePath = os.path.join(config["ARDUINO_PATH"], "hardware", "arduino", "cores", "arduino")
    libsPath = os.path.join(config["ARDUINO_PATH"], "libraries")
    downloaderPath = os.path.join(config["ARDUINO_PATH"], "hardware", "tools", "avr")
    avrDudePath = os.path.join(downloaderPath, "bin", "avrdude")

    includeFlags = "-I. -I{0}".format(corePath)

    for library in libraries:
        includeFlags += " -I{0}".format(os.path.join(libsPath, library))

    buildFlags = [includeFlags]
    buildFlags.append("-DF_CPU={0}".format(config["CPU_FREQUENCY"]))
    buildFlags.append("-Os")
    buildFlags.append("-mmcu={0}".format(config["CPU"]))
    #buildFlags.append("-w")

    cBuildFlags = buildFlags[:]
    cppBuildFlags = buildFlags[:]
    asBuildFlags = buildFlags[:]

    cBuildFlags.append("-std=gnu99")
    cBuildFlags.append("-gstabs")
    cBuildFlags.append("-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")

    asBuildFlags.append("-x assembler-with-cpp")

    avrDudeFlags = []
    avrDudeFlags.append("-V -F")
    avrDudeFlags.append("-C {0}".format(os.path.join(downloaderPath, "etc", "avrdude.conf")))
    avrDudeFlags.append("-p {0}".format(config["CPU"]))
    avrDudeFlags.append("-P {0}".format(config["SERIAL_PORT"]))
    avrDudeFlags.append("-c {0}".format(config["PROGRAMMER"]))
    avrDudeFlags.append("-b {0}".format(config["SERIAL_RATE"]))

    for basePath in [corePath] + [os.path.join(libsPath, library) for library in libraries]:
        for path, dirs, files in os.walk(basePath):
            for filename in [os.path.abspath(os.path.join(path, filename)) for filename in files]:
                if filename.endswith(".cpp"):
                    if filename.endswith("Tone.cpp") or filename.endswith("WString.cpp") or filename.endswith("main.cpp"):
                        continue
                    cppSources.append(filename)
                elif filename.endswith(".c"):
                    cSources.append(filename)

    objects = []

    for src in cSources:
        obj = os.path.basename(src).replace(".c", ".o")
        objects.append(obj)
        if(os.system("avr-gcc -c {0} {1} -o {2}".format(" ".join(cBuildFlags), src, obj))):
            raise BuildException(src)

    for src in cppSources:
        obj = os.path.basename(src).replace(".cpp", ".o")
        objects.append(obj)
        if(os.system("avr-g++ -c {0} {1} -o {2}".format(" ".join(cppBuildFlags), src, obj))):
            raise BuildException(src)

    for src in objects:
        os.system("avr-ar rcs core.a {0}".format(src))

    if(os.system("avr-g++ -lm {0} {1} core.a -o out.elf".format(" ".join(cppBuildFlags), " ".join(progSources)))):
        raise BuildException("linking")

    os.system("avr-objcopy -O ihex -R .eeprom out.elf out.hex")

    pulseDTR(config["SERIAL_PORT"])
    os.system("{0} {1} -U flash:w:out.hex".format(avrDudePath, " ".join(avrDudeFlags)))

buildProject(["binary-counter.cpp"], loadConfiguration(), libraries=["SPI"])