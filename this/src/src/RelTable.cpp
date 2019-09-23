// File: RelTable.cpp
// Created by: Xerox
// Date: 06.05.2018
// Last modified: 01.06.2018.

#include <string.h>
#include <fstream>

#include "RelTable.h"
#include "SymbolTable.h"

char* itoa(unsigned long num, char* str, int base);

RelTable::RelTable() {
	for(int i = 0; i < MAX_NUM_OF_SECTION; i++)
		this->tableHead[i] = this->tableTail[i] = nullptr;

	for (int i = 0; i < MAX_NUM_OF_SECTION; i++)
		this->isFull[i] = false;
}

RelTable::~RelTable() {
	TableRow* oldRow = nullptr;

	for (int i = 0; i < MAX_NUM_OF_SECTION; i++) {
		TableRow* curRow = tableHead[i];

		while (curRow != nullptr) {
			oldRow = curRow;
			curRow = curRow->next;
			delete oldRow;
		}
	}

	for (int i = 0; i < MAX_NUM_OF_SECTION; i++)
		this->tableHead[i] = this->tableTail[i] = nullptr;
}

void RelTable::addRelRecord(OFFSET offset, REL_TYPE rtype, VALUE value, SECTION section) {
	if (!this->tableHead[section]) {
		this->tableHead[section] = this->tableTail[section] = new TableRow(offset, rtype, value);
	}
	else {
		this->tableTail[section] = this->tableTail[section]->next = new TableRow(offset, rtype, value);
	}
	this->isFull[section] = true;
}

void RelTable::updateRelTable(SymbolTable* curTableSym, SymbolTable* globalSymTable, std::string& sectionContent, SECTION section) {
	for (TableRow* curRow = tableHead[section]; curRow != nullptr; curRow = curRow->next) {
		if (curTableSym->isUndefinedSymbol(curRow->value)) {
			VALUE newValue = curTableSym->getSymbolValue(curRow->value, globalSymTable);
			updateRelRecord(curRow->offset, newValue, sectionContent, curRow->rtype);
		}
	}
}

void RelTable::printTables(std::ofstream& outputFile) {
	for (int i = 0; i < MAX_NUM_OF_SECTION; i++) {
		if (!isFull[i]) continue;
		outputFile << "#rel" << sections[i] << std::endl;
		outputFile << "#Off" << "\t" << "Tip" << "\t" << "Vre" << "\t" << std::endl;
		for (TableRow *cur = tableHead[i]; cur != 0; cur = cur->next)
			outputFile << cur->offset << "\t" << cur->rtype << "\t" << cur->value << "\t" << std::endl;
	}
}

void RelTable::updateRelRecord(OFFSET offset, VALUE value, std::string & sectionContent, REL_TYPE relType) {
	int newValue = 0;
	char* num = new char[5];
	offset <<= 1;
	if (relType == R_386_32)
		newValue = value;
	else {
		std::string s1 = sectionContent.substr(offset, 4);
		num[0] = s1[2];
		num[1] = s1[3];
		num[2] = s1[0];
		num[3] = s1[1];
		num[4] = '\0';
		long valTemp = strtol(num, 0, 16);
		if (valTemp > 0x7fff) {
			valTemp -= 0x10000;
			valTemp = value + valTemp; // valTemp is negative number. So this will actualy be a substraction.
		} else
			valTemp = value - valTemp;
		valTemp &= ~(~0L << 16);
		newValue = valTemp;
	}


	num = itoa(newValue, num, 16);
	std::string str(num);

	switch (str.size()) {
	case 1:
		str = "000" + str;
		break;
	case 2:
		str = "00" + str;
		break;
	case 3:
		str = "0" + str;
		break;
	}

	sectionContent[offset] = str[2];
	sectionContent[offset + 1] = str[3];
	sectionContent[offset + 2] = str[0];
	sectionContent[offset + 3] = str[1];
	delete num;
}
