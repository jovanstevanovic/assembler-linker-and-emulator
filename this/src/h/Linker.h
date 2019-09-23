#ifndef _LINKER_H_
#define _LINKER_H_

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

	// Constructors
	Linker();

	// Util methods
	bool execute();

private:

	friend class Emulator;

	SymbolTable* symTable;
	RelTable* relTable;

	int indexOfBeginTextSection;
};

#endif
