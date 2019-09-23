#ifndef _PARSER_H_
#define _PARSER_H_

// File: Parser.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include <regex>

#include "Macros.h"

class Result;
class OPResult;

class Parser {
public:

	// Constructors
	Parser();

	// Destructor
	~Parser();

	// Util methods
	Result* parseString(const char* line);

private:

	Result* extractOperands(std::cmatch& m);
	CONDITION extractCondition(const std::string cond);
	void fillOPResult(std::cmatch& m, OPResult* opr, int num);
	bool checkCorrectness(OPResult* opr);

	std::regex* correctImportExport;
	std::regex* correctLabelDef;
	std::regex* correctSectionDef;
	std::regex* correctOperationDef;
	std::regex* correctDirectives;
	std::regex* correctOperand;
	std::regex* correctPopOperands;
	std::regex* correctOperBinOperation;
};

#endif
