# cducky

## Overview

A [Ducky USB](https://hakshop.com/products/usb-rubber-ducky-deluxe "Hak5 Ducky") encoder written in C. It takes as input a Ducky Script text file and produces a binary file usable by the Ducky.

## Installation
	git clone https://github.com/markoneill/cducky
	cd cducky && make
 
 ## Running
  
 Parameters: -i for input file path, -o for output file path, -l for keyboard layout file path
 
	./cducky -i myscript.txt
	./cducky -i myscript.txt -o output.bin
	./cducky -i myscript.txt -o inject.bin -l languages/us.txt
