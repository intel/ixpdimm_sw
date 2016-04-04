#!/usr/bin/env python

import subprocess
import platform
import os
import time
import sys
import manpage_helper

usageString = "Usage: create_ixpdimm-monitor_manpage.py " + manpage_helper.usage

#################################################################
# Start here
#################################################################

# Must be running on Linux
if "Linux" not in platform.system():
	exit("ERROR:  Man page creation must be done on Linux.")
	
# Text file must exist
inputTextFileName = "ixpdimm-monitor.manpage.text"

if not os.path.exists(inputTextFileName):
	exit("ERROR:  Missing header file '" + inputTextFileName + "' to create man page.")

#create temporary text file to be converted to the formatted man page later
tempTextFileName = "ixpdimm-monitorMan.txt"

# names from command line inputs override defaults
for i in range(len(sys.argv)):
	if i > 0:
		if manpage_helper.setProductName(sys.argv[i]) < 0:
			exit(usageString)

# add title line
title = manpage_helper.serviceName + " 8 \"" + time.strftime("%B %d, %Y") + "\" \"\" \"" + manpage_helper.fullProductName + " Monitor\""
with open(tempTextFileName, 'w') as manfile:
	manfile.write(title)

# copy input text file, replacing product name in text
	manfile.write(manpage_helper.getFileWithReplacedData(inputTextFileName))

# Create formatted man page from text file
os.system('sed -f ixpdimm-cli.sed ' + tempTextFileName + ' > temp.txt')
os.system('nroff -e -mandoc temp.txt 1>/dev/null')
os.system('mv temp.txt ' + manpage_helper.serviceName + '.8')
os.system('rm -f ' + manpage_helper.serviceName + '.8.gz')
os.system('gzip ' + manpage_helper.serviceName + '.8')

#delete temp files
os.system('rm ' + tempTextFileName)

# View man page by doing a 'man ./ixpdimm-monitor.8.gz'
# Installation needs to put this ixpdimm-monitor.8.gz file in the man8 subdirectory in the 
#    system's man path
