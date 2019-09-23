#pragma once
// File: SymbolTable.h
// Created by: Xerox
// Date: 06.05.2018
// Last modified: 01.06.2018.

#include "Macros.h"

class Linker;
class Assembler;
class Emulator;
class RelTable;

class SymbolTable {
public:

	// Constructor
	SymbolTable(bool preparingForExec = false);

	// Destructor.
	~SymbolTable();

	// Add methods
	bool addSection(SYM_NAME name, SECTION sec, ATTR attr, VALUE value = 0, SIZE size = 0);
	bool addSymbol(SYM_NAME name, SECTION sec, VALUE value, VISIBILITY vis, SIZE size, ATTR attr);

	// Get methods
	SER_NUM getSerialNumber(SYM_NAME name);
	VALUE getSymbolValue(SYM_NAME name);
	VALUE getSymbolValue(SER_NUM sn);
	SER_NUM getSecSerialNumberOfSymbol(SYM_NAME name);
	SIZE getSectionSize(SECTION section);
	SER_NUM SymbolTable::getSerialNumber(SER_NUM sn, SymbolTable* globalSymbolTable);
	SECTION getSectionBySerialNumber(SER_NUM sn);
	long getObjFileSize();

	// Find methods
	bool isSymbolExists(SYM_NAME name);
	bool checkIfAllSymbolAreDefined(SymbolTable* symTable);
	bool isUndefinedSymbol(SER_NUM sn);
	bool isUndefinedSymbol(SYM_NAME name);
	bool isGlobal(SYM_NAME name);

	// Update methods
	SIZE updateSymbolTable(SECTION section, long locCounter, SIZE sectionsSize);

	// Utility methods
	void mergeTables();
	void printTables(std::ofstream& outputFile);
	bool setSectionSize(SECTION section, SIZE size);
	bool setGlobalVisibility(SYM_NAME name);
	bool fillWithDefinedSymbols(SymbolTable* symTable);

private:

	friend class Linker;
	friend class Assembler;
	friend class Emulator;
	friend class RelTable;

	SER_NUM_SEED seed;
	
	struct TableRow {
		SER_NUM sn;
		SYM_NAME name;
		SECTION sec_num;
		VALUE value;
		VISIBILITY vis;
		SIZE size;
		ATTR attr;
		TableRow* next;

		TableRow(SYM_NAME name, SECTION sec_num, VALUE value, VISIBILITY vis, SIZE size, ATTR attr, SER_NUM sn) : attr(attr), next(nullptr) {
			this->sn = sn;
			this->name = name;
			this->sec_num = sec_num;
			this->value = value;
			this->vis = vis;
			this->size = size;
		}
	};
	
	bool preparingForExec;

	long allSectionsSize;
	long startAddress;

	TableRow *sectionTHead, *sectionTTail;
	TableRow *symbolTHead, *symbolTTail;
	TableRow *tablePointer;
};

