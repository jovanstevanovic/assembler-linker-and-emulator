#ifndef _RESULT_H_
#define _RESULT_H_

// File: Result.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "Macros.h"

class Parser;
class Assembler;

class Result {
public:

	// Util methods
	static Result* getInstance();

protected:

	// Constructors
	Result();

private:

	friend class Parser;
	friend class Assembler;

	static Result* result;

	STATUS status;
};

#endif
