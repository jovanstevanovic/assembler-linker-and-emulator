#ifndef _EIRESULT_H_
#define _EIRESULT_H_

// File: EIResult.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include <string>

#include "Result.h"

class Parser;
class Assembler;

class EIResult : public Result {
public:

	// Util methods
	static EIResult* getInstance();

protected:

	// Constructors
	EIResult();

private:

	friend class Parser;
	friend class Assembler;

	static EIResult* eiResult;

	std::string EISymbolName;
	bool isGlobal;
};

#endif
