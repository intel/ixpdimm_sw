#!/usr/bin/env python

import subprocess
import platform
import os
import time
import sys

#################################################################
# Appends to create script name for usage info
#################################################################
usage = "[cliName=<name>] [serviceName=<name>] [fullProductName=<name>] [abbrProductName=<name>] [dataPath=<full-path>] [installPath=<full-path>] [outputPath=<full-path>]"

#################################################################
# Defaults.  If product name, etc. changes, change these.
#################################################################
cliName = "ixpdimm-cli"
fullProductName = "Intel(R) DIMM"
abbrProductName = "DIMM"
serviceName = "ixpdimm-monitor"
dataPath = "/usr/share/nvdimm-mgmt"
installPath = "/var/run"
outputPath = "../output/build/linux/real/debug"

#################################################################
# set full product name, abbreviated product name, and cli name from input
#################################################################
def setProductName(arg):
	rc = 0
	if "=" in arg:
		nameToSet, nameStr = arg.split("=",1)
		if nameToSet == "fullProductName":
			global fullProductName
			fullProductName = nameStr
		elif nameToSet == "abbrProductName":
			global abbrProductName
			abbrProductName = nameStr
		elif nameToSet == "cliName":
			global cliName
			cliName = nameStr
		elif nameToSet == "serviceName":
			global serviceName
			serviceName = nameStr
		elif nameToSet == "dataPath":
			global dataPath
			dataPath = nameStr
		elif nameToSet == "installPath":
			global installPath
			installPath = nameStr
		elif nameToSet == "outputPath":
			global outputPath
			outputPath = nameStr
		else:
			print("ERROR:  Invalid name setting '" + nameToSet + "'.")
			rc = -1
	else:
		print("ERROR:  Invalid option '" + arg + "'.")
		rc = -1
	return rc


def getFileWithReplacedData(filename):
	global serviceName
	data = ""		
	with open(filename, 'r') as infile:
		data = infile.read()
	data = data.replace("**abbrProductName**", abbrProductName)
	data = data.replace("**fullProductName**", fullProductName)
	data = data.replace("**cliName**", cliName)
	data = data.replace("**dataPath**", dataPath)
	data = data.replace("**serviceName**", serviceName)
	data = data.replace("**installPath**", installPath)
	return data
