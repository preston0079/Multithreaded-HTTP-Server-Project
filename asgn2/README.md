## Assignment 2 HTTP Server - Preston Nguyen
prhunguy

This directory contains source code and other files for Assignment 2.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

This program implements an HTTP server that executes the commands GET and PUT on the files within the directory.

## Usage
In terminal, input ./httpserver along with the port (between 1 - 65535).
Follow this by entering either GET or PUT along with the necessary details:
Request-Line\r\n     ( Method [a-zA-Z] URI [a-zA-Z0-9.-] Version [HTTP/#.#] )
(Header-Field\r\n)   ( key: value [a-zA-Z0-9.-] \r\n )
\r\n
Message-Body         ( for PUT only: Content-Length: # of bytes for message\r\n\r insert message to put here )

Example: PUT /hello.txt HTTP/1.1\r\nContent-Length: 12\r\n\r\nHello world!

## httpserver.c
Implementation of http server. Commands available are GET and PUT.

## Makefile
Compiles the files that are part of this assignment and make them executable. 

## Comments

