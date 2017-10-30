# cducky

## Overview

A [Ducky USB](https://hakshop.com/products/usb-rubber-ducky-deluxe "Hak5 Ducky") encoder written in C. It takes as input a [Ducky Script](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Duckyscript "Ducky Script reference") text file and produces a binary file usable by the Ducky.

## Installation
	git clone https://github.com/markoneill/cducky
	cd cducky && make
 
## Running
  
Parameters: -i for input file path, -o for output file path, -l for keyboard layout file path

Examples:

	./cducky -i myscript.txt
	./cducky -i myscript.txt -o output.bin
	./cducky -i myscript.txt -o inject.bin -l languages/us.txt

## Exploration

You can find various [payload examples](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Payloads "Ducky Payloads") or make your own using [Ducky Script](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Duckyscript "Ducky Script reference")

## Additional Notes

cducky supports all of the standard Ducky Script commands and also supports STRING_DELAY (see example below), which will delay individual characters in a string. It's keyboard layout files are are sufficient to support all keys, including NUMLOCK, BACKSPACE, and others unsupported by many other encoders.

## Example Script

	DELAY 3000
	GUI r
	DELAY 500
	STRING notepad
	DELAY 500
	ENTER
	DELAY 750
	STRING_DELAY 500 Hello World!
	BACKSPACE
	REPEAT 12
