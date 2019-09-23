#ifndef _OPRESULT_H_
#define _OPRESULT_H_

// File: OPResult.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include <string>

#include "Result.h"

class Parser;
class Assembler;

class OPResult : public Result {
public:

	// Util methods
	static OPResult* getInstance();

protected:

	// Constructors
	OPResult();

private:

	friend class Parser;
	friend class Assembler;

	static OPResult* opResult;

	std::string op1;
	std::string op2;
	std::string mnm;

	ADDR_TYPE addrTypeOper1;
	ADDR_TYPE addrTypeOper2;

	long constoffsetValue;
	std::string memOffsetValue;
	
	OP_TYPE opType;

	CONDITION cond;
	
	std::string labName;
};

#endif
