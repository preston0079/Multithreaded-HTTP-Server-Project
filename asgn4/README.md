## Assignment 4 directory

prhunguy

This directory contains source code and other files for Assignment 4.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

This program implements a multithreaded HTTP server. It executes the commands GET and PUT on the files within the directory. This server will process multiple clients simultaneously and ensure coherency and atomicity.

## Usage

Input: "make" into terminal to compile files.

./httpserver [-t threads] <port>

theads is optional (default 4)

Implementation is similar to asgn2:

Terminal Example: 

GET /a.txt HTTP/1.1\r\nRequest-Id: 1\r\n\r\n

GET /b.txt HTTP/1.1\r\nRequest-Id: 2\r\n\r\n

PUT /b.txt HTTP/1.1\r\nRequest-Id: 3\r\nContent-Length: 3 \r\n\r\nbye

GET /b.txt HTTP/1.1\r\n\r\n


Audit Log will reflect Example:

GET,/a.txt,200,1

GET,/b.txt,404,2

PUT,/b.txt,201,3

GET,/b.txt,200,0


## httpserver.c

Implementation of http server.

## Makefile

Compiles the files that are part of this assignment and make them executable.

## Comments

Starter code and TA pseudocode used.
