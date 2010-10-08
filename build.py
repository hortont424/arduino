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

def buildProject(sources, config, libraries=None):
    sources = " ".join(sources)

    corePath = os.path.join(config["ARDUINO_PATH"], "hardware", "arduino", "cores", "arduino")
    libsPath = os.path.join(config["ARDUINO_PATH"], "libraries")
    downloaderPath = os.path.join(config["ARDUINO_PATH"], "hardware", "tools", "avr")

    includeFlags = "-I{0}".format(corePath)

    for library in libraries:
        includeFlags += " -I{0}".format(os.path.join(libsPath, library))

    print corePath
    print includeFlags

buildProject(["binary-counter.pde"], loadConfiguration(), libraries=["SPI"])