#ifndef _DIRRESULT_H_
#define _DIRRESULT_H_

// File: DirResult.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "Macros.h"
#include "Result.h"

class Parser;
class Assembler;

class DirResult : public Result {
public:

	// Util methods
	static DirResult* getInstance();

protected:

	// Constructors
	DirResult();

private:

	friend class Parser;
	friend class Assembler;

	static DirResult* dirResult;

	DIR_TYPE type;
	long value;

	std::string labName;
};

#endif
