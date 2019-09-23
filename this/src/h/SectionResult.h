#ifndef _SEC_RESULT_H_
#define _SEC_RESULT_H_

// File: SectionResult.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "Result.h"
#include "Macros.h"

class Parser;
class Assembler;

class SecResult : public Result {
public:

	// Util methods
	static SecResult* getInstance();

protected:

	// Constructors
	SecResult();

private:
	friend class Parser;
	friend class Assembler;

	static SecResult* secResult;

	SECTION sectionName;
};

#endif
