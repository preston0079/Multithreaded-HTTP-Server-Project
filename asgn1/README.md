## Assignment 1 Command-line Memory - Preston Nguyen
prhunguy

This directory contains source code and other files for Assignment 1.

Use this README document to store notes about design, testing, and questions you have while developing your assignment.

This program will allow the user to manipule the txt files within its directory by either showing the text that the file holds in the terminal or by changing the text in the file from within the terminal.

## Usage
Input into the terminal "make memory" to execute the Makefile and make the memory.c code executable.
Then input ./memory to start the program.
Input “get\n<'location'>\n” to have memory.c print the text within the txt file.
Input "set\n<'location'>\n<'content_length'>\n<'contents>'” to have memory.c set/change the text within the txt file into the contents that are set by the user. If the name is unknown then a new file will be made with the contents provided.

./memory

get

"INSERT FILE NAME HERE"

Will output results

./memory

set

"INSERT FILE NAME HERE"

"INSERT LENGTH OF NEW CONTENT"

"INSERT CONTENT"

OK

Will output "OK" if successful

## memory.c
Implements get/set memory abstraction for files in a Linux directory. 

## Makefile
Compiles the files that are part of this assignment and make them executable.

## Comments

