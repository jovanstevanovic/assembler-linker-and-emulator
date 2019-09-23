#ifndef _STTOKEN_H_
#define _STTOKEN_H_

// File: StringTokenizer.h
// Created by: Xerox
// Date: 27.05.2018
// Last modified: 02.06.2018.

#include <string>
#include "Macros.h"

class STResult;

class StringTokenizer {
public:

	// Util methods
	long getValue(std::string& token);
	STResult* parseString(std::string& token);
	
private:

	OP_TYPE countNumOfArgs(short mnm);
};

#endif
