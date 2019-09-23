#pragma once
// File: RelTable.h
// Created by: Xerox
// Date: 06.05.2018
// Last modified: 30.05.2018.

#include "Macros.h"

class Linker;
class SymbolTable;

class RelTable {
public:

	// Constructors
	RelTable();

	// Destructor
	~RelTable();

	// Add methods
	void addRelRecord(OFFSET offset, REL_TYPE rtype, VALUE value, SECTION section);

	// Update methods
	void updateRelTable(SECTION section, SIZE curSectionPosition, SymbolTable* globalSymTable, SymbolTable* proModelSymTable
		, std::string& sectionContent);
	void updateRelTable2(SymbolTable* curTableSym, SymbolTable* globalSymTable, std::string& sectionContent, SECTION section);

	// Utility methods
	void printTables(std::ofstream& outputFile);
	void mergeWithExistingRelTables(RelTable* relTable);
	void updateRelRecord(OFFSET offset, VALUE value, std::string& sectionContent, REL_TYPE relType);

private:

	friend class Linker;

	struct TableRow {
		OFFSET offset;
		REL_TYPE rtype;
		VALUE value;
		TableRow* next;

		TableRow(OFFSET offset, REL_TYPE rtype, VALUE value) : next(0) {
			this->offset = offset;
			this->rtype = rtype;
			this->value = value;
		}
	};
	
	char* sections[5] = { ".und", ".text", ".data", ".bss", ".rodata" };

	TableRow *tableHead[MAX_NUM_OF_SECTION], *tableTail[MAX_NUM_OF_SECTION];
	bool isFull[MAX_NUM_OF_SECTION];
};
