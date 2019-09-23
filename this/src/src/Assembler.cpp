// File: Assembler.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 31.05.2018.

#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>

#include "Assembler.h"
#include "SymbolTable.h"
#include "RelTable.h"
#include "Parser.h"
#include "Result.h"
#include "SectionResult.h"
#include "OPResult.h"
#include "LabelResult.h"
#include "DirResult.h"
#include "EIResult.h"

std::array<SymbolTable*, MAX_NUM_OF_INPUT_FILES> Assembler::listOfSymTables;
std::array<RelTable*, MAX_NUM_OF_INPUT_FILES> Assembler::listOfRelTables;
std::array<std::string*, 4 * MAX_NUM_OF_INPUT_FILES> Assembler::listOfSecCont;
std::array<int, 4 * MAX_NUM_OF_INPUT_FILES> Assembler::orderOfListSections;
int Assembler::curNumber = 0;

void swap(char str[], int firstCharPos, int secondCharPos) {
	char temp = str[firstCharPos];
	str[firstCharPos] = str[secondCharPos];
	str[secondCharPos] = temp;
}

void reverse(char str[], int length) {
    int start = 0;
    int end = length -1;
    while (start < end) {
        swap(str, start, end);
        start++;
        end--;
    }
}

char* itoa(unsigned long num, char* str, int base) {
    int i = 0;
    bool isNegative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';

    reverse(str, i);

    return str;
}

Assembler::Assembler() {

}

Assembler::Assembler(const char* fileName, unsigned short startAddress) {
	strcpy(this->fileName, fileName);

	this->parser = new Parser();

	this->locCounter = startAddress;
	this->startAddress = startAddress;
}

Assembler::~Assembler() {
	delete parser;

	symTable = nullptr;
	relTable = nullptr;
	parser = nullptr;
}

bool Assembler::execute() {
	bool succ = firstPass();
	if (!succ)
		return false;

	succ = secondPass();
	if (!succ)
		return false;

	listOfSymTables[curNumber] = symTable;
	listOfRelTables[curNumber] = relTable;
	listOfSecCont[curNumber * 4] = &sectionContent[1];
	listOfSecCont[curNumber * 4 + 1] = &sectionContent[2];
	listOfSecCont[curNumber * 4 + 2] = &sectionContent[3];
	listOfSecCont[curNumber * 4 + 3] = &sectionContent[4];
	switch (numOfOccurency) {
		case 0:
			orderOfListSections[curNumber * 4] = UND;
			orderOfListSections[curNumber * 4 + 1] = UND;
			orderOfListSections[curNumber * 4 + 2] = UND;
			orderOfListSections[curNumber * 4 + 3] = UND;
			break;
		case 1:
			orderOfListSections[curNumber * 4] = sectionOrder[0];
			orderOfListSections[curNumber * 4 + 1] = UND;
			orderOfListSections[curNumber * 4 + 2] = UND;
			orderOfListSections[curNumber * 4 + 3] = UND;
			break;
		case 2:
			orderOfListSections[curNumber * 4] = sectionOrder[0];
			orderOfListSections[curNumber * 4 + 1] = sectionOrder[1];
			orderOfListSections[curNumber * 4 + 2] = UND;
			orderOfListSections[curNumber * 4 + 3] = UND;
			break;
		case 3:
			orderOfListSections[curNumber * 4] = sectionOrder[0];
			orderOfListSections[curNumber * 4 + 1] = sectionOrder[1];
			orderOfListSections[curNumber * 4 + 2] = sectionOrder[2];
			orderOfListSections[curNumber * 4 + 3] = UND;
			break;
		case 4:
			orderOfListSections[curNumber * 4] = sectionOrder[0];
			orderOfListSections[curNumber * 4 + 1] = sectionOrder[1];
			orderOfListSections[curNumber * 4 + 2] = sectionOrder[2];
			orderOfListSections[curNumber * 4 + 3] = sectionOrder[3];
			break;
	}
	curNumber++;
	return true;
}

bool Assembler::firstPass() {
	symTable = new SymbolTable();
	symTable->startAddress = startAddress;
	relTable = new RelTable();

	long curSectionSize = 0;
	numOfOccurency = 0;
	std::fstream file;

	file.open(fileName);
	if (!file.is_open()) {
		std::cout << "\n**** OTVARANJE ULAZNOG FAJLA NIJE USPELO! ****\n" << std::endl;
		return false;
	}

	std::string line;
	this->curSection = UND;
	while(std::getline(file, line)) {
		if (line.empty()) continue;
		if (!line.compare(".end")) {
			if (curSection != UND) {
				symTable->setSectionSize(curSection, locCounter - curSectionSize);
				symTable->mergeTables(); // Merge symbol and section table.
				file.close();
				return true;
			}
			return false;
		}

		Result* r = parser->parseString(line.c_str());
		switch (r->status) {
			case LABEL: {
				LabelResult* lr = (LabelResult*)r;
				bool succ = symTable->addSymbol(lr->labName, curSection, locCounter, LOCAL, 0, NO_ATTR);
				if (!succ) {
					std::cout << "\n**** SIMBOL " << lr->labName << " JE VEC DEFINISAN! ****\n" << std::endl;
					return false;
				}
			}
				break;
			case LABEL_OP: {
				OPResult* opr = (OPResult*)r;
				bool succ = symTable->addSymbol(opr->labName, curSection, locCounter, LOCAL, 0, NO_ATTR);
				if (!succ) {
					std::cout << "\n**** SIMBOL " << opr->labName << " JE VEC DEFINISAN! ****\n" << std::endl;
					return false;
				}

				locCounter += 2;
				if (!((opr->opType == BIN && opr->addrTypeOper1 == REG_DIR && opr->addrTypeOper2 == REG_DIR)
					|| ((opr->opType == UN && opr->addrTypeOper1 == REG_DIR) || opr->opType == NO_ARGS)))
					locCounter += 2;
			}
				break;
			case OP: {
				OPResult* opr = (OPResult*)r;
				locCounter += 2;
				if (!((opr->opType == BIN && opr->addrTypeOper1 == REG_DIR && opr->addrTypeOper2 == REG_DIR)
					|| ((opr->opType == UN && opr->addrTypeOper1 == REG_DIR) || opr->opType == NO_ARGS)))
					locCounter += 2;
			}
				break;
			case SECT: {
				SecResult* sr = (SecResult*)r;
				switch (sr->sectionName) {
				case DATA: {
					bool succ = symTable->addSection(".data", DATA, AWP, locCounter);
					if (!succ) {
						std::cout << "\n**** SEKCIJA DATA JE VEC DEFINISANA! ****\n" << std::endl;
						return false;
					}
				}
					break;
				case TEXT: {
					bool succ = symTable->addSection(".text", TEXT, AXP, locCounter);
					if (!succ) {
						std::cout << "\n**** SEKCIJA TEXT JE VEC DEFINISANA! ****\n" << std::endl;
						return false;
					}
				}
					break;
				case BSS: {
					bool succ = symTable->addSection(".bss", BSS, AW, locCounter);
					if (!succ) {
						std::cout << "\n**** SEKCIJA BSS JE VEC DEFINISANA! ****\n" << std::endl;
						return false;
					}
				}
					break;
				case RODATA: {
					bool succ = symTable->addSection(".rodata", RODATA, ARP, locCounter);
					if (!succ) {
						std::cout << "\n**** SEKCIJA RODATA JE VEC DEFINISANA! ****\n" << std::endl;
						return false;
					}
				}
					break;
                default:
                    break;
				}

				if (curSection != UND)
					symTable->setSectionSize(curSection, locCounter - curSectionSize);
				curSection = sr->sectionName;
				sectionOrder[numOfOccurency++] = curSection;
				curSectionSize = locCounter;
			}
				break;
			case LABEL_DIR: {
				DirResult* dir = (DirResult*)r;
				bool succ = symTable->addSymbol(dir->labName, curSection, locCounter, LOCAL, 0, NO_ATTR);
				if (!succ) {
					std::cout << "\n**** SIMBOL " << dir->labName << " JE VEC DEFINISAN! ****\n" << std::endl;
					return false;
				}

				switch (dir->type) {
				case CHAR:
					locCounter += 1;
					break;
				case WORD:
					locCounter += 2;
					break;
				case LONG:
					locCounter += 4;
					break;
				case ALIGN:
					locCounter += dir->value;
					break;
				case SKIP:
					locCounter += dir->value;
					break;
				}
			}
				break;
			case DIRECTIVE: {
				DirResult* dir = (DirResult*)r;
				switch (dir->type) {
				case CHAR:
					locCounter += 1;
					break;
				case WORD:
					locCounter += 2;
					break;
				case LONG:
					locCounter += 4;
					break;
				case ALIGN:
					locCounter += dir->value;
					break;
				case SKIP:
					locCounter += dir->value;
					break;
				}
			}
				break;
			case IMPORT_EXPORT: // No effect in first pass.
				break;
			case ERROR:
                std::cout << line << std::endl;
				std::cout << "\n**** GRESKA PRILIKOM PARSIRANJE ULAZNOG FAJLA ****\n" << std::endl;
				return false;
			}
	}

	return false; // No ".end" directive found.
}

bool Assembler::secondPass() {

	char resultFileName[MAX_FILE_NAME_LENGTH];
	int length = strlen(fileName);
	for (int i = 0; i < length; i++)
		resultFileName[i] = fileName[i];

	resultFileName[length] = 'r';
	resultFileName[length + 1] = '\0';

	std::ofstream outputFile;
	outputFile.open(resultFileName);
	if (!outputFile.is_open()) {
		std::cout << "\n**** OTVARANJE IZLAZNOG FAJLA NIJE USPELO! ****\n" << std::endl;
		return false;
	}

	std::fstream file;
	file.open(fileName);
	if (!file.is_open()) {
		std::cout << "\n**** OTVARANJE ULAZNOG FAJLA NIJE USPELO! ****\n" << std::endl;
		return false;
	}

	this->locCounter = startAddress;
	long curSectionSize = 0;
	std::string line;
	this->curSection = UND;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		if (!line.compare(".end")) {
			if (curSection != UND) {
				outputFile << std::endl;
				symTable->printTables(outputFile);

				outputFile << std::endl;
				relTable->printTables(outputFile);

				file.close();
				outputFile.close();
				return true;
			}
			return false;
		}

		Result* r = parser->parseString(line.c_str());
		switch (r->status) {
		case LABEL:// No effect in second pass.
			break;
		case LABEL_OP: case OP: {
			OPResult* opr = (OPResult*)r;
			bool inUse1 = false, inUse2 = false;
			short res = getPseudoOPCode(opr->mnm);

			if (res != UNDEFINED_VALUE) {
				switch (res) {
					case 0: // reteq
						opr = (OPResult*)parser->parseString("popeq r7");
						break;
					case 1: // retne
						opr = (OPResult*)parser->parseString("popne r7");
						break;
					case 2: // retgt
						opr = (OPResult*)parser->parseString("popgt r7");
						break;
					case 3: // regal
						opr = (OPResult*)parser->parseString("popal r7");
						break;
					case 4: // jmpeq
						if (opr->addrTypeOper1 == IMM || opr->addrTypeOper1 == VALUE_OF_SYM)
							opr->mnm = "moveq";
						else
							opr->mnm = "addeq";

						opr->addrTypeOper2 = opr->addrTypeOper1;
						opr->addrTypeOper1 = REG_DIR;
						opr->op2 = opr->op1;
						opr->op1 = "7";
						opr->opType = BIN;
						break;
					case 5: // jmpne
						if (opr->addrTypeOper1 == IMM || opr->addrTypeOper1 == VALUE_OF_SYM)
							opr->mnm = "movne";
						else
							opr->mnm = "addne";

						opr->addrTypeOper2 = opr->addrTypeOper1;
						opr->addrTypeOper1 = REG_DIR;
						opr->op2 = opr->op1;
						opr->op1 = "7";
						opr->opType = BIN;
						break;
					case 6: // jmpgt
						if (opr->addrTypeOper1 == IMM || opr->addrTypeOper1 == VALUE_OF_SYM)
							opr->mnm = "movgt";
						else
							opr->mnm = "addgt";

						opr->addrTypeOper2 = opr->addrTypeOper1;
						opr->addrTypeOper1 = REG_DIR;
						opr->op2 = opr->op1;
						opr->op1 = "7";
						opr->opType = BIN;
						break;
					case 7: // jmpal
						if (opr->addrTypeOper1 == IMM || opr->addrTypeOper1 == VALUE_OF_SYM)
							opr->mnm = "moval";
						else
							opr->mnm = "addal";

						opr->addrTypeOper2 = opr->addrTypeOper1;
						opr->addrTypeOper1 = REG_DIR;
						opr->op2 = opr->op1;
						opr->op1 = "7";
						opr->opType = BIN;
						break;
				}
			}

			unsigned long printValue = getInstructionCode(opr, inUse1, inUse2, curSectionSize);
			if (inUse1 || inUse2) {
				unsigned short firstWord = printValue & ~(~0L << 16);
				unsigned short firstWordFirstPart = firstWord & ~(~0L << 8);
				unsigned short firstWordSecondPart = firstWord >> 8;

				unsigned short secondWord = printValue >> 16;
				unsigned short secondWordFirstPart = secondWord & ~(~0L << 8);
				unsigned short secondWordSecondPart = secondWord >> 8;

				outputFile << std::setw(2) << std::setfill('0') << std::hex << secondWordFirstPart << " " << std::setw(2) << secondWordSecondPart
					<< " " << std::setw(2) << firstWordFirstPart << " " << std::setw(2) << firstWordSecondPart << " ";
			}
			else {
				unsigned short firstWord = printValue & ~(~0L << 16);
				unsigned short firstWordFirstPart = firstWord & ~(~0L << 8);
				unsigned short firstWordSecondPart = firstWord >> 8;

				outputFile << std::setw(2) << std::setfill('0') << std::hex << firstWordFirstPart << " " << std::setw(2) << firstWordSecondPart << " ";
			}

			char* prValue = new char[32];
			prValue = itoa(printValue, prValue, 16);
			std::string stemp = std::string(prValue);
			delete prValue;
			std::string str("");

			if (inUse1 || inUse2) {
				switch (stemp.size()) {
                    case 1:
                        stemp = "0000000" + stemp;
                        break;
                    case 2:
                        stemp = "000000" + stemp;
                        break;
                    case 3:
                        stemp = "00000" + stemp;
                        break;
                    case 4:
                        stemp = "0000" + stemp;
                        break;
                    case 5:
                        stemp = "000" + stemp;
                        break;
                    case 6:
                        stemp = "00" + stemp;
                        break;
                    case 7:
                        stemp = "0" + stemp;
                        break;
				}
				str.append(stemp.substr(2, 2)).append(stemp.substr(0, 2)).append(stemp.substr(6, 2)).append(stemp.substr(4, 2));
			}
			else {
				switch (stemp.size()) {
                    case 1:
                        stemp = "000" + stemp;
                        break;
                    case 2:
                        stemp = "00" + stemp;
                        break;
                    case 3:
                        stemp = "0" + stemp;
                        break;
				}
				str.append(stemp.substr(2, 2)).append(stemp.substr(0, 2));
			}

			sectionContent[curSection] = sectionContent[curSection].append(str);

			locCounter += 2;
			if (!((opr->opType == BIN && opr->addrTypeOper1 == REG_DIR && opr->addrTypeOper2 == REG_DIR)
					|| ((opr->opType == UN && opr->addrTypeOper1 == REG_DIR) || opr->opType == NO_ARGS)))
				locCounter += 2;
		}
			break;
		case SECT: {
			SecResult* sr = (SecResult*)r;
			if (curSection == UND)
				outputFile << "#" << resultFileName;
			curSection = sr->sectionName;
			curSectionSize = locCounter;
			outputFile << "\n#" << sections[curSection] << std::endl;
		}
		    break;
		case LABEL_DIR: case DIRECTIVE: {
			DirResult* dir = (DirResult*)r;
			switch (dir->type) {
			case CHAR: {
				char* prValue = new char[32];
				prValue = itoa(dir->value, prValue, 16);
				std::string stemp = std::string(prValue);
				delete prValue;

				switch (stemp.size()) {
                    case 1:
                        stemp = "0" + stemp;
                        break;
                    default:
                        break;
				}

				outputFile << std::hex << stemp << " ";
				sectionContent[curSection] = sectionContent[curSection].append(stemp);
				locCounter += 1;
			}
				break;
			case WORD: {
				char* prValue = new char[32];
				prValue = itoa(dir->value, prValue, 16);
				std::string stemp = std::string(prValue);
				delete prValue;

				switch (stemp.size()) {
                    case 1:
                        stemp = "000" + stemp;
                        break;
                    case 2:
                        stemp = "00" + stemp;
                        break;
                    case 3:
                        stemp = "0" + stemp;
                        break;
                    default:
                        break;
				}

				outputFile << std::hex << stemp[2] << stemp[3] << stemp[0] << stemp[1] << " ";
				std::string s1("");
				s1.append(stemp.substr(2, 2)).append(stemp.substr(0, 2));
				sectionContent[curSection] = sectionContent[curSection].append(s1);
				locCounter += 2;
			}
				break;
			case LONG: {
				char* prValue = new char[32];
				prValue = itoa(dir->value, prValue, 16);
				std::string stemp(prValue);
				delete prValue;

				switch (stemp.size()) {
                    case 1:
                        stemp = "0000000" + stemp;
                        break;
                    case 2:
                        stemp = "000000" + stemp;
                        break;
                    case 3:
                        stemp = "00000" + stemp;
                        break;
                    case 4:
                        stemp = "0000" + stemp;
                        break;
                    case 5:
                        stemp = "000" + stemp;
                        break;
                    case 6:
                        stemp = "00" + stemp;
                        break;
                    case 7:
                        stemp = "0" + stemp;
                        break;
				}

				outputFile << std::hex << stemp[6] << stemp[7] << stemp[4] << stemp[5] << " " << stemp[2] << stemp[3] << stemp[0] << stemp[1] << " ";
				std::string s1("");
				s1.append(stemp.substr(6, 2)).append(stemp.substr(4, 2)).append(stemp.substr(2, 2)).append(stemp.substr(0, 2));
				sectionContent[curSection] = sectionContent[curSection].append(s1);
				locCounter += 4;
			}
				break;
			case ALIGN: {
				char c = '0';
				for (int i = 0; i < dir->value; i++) {
					outputFile << std::hex << c << c << " ";
					sectionContent[curSection] = sectionContent[curSection].append("00");
				}
				locCounter += dir->value;
			}
				break;
			case SKIP: {
				char c = '0';
				for (int i = 0; i < dir->value; i++) {
					outputFile << std::hex << c << c << " ";
					sectionContent[curSection] = sectionContent[curSection].append("00");
				}
				locCounter += dir->value;
			}
				break;
			}
		}
			break;
		case IMPORT_EXPORT: {
			EIResult* eir = (EIResult*)r;
			if (!eir->isGlobal) {// Extern symbol.
				bool succ = symTable->addSymbol(eir->EISymbolName, UND, 0, GLOBAL, 0, NO_ATTR);
				if (!succ) {
					std::cout << "\n**** SIMBOL " << eir->EISymbolName << " JE VEC DEFINISAN! ****\n" << std::endl;
					return false;
				}
			}
			else // Global symbol.
				symTable->setGlobalVisibility(eir->EISymbolName);
		}
			break;
		case ERROR:
            std::cout << line << std::endl;
			std::cout << "\n**** GRESKA PRILIKOM PARSIRANJE ULAZNOG FAJLA ****\n";
			return false;
		}
	}
	return false; // Should never happen.
}

long Assembler::getInstructionCode(OPResult* opr, bool& inUse1, bool& inUse2, long curSectionSize) {
	long instCode = 0;
	long ret = 0;

	instCode |= opr->cond;
	short temp = getOPCode(opr->mnm);
	instCode |= (temp << 2);

	if (opr->opType == NO_ARGS)
		return instCode;

	long value1 = 0;
	inUse1 = false;

	temp = getAddrCode(opr, 0, value1, inUse1, curSectionSize);
	instCode |= (temp << 6);

	if (opr->opType == UN) {
		if (inUse1) {
			value1 &= ~(~0L << 16);
			ret = instCode << 16 | value1;
		}
		else
			ret = instCode;
		return ret;
	}

	long value2 = 0;
	inUse2 = false;

	temp = getAddrCode(opr, 1, value2, inUse2, curSectionSize);
	instCode |= (temp << 11);

	if (inUse1) {
		value1 &= ~(~0L << 16);
		ret = instCode << 16 | value1;
	}
	else {
		if (inUse2) {
			value2 &= ~(~0L << 16);
			ret = instCode << 16 | value2;
		}
		else
			ret = instCode;
	}

	return ret;
}

short Assembler::getOPCode(std::string& opcode) {
	for (int i = 0; i < NUM_OF_INSRUCTION_OPCODES; i++) {
		for (int j = 0; j < NUM_OF_COND_TYPE; j++) {
			if (!strcmp(opcode.c_str(), operation[i][j])) {
				return i;
			}
		}
	}
	return UNDEFINED_VALUE; // Won't happen.
}

short Assembler::getPseudoOPCode(std::string & opcode) {
	for (int i = 0; i < NUM_OF_PSEUDO_OPCODES; i++) {
		for (int j = 0; j < NUM_OF_COND_TYPE; j++) {
			if (!strcmp(opcode.c_str(), secialOperations[i][j])) {
				return i*NUM_OF_COND_TYPE + j;
			}
		}
	}
	return UNDEFINED_VALUE; // Won't happen.
}

short Assembler::getAddrCode(OPResult* opr, int i, long& value, bool& inUse, long curSectionSize) {
	short ret = 0;
	if (i == 1) {
		switch (opr->addrTypeOper2) {
		case IMM:
			value = stol(opr->op2);
			inUse = true;
			return ret;
		case REG_DIR:
			ret = (1 << 3) | stol(opr->op2);
			return ret;
		case REG_IND_CONST_OFFSET:
			ret = (3 << 3) | stol(opr->op2);
			value = opr->constoffsetValue;
			inUse = true;
			return ret;
		case REG_IND_MEM_OFFSET:
			ret = (3 << 3) | stol(opr->op2);
			if (symTable->isGlobal(opr->memOffsetValue)) {
				if (symTable->isUndefinedSymbol(opr->memOffsetValue)) {
					value = 0;
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->memOffsetValue), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->memOffsetValue);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->memOffsetValue), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->memOffsetValue);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSecSerialNumberOfSymbol(opr->memOffsetValue), curSection);
			}
			inUse = true;
			return ret;
		case MEM:
			ret = 2 << 3;
			if (symTable->isGlobal(opr->op2)) {
				if (symTable->isUndefinedSymbol(opr->op2)) {
					value = 0;
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op2), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->op2);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op2), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->op2);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSecSerialNumberOfSymbol(opr->op2), curSection);
			}
			inUse = true;
			return ret;
		case VALUE_OF_SYM:
			if (symTable->isGlobal(opr->op2)) {
				if (symTable->isUndefinedSymbol(opr->op2)) {
					value = 0;
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op2), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->op2);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op2), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->op2);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSecSerialNumberOfSymbol(opr->op2), curSection);
			}
			inUse = true;
			return ret;
		case PC_REL:
			ret = (3 << 3) | 7; // r7 is PC
			if (symTable->isGlobal(opr->op2)) {
				if (symTable->isUndefinedSymbol(opr->op2)) {
					value = - (locCounter + OFFSET_LENGTH + OP_CODE_LENGTH);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_PC32, symTable->getSerialNumber(opr->op2), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->op2) - (locCounter + OFFSET_LENGTH + OP_CODE_LENGTH);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_PC32, symTable->getSerialNumber(opr->op2), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->op2) - (locCounter + OFFSET_LENGTH + OP_CODE_LENGTH);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_PC32, symTable->getSecSerialNumberOfSymbol(opr->op2), curSection);
			}
			inUse = true;
			return ret;
		case ABS:
			ret = 2 << 3;
			value = stol(opr->op2);
			inUse = true;
			return ret;
		}
	}
	else {
		switch (opr->addrTypeOper1) {
		case IMM:
			value = stol(opr->op1);
			inUse = true;
			return ret;
		case REG_DIR:
			ret = (1 << 3) | stol(opr->op1);
			return ret;
		case REG_IND_CONST_OFFSET:
			ret = (3 << 3) | stol(opr->op1);
			value = opr->constoffsetValue;
			inUse = true;
			return ret;
		case REG_IND_MEM_OFFSET:
			ret = (3 << 3) | stol(opr->op1);
			if (symTable->isGlobal(opr->memOffsetValue)) {
				if (symTable->isUndefinedSymbol(opr->memOffsetValue)) {
					value = 0;
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->memOffsetValue), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->memOffsetValue);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->memOffsetValue), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->memOffsetValue);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSecSerialNumberOfSymbol(opr->memOffsetValue), curSection);
			}
			inUse = true;
			return ret;
		case MEM:
			ret = 2 << 3;
			if (symTable->isGlobal(opr->op1)) {
				if (symTable->isUndefinedSymbol(opr->op1)) {
					value = 0;
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op1), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->op1);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op1), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->op1);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSecSerialNumberOfSymbol(opr->op1), curSection);
			}
			inUse = true;
			return ret;
		case VALUE_OF_SYM:
			if (symTable->isGlobal(opr->op1)) {
				if (symTable->isUndefinedSymbol(opr->op1)) {
					value = 0;
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op1), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->op1);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSerialNumber(opr->op1), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->op1);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_32, symTable->getSecSerialNumberOfSymbol(opr->op1), curSection);
			}
			inUse = true;
			return ret;
		case PC_REL:
			ret = (3 << 3) | 7; // r7 is PC
			if (symTable->isGlobal(opr->op1)) {
				if (symTable->isUndefinedSymbol(opr->op1)) {
					value = -(locCounter + OFFSET_LENGTH + OP_CODE_LENGTH);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_PC32, symTable->getSerialNumber(opr->op1), curSection);
				}
				else {
					value = symTable->getSymbolValue(opr->op1) - (locCounter + OFFSET_LENGTH + OP_CODE_LENGTH);
					relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_PC32, symTable->getSerialNumber(opr->op1), curSection);
				}
			}
			else {
				value = symTable->getSymbolValue(opr->op1) - (locCounter + OFFSET_LENGTH + OP_CODE_LENGTH);
				relTable->addRelRecord(locCounter + OP_CODE_LENGTH - curSectionSize, R_386_PC32, symTable->getSecSerialNumberOfSymbol(opr->op1), curSection);
			}
			inUse = true;
			return ret;
		case ABS:
			ret = 2 << 3;
			value = stol(opr->op1);
			inUse = true;
			return ret;
		}
	}
	return 0;
}
