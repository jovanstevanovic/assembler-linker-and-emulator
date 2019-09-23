// File: SymbolTable.cpp
// Created by: Xerox
// Date: 06.05.2018
// Last modified: 01.06.2018.

#include <string.h>
#include <fstream>
#include <iostream>

#include "SymbolTable.h"
#include "Assembler.h"
#include "Emulator.h"

SymbolTable::SymbolTable(bool preparingForExec) {
	this->seed = 0;

	if(!preparingForExec)
		this->tablePointer = new TableRow("", UND, 0, LOCAL, 0, NO_ATTR, seed++);

	this->preparingForExec = preparingForExec;
	this->sectionTHead = this->sectionTTail = 0;
	this->symbolTHead = this->symbolTTail = 0;
	this->allSectionsSize = 0;
	this->startAddress = 0;
}

SymbolTable::~SymbolTable() {
	TableRow* oldRow = nullptr;
	TableRow* curRow = tablePointer;

	while (curRow != nullptr) {
		oldRow = curRow;
		curRow = curRow->next;
		delete oldRow;
	}

	tablePointer = nullptr;
}

bool SymbolTable::addSection(SYM_NAME name, SECTION sec, ATTR attr, VALUE value, SIZE size) {
	if (this->isSymbolExists(name))
		return false;

	if (!this->sectionTHead) {
		this->sectionTHead = this->sectionTTail = new TableRow(name, sec, value, LOCAL, size, attr, seed++);
	}
	else {
		this->sectionTTail = this->sectionTTail->next = new TableRow(name, sec, value, LOCAL, size, attr, seed++);
	}
	return true;
}

bool SymbolTable::addSymbol(SYM_NAME name, SECTION sec, VALUE value, VISIBILITY vis, SIZE size, ATTR attr) {
	if (this->isSymbolExists(name))
		return false;

	if (!this->symbolTHead) {
		this->symbolTHead = this->symbolTTail = new TableRow(name, sec, value, vis, size, attr, seed++);
	}
	else {
		this->symbolTTail = this->symbolTTail->next = new TableRow(name, sec, value, vis, size, attr, seed++);
	}
	return true;
}

SER_NUM SymbolTable::getSerialNumber(SYM_NAME name) {
	for (TableRow *cur = tablePointer; cur != 0; cur = cur->next)	{
		if (!name.compare(cur->name))
			return cur->sn;
	}
	return UNDEFINED_VALUE;
}

VALUE SymbolTable::getSymbolValue(SYM_NAME name) {
	for (TableRow *cur = tablePointer; cur != nullptr; cur = cur->next) {
		if (!name.compare(cur->name))
			return cur->value;
	}
	return UNDEFINED_VALUE;
}

VALUE SymbolTable::getSymbolValue(SER_NUM sn) {
	for (TableRow *cur = tablePointer; cur != nullptr; cur = cur->next) {
		if (cur->sn == sn)
			return cur->value;
	}
	return UNDEFINED_VALUE;
}

VALUE SymbolTable::getSymbolValue(SER_NUM sn, SymbolTable * st) {
	for (TableRow *cur = tablePointer; cur != nullptr; cur = cur->next) {
		if (cur->sn == sn) {
			return st->getSymbolValue(cur->name);
		}
	}
	return UNDEFINED_VALUE;
}

bool SymbolTable::isUndefinedSymbol(SER_NUM sn) {
	for (TableRow *cur = tablePointer; cur != nullptr; cur = cur->next) {
		if (cur->sn == sn && cur->sec_num == UND)
			return true;
	}
	return false;
}

bool SymbolTable::isUndefinedSymbol(SYM_NAME name) {
	for (TableRow *cur = tablePointer; cur != 0; cur = cur->next) {
		if (!name.compare(cur->name) && cur->sec_num == UND)
			return true;
	}
	return false;
}

bool SymbolTable::isGlobal(SYM_NAME name) {
	for (TableRow *cur = tablePointer; cur != 0; cur = cur->next) {
		if (!name.compare(cur->name)) {
			if (cur->vis == GLOBAL)
				return true;
			else
				return false;
		}
	}
	return false;
}

SER_NUM SymbolTable::getSecSerialNumberOfSymbol(SYM_NAME name) {
	for (TableRow *cur = tablePointer; cur != 0; cur = cur->next) {
		if (!name.compare(cur->name)) {
			SECTION section = cur->sec_num;
			for (TableRow* curR = tablePointer; curR != 0; curR = curR->next) {
				if (curR->sec_num == section)
					return curR->sn;
			}
		}
	}
	return UNDEFINED_VALUE;
}

bool SymbolTable::isSymbolExists(SYM_NAME name) {
	for (TableRow *cur = symbolTHead; cur != nullptr; cur = cur->next) {
		if (!name.compare(cur->name))
			return true;
	}
	for (TableRow *cur = sectionTHead; cur != nullptr; cur = cur->next) {
		if (!name.compare(cur->name))
			return true;
	}
	return false;
}

bool SymbolTable::checkIfAllSymbolAreDefined(SymbolTable * symTable) {
	for (TableRow* cur = symbolTHead; cur != nullptr; cur = cur->next) {
		if (cur->sec_num == UND) {
			bool succ = symTable->isSymbolExists(cur->name);
			if (!succ) {
				std::cout << "\n**** GLOBALNI SIMBOL " << cur->name << " NIJE DEFINISAN! ****\n" << std::endl;
				return false;
			}
		}
	}
	return true;
}

void SymbolTable::mergeTables() {
	if(this->sectionTTail && this->symbolTHead)
		this->sectionTTail->next = this->symbolTHead;
	if(!this->preparingForExec)
		this->tablePointer->next = this->sectionTHead;
	else
		this->tablePointer = this->sectionTHead;
}

void SymbolTable::printTables(std::ofstream& outputFile) {
	outputFile << "\n#RedBr." << "\t" << "Sim" << "\t" << "Sek" << "\t" << "Vre" << "\t" << "Vid" << "\t"
		<< "Vel" << "\t" << "Atr" << std::endl;
	for (TableRow *cur = tablePointer; cur != 0; cur = cur->next) {
		outputFile << cur->sn << "\t" << cur->name << "\t" << cur->sec_num << "\t" << cur->value << "\t" << cur->vis << "\t"
			<< cur->size << "\t" << cur->attr << std::endl;
	}
}

bool SymbolTable::setSectionSize(SECTION section, SIZE size) {
	for (TableRow *cur = sectionTHead; cur != 0; cur = cur->next) {
		if (cur->sec_num == section) {
			cur->size = size;
			this->allSectionsSize += size;
			return true;
		}
	}
	return false;
}

bool SymbolTable::setGlobalVisibility(SYM_NAME name) {
	for (TableRow *cur = tablePointer; cur != nullptr; cur = cur->next) {
		if (!name.compare(cur->name)) {
			cur->vis = GLOBAL;
			return true;
		}
	}
	return false;
}

bool SymbolTable::fillWithDefinedSymbols(SymbolTable * symTable) {
	for (TableRow *cur = symbolTHead; cur != nullptr; cur = cur->next) {
		if (cur->sec_num != UND && cur->vis == GLOBAL) {
			bool succ = symTable->addSymbol(cur->name, cur->sec_num, cur->value, cur->vis, cur->size, cur->attr);
			if (!succ) {
				std::cout << "\n**** GLOBALNI SIMBOL " << cur->name << " JE VEC DEFINISAN! ****\n" << std::endl;
				return false;
			}
		}
	}
	return true;
}
