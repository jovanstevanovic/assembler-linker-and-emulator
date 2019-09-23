#ifndef _EMULATOR_H_
#define _EMULATOR_H_

// File: Emulator.h
// Created by: Xerox
// Date: 23.05.2018
// Last modified: 05.06.2018.

#include "Macros.h"
#include <string>

class Linker;
class StringTokenizer;
class STResult;

class Emulator {
public:

	// Consutructors
	Emulator(const char* filesName[], int numOfObjFiles);

	// Destructor
	~Emulator();

	// Util methods
	bool execute();
	void loadIntoMemory();

private:

	bool checkIfMemCntWillCorrupt();

	long exctractOperand(int index, STResult* res, long& dstAddr);
	long getValueFromMemory(unsigned short offset);

	void writeInMemory(unsigned short a, long b);
	void readFromMemory(std::string& nextToken);

	void writeInStack(long a);
	short readFromStack();

	void extractPSWfromShort(unsigned short psw);
	unsigned short insertPSWIntoShort();

	bool isConditionRight(CONDITION cond);
	void updatePSWStateZN(short res);
	void updatePSWStateZNOC(short mnm, short firstOperand, short secondOperand, short res);
	void updatePSWStateC(short mnm, short firstOperand, short secondOperand, short res);
	
	void loadIVTRoutineContentIntoMemory();

	struct PSW {
		bool Z;
		bool C;
		bool V;
		bool N;
		bool I;

		PSW() {
			this->Z = this->C = this->V = this->N = false;
			this->I = true;
		}
	};

	Linker* linker;
	StringTokenizer* st;
	
	char MEM[MEM_SIZE];
	short regs[NUM_OF_REG];
	unsigned short PC, SP;

	const char* IVTRoutineContent[NUM_OF_INTERRUPT_ROUTINES] = {  // 3000 -> 0030 -> IRET
		"3300", 
		"3300", 
		"3300", 
		"3300", 
		"3300", 
		"3300", 
		"3300", 
		"3300" 
	};
	short IVT_TABLE[NUM_OF_INTERRUPT_ROUTINES];
	short endAddressOfIVTRoutineContent;
	bool dividingByZero;

	PSW* psw;
	PSW* oldPsw;

	bool end;
	float clockTime;

};

#endif
