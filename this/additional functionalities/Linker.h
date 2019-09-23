#pragma once
// File: Linker.h
// Created by: Xerox
// Date: 23.05.2018
// Last modified: 01.06.2018.

#include "Macros.h"

class Assembler;
class SymbolTable;
class RelTable;
class Emulator;

class Linker {
public:

	// Consutructors
	Linker(const char outputFileName[], const char* filesName[], int numOfObjFiles);

	// Destructor
	~Linker();

	// Util methods
	bool execute();
	/*void printSectionsContent(std::ofstream& outputFile);*/

private:

	friend class Emulator;
	
	long getPCValue(OFFSET offset);

	Assembler** assmArr;
	int numOfObjFiles;

	SymbolTable* symTable;
	RelTable* relTable;

	char* outputFileName;
	
	std::string sectionsContextGlb;

	int indexOfBeginTextSection;
	int indexOfEndTextSection;

	long locCounter;
};