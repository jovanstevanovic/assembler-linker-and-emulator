#ifndef _STRESULT_H_
#define _STRESULT_H_

// File: STResult.h
// Created by: Xerox
// Date: 27.05.2018
// Last modified: 02.06.2018.

#include "Macros.h"

class Emulator;
class StringTokenizer;

class STResult {
public:

	// Util methods
	static STResult* getInstance();

protected:

	// Construcotrs
	STResult();

private:

	friend class Emulator;
	friend class StringTokenizer;

	static STResult* result;

	CONDITION cond;
	short mnm;

	short reg1;
	short reg2;
	
	ADDR_TYPE addrType1;
	ADDR_TYPE addrType2;

	OP_TYPE numOfArgs;
};

#endif
