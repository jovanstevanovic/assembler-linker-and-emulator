// File: RelTable.css
// Created by: Xerox
// Date: 06.05.2018
// Last modified: 30.05.2018.

#include <string.h>
#include <fstream>
#include <iostream>

#include "RelTable.h"
#include "SymbolTable.h"

using namespace std;

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

void RelTable::updateRelTable(SECTION section, SIZE curSectionPosition, SymbolTable* globalSymTable, SymbolTable* proModelSymTable
	, std::string& sectionContent) {

	for (TableRow* curRow = tableHead[section]; curRow != nullptr; curRow = curRow->next) {
		curRow->offset += curSectionPosition;
		curRow->value = proModelSymTable->getSerialNumber(curRow->value, globalSymTable);

		VALUE newValue = globalSymTable->getSymbolValue(curRow->value);
		updateRelRecord(curRow->offset, newValue, sectionContent, curRow->rtype);

		curRow->value = globalSymTable->getSectionBySerialNumber(curRow->value);
	}
}

void RelTable::updateRelTable2(SymbolTable* curTableSym, SymbolTable* globalSymTable, std::string& sectionContent, SECTION section) {
	
	for (TableRow* curRow = tableHead[section]; curRow != nullptr; curRow = curRow->next) {
		if (curTableSym->isUndefinedSymbol(curRow->value)) {
			VALUE newValue = globalSymTable->getSymbolValue(curRow->value);
			updateRelRecord(curRow->offset, newValue, sectionContent, curRow->rtype);
		}
	}
}

void RelTable::printTables(ofstream& outputFile) {
	for (int i = 0; i < MAX_NUM_OF_SECTION; i++) {
		if (!isFull[i]) continue;
		outputFile << "#rel" << sections[i] << endl;
		outputFile << "#Off" << "\t" << "Tip" << "\t" << "Vre" << "\t" << endl;
		for (TableRow *cur = tableHead[i]; cur != 0; cur = cur->next)
			outputFile << cur->offset << "\t" << cur->rtype << "\t" << cur->value << "\t" << endl;
	}
}

void RelTable::mergeWithExistingRelTables(RelTable* relTable) {
	for (int i = 0; i < MAX_NUM_OF_SECTION; i++) {
		if (!isFull[i]) continue;
		for (TableRow *cur = tableHead[i]; cur != 0; cur = cur->next)
			relTable->addRelRecord(cur->offset, cur->rtype, cur->value, (SECTION)i);
	}
}

void RelTable::updateRelRecord(OFFSET offset, VALUE value, std::string & sectionContent, REL_TYPE relType) {
	int iterator = 0;
	OFFSET offsetTemp = offset;

	offset *= 2;
	while (offset > 0) {
		if (sectionContent[iterator++] == ' ') continue;
		offset -= 1;
	}
	
	if (sectionContent[iterator] == ' ')
		iterator++;

	char* num = new char[5];
	int j = 0;
	for (int i = 0; i < 4; i++) 
		num[j++] = sectionContent[iterator + i];
	num[4] = '\0';

	int newValue = 0;
	if (relType == R_386_32)
		newValue = strtol(num, 0, 16) + value;
	else {
		int valTemp = value - strtol(num, 0, 16) - offsetTemp;
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
	default:
		break;
	}

	for (int i = 0; i < 4; i++)
		sectionContent[iterator + i] = str[i];
	/*cout << str << endl;*/
}
