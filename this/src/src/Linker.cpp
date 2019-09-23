// File: Linker.cpp
// Created by: Xerox
// Date: 23.05.2018
// Last modified: 01.06.2018.

#include <fstream>
#include <iostream>
#include <string.h>
#include <array>

#include "Linker.h"
#include "Assembler.h"
#include "SymbolTable.h"
#include "RelTable.h"

Linker::Linker() {

	this->symTable = new SymbolTable();
}

bool Linker::execute() {

	for (int i = 0; i < Assembler::curNumber; i++) {
		bool succ = Assembler::listOfSymTables[i]->fillWithDefinedSymbols(symTable);
		if (!succ)
			return false;
	}

	for (int i = 0; i < Assembler::curNumber; i++) {
		bool succ = Assembler::listOfSymTables[i]->checkIfAllSymbolAreDefined(symTable);
		if (!succ)
			return false;
	}

	symTable->tablePointer = symTable->symbolTHead;

	VALUE startSymbolValue = symTable->getSymbolValue("_start");
	if (startSymbolValue == (unsigned long)UNDEFINED_VALUE) {
		std::cout << "\n**** SIMBOL _start NIJE DEFINISAN! ****\n" << std::endl;
		return false;
	}

	indexOfBeginTextSection = startSymbolValue;

	for (int i = 0; i < Assembler::curNumber; i++) {
		for (int j = 1; j < MAX_NUM_OF_SECTION; j++)
			Assembler::listOfRelTables[i]->updateRelTable(Assembler::listOfSymTables[i], symTable,
				*Assembler::listOfSecCont[i * (MAX_NUM_OF_SECTION - 1) + j - 1], (SECTION)j);
	}
	return true;
}
