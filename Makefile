CC = g++ # compiler on linux

all: SerialMessageParser.cpp # requires .cpp file
	@$(CC) SerialMessageParser.cpp -o SerialMessageParser HelperFunctions.h HelperFunctions.cpp # compiled C++ program is SerialMessageParser

clean:
	@rm -rf SerialMessageParser # clears the output