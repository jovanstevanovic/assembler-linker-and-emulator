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

Linker::Linker(const char outputFileName[], const char * filesName[], int numOfObjFiles) {
	this->outputFileName = (char*)outputFileName;

	this->assmArr = new Assembler*[numOfObjFiles];
	for (int i = 0; i < numOfObjFiles; i++)
		assmArr[i] = new Assembler(filesName[i], 100);

	this->numOfObjFiles = numOfObjFiles;
	this->locCounter = 0;
	this->indexOfBeginTextSection = -1; 

	this->symTable = new SymbolTable(true);
	this->relTable = new RelTable();
}

Linker::~Linker() {
	for (int i = 0; i < numOfObjFiles; i++)
		delete assmArr[i];
	assmArr = nullptr;

	delete symTable;
	symTable = nullptr;

	delete relTable;
	relTable = nullptr;
}

bool Linker::execute() {

	/*for (int i = 0; i < numOfObjFiles; i++) {
		bool succ = assmArr[i]->execute();
		if (!succ)
			return false;
	}

	bool visited[MAX_NUM_OF_SECTION];
	for (int i = 0; i < MAX_NUM_OF_SECTION; i++)
		visited[i] = false;
	visited[0] = true;

	for (int i = 0; i < numOfObjFiles; i++) {
		for (SymbolTable::TableRow* curRow = assmArr[i]->symTable->sectionTHead; curRow != assmArr[i]->symTable->symbolTHead; curRow = curRow->next) {
			if (visited[curRow->sec_num]) continue;
			visited[curRow->sec_num] = true;
			SIZE curSectionSize = 0;

			for (int j = i; j < numOfObjFiles; j++) {
				SIZE sectionSize = assmArr[j]->symTable->updateSymbolTable(curRow->sec_num, locCounter, curSectionSize);
				curSectionSize += sectionSize;
			}
			symTable->addSection(curRow->name, curRow->sec_num, curRow->attr, locCounter, curSectionSize);

			for (int i = 0; i < numOfObjFiles; i++)
				sectionsContextGlb = sectionsContextGlb.append(assmArr[i]->sectionContent[curRow->sec_num]);

			locCounter += curSectionSize;
		}
	}

	for (int i = 0; i < numOfObjFiles; i++) {
		bool succ = assmArr[i]->symTable->fillWithDefinedSymbols(symTable);
		if (!succ)
			return false;
	}

	for (int i = 0; i < numOfObjFiles; i++) {
		bool succ = assmArr[i]->symTable->checkIfAllSymbolAreDefined(symTable);
		if (!succ)
			return false;
	}

	for (int i = 0; i < MAX_NUM_OF_SECTION; i++)
		visited[i] = false;
	visited[0] = true;
	locCounter = 0;
	symTable->mergeTables();*/

	for (int i = 0; i < numOfObjFiles; i++) {
		bool succ = Assembler::listOfSymTables[i]->fillWithDefinedSymbols(symTable);
		if (!succ)
			return false;
	}

	VALUE startSymbolValue = symTable->getSymbolValue(std::string("_start"));
	if (startSymbolValue == UNDEFINED_VALUE) {
		std::cout << "\n**** SIMBOL _start NIJE DEFINISAN! ****\n" << std::endl;
		return false;
	}
	indexOfBeginTextSection = getPCValue(startSymbolValue);

	/*for (int i = 0; i < numOfObjFiles; i++) {
		for (SymbolTable::TableRow* curRow = assmArr[i]->symTable->sectionTHead; curRow != assmArr[i]->symTable->symbolTHead; curRow = curRow->next) {
			if (visited[curRow->sec_num]) continue;
			visited[curRow->sec_num] = true;
			SIZE curSectionSize = 0;

			for (int j = i; j < numOfObjFiles; j++) {
				assmArr[j]->relTable->updateRelTable(curRow->sec_num, locCounter + curSectionSize, symTable, assmArr[j]->symTable,
					sectionsContextGlb);

				SIZE sectionSize = assmArr[j]->symTable->getSectionSize(curRow->sec_num);
				if(sectionSize != UNDEFINED_VALUE)
					curSectionSize += sectionSize;
			}
			locCounter += curSectionSize;
		}
	}*/

	for (int i = 0; i < numOfObjFiles; i++) {
		for (int j = 1; j < MAX_NUM_OF_SECTION; j++)
			Assembler::listOfRelTables[i]->updateRelTable2(Assembler::listOfSymTables[i], symTable, *Assembler::listOfSecCont[i + j - 1], (SECTION)j);
	}

	/*for (int i = 0; i < numOfObjFiles; i++)
			assmArr[i]->relTable->mergeWithExistingRelTables(relTable);*/

	std::ofstream outputFile;
	outputFile.open(this->outputFileName);
	if (!outputFile.is_open())
		return false;

	for (int i = 0; i < numOfObjFiles; i++) {
		Assembler::listOfSymTables[i]->printTables(outputFile);
		outputFile << std::endl;
	}

	for (int i = 0; i < numOfObjFiles; i++)
		Assembler::listOfRelTables[i]->printTables(outputFile);

	for (int i = 0; i < numOfObjFiles; i++) {
		for (int j = 1; j < MAX_NUM_OF_SECTION; j++)
			outputFile << Assembler::listOfSecCont[i + j - 1];

		/*this->printSectionsContent(outputFile);*/
		outputFile.close();

		return true;
	}
}

//void Linker::printsectionscontent(std::ofstream & outputfile) {
//	outputfile << "\n#objektni fajl" << "\n";
//	outputfile << sectionscontextglb << "\n";
//}

long Linker::getPCValue(OFFSET offset) {
	int iterator = 0;

	offset *= 2;
	while (offset > 0) {
		if (sectionsContextGlb[iterator++] == ' ') continue;
		offset -= 1;
	}

	if (sectionsContextGlb[iterator] == ' ')
		iterator++;
	return iterator;
}
