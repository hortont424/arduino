#!/usr/bin/env python

import os
import json
from util import *

class ConfigurationException(Exception):
    def __init__(self, problem):
        self.problem = problem

    def __str(self):
        return "ConfigurationError: {0}".format(self.problem)

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

def buildProject(cppSources, config, libraries=None):
    cSources = []

    corePath = os.path.join(config["ARDUINO_PATH"], "hardware", "arduino", "cores", "arduino")
    libsPath = os.path.join(config["ARDUINO_PATH"], "libraries")
    downloaderPath = os.path.join(config["ARDUINO_PATH"], "hardware", "tools", "avr")

    includeFlags = "-I. -I{0}".format(corePath)

    for library in libraries:
        includeFlags += " -I{0}".format(os.path.join(libsPath, library))

    buildFlags = [includeFlags]
    buildFlags.append("-DF_CPU={0}".format(config["CPU_FREQUENCY"]))
    buildFlags.append("-Os")
    buildFlags.append("-mmcu={0}".format(config["CPU"]))

    cBuildFlags = buildFlags[:]
    cppBuildFlags = buildFlags[:]
    asBuildFlags = buildFlags[:]

    cBuildFlags.append("-std=gnu99")
    cBuildFlags.append("-gstabs")
    cBuildFlags.append("-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")

    asBuildFlags.append("-x assembler-with-cpp")

    ldBuildFlags = ["-lm"]

    for path, dirs, files in os.walk(corePath):
        for filename in [os.path.abspath(os.path.join(path, filename)) for filename in files]:
            if filename.endswith(".cpp"):
                cppSources.append(filename)
            elif filename.endswith(".c"):
                cSources.append(filename)

    for src in cSources:
        os.system("avr-gcc -c {0} {1} -o {2}".format(" ".join(cBuildFlags), src, src.replace(".c", ".o")))

buildProject(["binary-counter.pde"], loadConfiguration(), libraries=["SPI"])