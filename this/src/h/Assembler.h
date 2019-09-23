#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

// File: Assembler.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 31.05.2018.

#include <fstream>
#include <array>

#include "Macros.h"

class Parser;
class SymbolTable;
class RelTable;
class Emulator;
class OPResult;

class Assembler {
public:

	// Constructors
	Assembler();
	Assembler(const char* fileName, unsigned short startAddress);

	// Destructor
	~Assembler();

	// Util methods
	bool execute();

public:

	friend class Emulator;

	bool firstPass();
	bool secondPass();

	long locCounter;
	SECTION curSection;
	Parser* parser;
	unsigned long startAddress;

	SymbolTable* symTable;
	RelTable* relTable;

	static std::array<SymbolTable*, MAX_NUM_OF_INPUT_FILES> listOfSymTables;
	static std::array<RelTable*, MAX_NUM_OF_INPUT_FILES> listOfRelTables;
	static std::array<std::string*, 4 * MAX_NUM_OF_INPUT_FILES> listOfSecCont;
	static std::array<int, 4 * MAX_NUM_OF_INPUT_FILES> orderOfListSections;
	static int curNumber;

	char fileName[MAX_FILE_NAME_LENGTH];

	char* operation[NUM_OF_INSRUCTION_OPCODES][4] = {
		{(char*)"addeq", (char*)"addne", (char*)"addgt", (char*)"addal"},
		{(char*)"subeq", (char*)"subne", (char*)"subgt", (char*)"subal"},
		{(char*)"muleq", (char*)"mulne", (char*)"mulgt", (char*)"mulal"},
		{(char*)"diveq", (char*)"divne", (char*)"divgt", (char*)"dival"},
		{(char*)"cmpeq", (char*)"cmpne", (char*)"cmpgt", (char*)"cmpal"},
		{(char*)"andeq", (char*)"andne", (char*)"andgt", (char*)"andal"},
		{(char*)"oreq", (char*)"orne", (char*)"orgt", (char*)"oral"},
		{(char*)"noteq", (char*)"notne", (char*)"notgt", (char*)"notal"},
		{(char*)"testeq",(char*)"testne", (char*)"testgt", (char*)"testal"},
		{(char*)"pusheq", (char*)"pushne", (char*)"pushgt", (char*)"pushal"},
		{(char*)"popeq", (char*)"popne", (char*)"popgt", (char*)"popal"},
		{(char*)"calleq", (char*)"callne", (char*)"callgt", (char*)"callal"},
		{(char*)"ireteq", (char*)"iretne", (char*)"iretgt", (char*)"iretal"},
		{(char*)"moveq", (char*)"movne", (char*)"movgt", (char*)"moval"},
		{(char*)"shleq", (char*)"shlne", (char*)"shlgt", (char*)"shlal"},
		{(char*)"shreq", (char*)"shrne", (char*)"shrgt", (char*)"shral"}
	 };

	char* secialOperations[NUM_OF_PSEUDO_OPCODES][4] = {
		{ (char*)"reteq", (char*)"retne", (char*)"retgt", (char*)"retal" },
		{ (char*)"jmpeq", (char*)"jmpne", (char*)"jmpgt", (char*)"jmpal" }
	};

	char* sections[5] = { (char*)".und", (char*)".text", (char*)".data", (char*)".bss", (char*)".rodata" };

	std::string sectionContent[MAX_NUM_OF_SECTION];
	SECTION sectionOrder[4];
	int numOfOccurency;

	long getInstructionCode(OPResult* opr, bool& inUse1, bool& inUse2, long curSectionSize);
	short getOPCode(std::string& opcode);
	short getPseudoOPCode(std::string& opcode);
	short getAddrCode(OPResult* opr, int i, long& value, bool& inUse, long curSectionSize);
};

#endif
