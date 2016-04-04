#!/usr/bin/env python

import subprocess
import platform
import os
import time
import sys
import manpage_helper

usageString = "Usage: create_ixpdimm-cli_manpage.py " + manpage_helper.usage

#################################################################
# Run the ixpdimm-cli command, and pipe the output to a file
#################################################################
def captureHelpOutput():
	# run ixpdimm-cli and save help output to a file	
	currpath = os.getcwd()
	os.chdir(manpage_helper.outputPath)
	os.environ['LD_LIBRARY_PATH'] = '.'
	oscmd = "./ixpdimm-cli > " + currpath + "/" + helpTempFile
	rc = os.system(oscmd)
	os.chdir(currpath)
	return rc
	
#################################################################
# Read help output, and put each command into a 2 column list,
# first entry is the command name, second entry is the syntax
#################################################################
def createCommandList():
	# create command list to format
	linenum = 0
	commandlist = []
	numCommands = -1
	with open(helpTempFile, 'r') as file:
		for line in file:
			# strip starting spaces of each line
			line = line.lstrip()
			line = line.replace('\n', '')
			if (line and
				not line.isspace() and
				"Commands:" not in line and
				"Usage:" not in line):
				if (line and
					":" in line and
					"dd:yyyy" not in line):
						numCommands += 1
						commandlist.append([line, ""])
				else:
					commandlist[numCommands][1] += line	
	return commandlist;
	
#################################################################
# Add spaces in Bracketed word so it is split into words ready
# to format alternating between bold and non-bold
#################################################################
def prepareBracketWordForFormatting(string):
	newstring = ""
	spaceAroundValuesForBold = 0
	if "|" in string and "GB|GiB" not in string:
		spaceAroundValuesForBold = 1
	for char in string:
		if char == '[':
			if string.startswith("[("):
				newstring += char
			else:
				newstring += char + ' '
		elif char == '|' and spaceAroundValuesForBold:
			newstring += ' ' + char + ' '
		elif char == ']':
			# if we have a ")]", no space needed to format
			if newstring.endswith(')') or newstring.endswith(']'):
				newstring += char  
			else:
				newstring += ' ' + char
		elif char == '(':
			if string.startswith("[(") or string.startswith('('):
				if spaceAroundValuesForBold:
					newstring += char + ' '
				else:
					newstring += char
			else:
				if spaceAroundValuesForBold:
					newstring += ' ' + char + ' '
				else:
					newstring += ' ' + char + ' '
		elif char == ')':
			if spaceAroundValuesForBold:
				newstring += ' ' + char
			else:
				newstring += char
		elif char == '=':
			newstring += ' ' + char
		else:
			newstring += char 
	return newstring
	
#################################################################
# Add spaces in key pairs so they are ready for formatting
# Ends up looking something like:  [ capacity = (capacity))]
# Ready for formatting alternating bold and non-bold per word
#################################################################
def addSpacesForKeyPair(string):
	newstring = ""
	if "=" not in string:
		return string
	else:
		equaloffset = 0;
		for i in range(len(string)):
			char = string[i]
			if char == "=":
				newstring += ' ' + char + " "
				equaloffset = i
			elif char == "[":
				if (i > equaloffset and
					string[i + 1] == '('):
						newstring += char
				else:
					newstring += char + " "
			elif char == "]":
				if newstring.endswith(")"):
					newstring += char
				else:
					newstring += " " + char
			else:
				newstring += char	
	return newstring  
	
#################################################################
# Convert the text list of commands into nroff ready formated
# list
#################################################################
def formatCommands(commandlist):
	manpagecommandlist = []
	# for each command, format for man page		
	for i in range(len(commandlist)):
		# set command name as section header on man page
		manpagecommand = "\n.B " + commandlist[i][0] + "\n.RS\n"
		
		# split the line into words so we can tackle one word at a time
		# cmd[0] ends up being the verb, and cmd[1] is a space,
		# and cmd[2] is the rest of the command
		cmd = commandlist[i][1].partition(" ")
	
		# format the command syntax, first word being the bolded verb
		manpagecommand += ".B " + cmd[0]
		
		# remove spaces before & after '=' so we can parse the value/pair
		# setting together later
		remaining = cmd[2].replace(" = ", "=")
		
		# walk through remaining command parts in a loop to format each word
		remaining = remaining.split(' ')
		
		for part in remaining:
			# if it starts with a '-', it is something the user types in, so bold	
			if part.startswith("-"):
				manpagecommand += "\n.B " + part
				
			# if word contains '=', then bold before & including '=' 
			# but not after
			elif "=" in part:
				# contains a key/pair, add spaces where appropriate 
				# so can be split for formatting
				keyPairWithSpaces = addSpacesForKeyPair(part);
				keyPairSplit = keyPairWithSpaces.split()
				keysplitoffset = 0;
				
				if keyPairSplit[keysplitoffset].startswith("["):
					# starts with a bracket, so add bracket and key
					manpagecommand += "\n.RB " + keyPairSplit[keysplitoffset] + "  " \
						+ keyPairSplit[keysplitoffset + 1]
					keysplitoffset += 2
					
				else:
					# no brackets, just add key
					manpagecommand += "\n.B " +  keyPairSplit[keysplitoffset]
					keysplitoffset += 1
					
				# add equal sign
				manpagecommand += "\n.B " + keyPairSplit[keysplitoffset]
				keysplitoffset += 1
			
				# key pair is list of options - so bold options, but not brackets or '|'
				if ("|" in keyPairSplit[keysplitoffset] 
					or keyPairSplit[keysplitoffset].startswith("(")):
					BracketWordWithSpaces = prepareBracketWordForFormatting(keyPairSplit[keysplitoffset]);
					manpagecommand += "\n.RB " + BracketWordWithSpaces
					
				# no choices, so just add option, bolded if not in parens
				else:
					if '(' in keyPairSplit[keysplitoffset]:
							manpagecommand += "\n" + keyPairSplit[keysplitoffset]
					else:
						manpagecommand += "\n.B " + keyPairSplit[keysplitoffset]
					
			# if it starts with brackets, need to separate with spaces so can 
			# use .B or .RB formatting as appropriate
			elif part.startswith("[") or part.startswith("("):
				BracketWordWithSpaces = prepareBracketWordForFormatting(part);
				manpagecommand += "\n.RB " + BracketWordWithSpaces

			# otherwise, just add it
			else:
				manpagecommand = manpagecommand + part

		manpagecommand += "\n.RE\n"
		manpagecommandlist.append(manpagecommand)
		manpagecommand = ""	
	return manpagecommandlist

#################################################################
# Start here
#################################################################

# Must be running on Linux
if "Linux" not in platform.system():
	exit("ERROR:  Man page creation must be done on Linux.")
	
# Header and footer files must exist
headerFileName = "ixpdimm-cli.manpage.header"

if not os.path.exists(headerFileName):
	exit("ERROR:  Missing header file '" + headerFileName + "' to create man page.")
footerFileName = "ixpdimm-cli.manpage.footer"
if not os.path.exists(footerFileName):
	exit("ERROR:  Missing footer file '" + footerFileName + "' to create man page.")

#create temporary files
helpTempFile = "helpout.tmp"
tempTextFileName = "ixpdimm-cliMan.tmp"

# names from command line inputs override defaults
for i in range(len(sys.argv)):
	if i > 0:
		if manpage_helper.setProductName(sys.argv[i]) < 0:
			exit(usageString)

#add title to man page
title = manpage_helper.cliName + " 8 \"" + time.strftime("%B %d, %Y") + "\" \"\" \"" + manpage_helper.fullProductName + "\""
with open(tempTextFileName, 'w') as manfile:
	manfile.write(title)

# add header info to man page
with open(tempTextFileName, 'a') as manfile:
	manfile.write(manpage_helper.getFileWithReplacedData(headerFileName));

if (0 != captureHelpOutput()):
	exit("ERROR:  Unable to capture help output for man page creation.")

commandlist = createCommandList()

# We now have a list of command parts, first part is command name for section header,
# second part is the command syntax to parse for formatting			

manpagecommandlist = formatCommands(commandlist)

# write formatted commands to man page file
with open(tempTextFileName, 'a') as manfile:
	for cmd in range(len(manpagecommandlist)):
		manfile.write(manpagecommandlist[cmd])

	# add rest of man page info to file
	# with open(footerFileName, 'r') as infile:
	manfile.write(manpage_helper.getFileWithReplacedData(footerFileName));

# Create formatted man page from text file
os.system('sed -f ixpdimm-cli.sed ' + tempTextFileName + ' > temp.txt')
os.system('nroff -e -mandoc temp.txt 1>/dev/null')
os.system('mv temp.txt ' + manpage_helper.cliName + '.8')
os.system('rm -f ' + manpage_helper.cliName + '.8.gz')
os.system('gzip ' + manpage_helper.cliName + '.8')

#delete temp files
os.system('rm ' + tempTextFileName)
os.system('rm ' + helpTempFile)

# View man page by doing a 'man ./ixpdimm-cli.8.gz'
# Installation needs to put this ixpdimm-cli.8.gz file in the man8 subdirectory in the 
#    system's man path
