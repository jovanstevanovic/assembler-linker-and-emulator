#ifndef _LABEL_RESULT_H_
#define _LABEL_RESULT_H_

// File: LabResult.h
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "OPResult.h"

class Parser;
class Assembler;

class LabelResult : public OPResult {
public:

	// Util methods
	static LabelResult* getInstance();

protected:

	// Constructors
	LabelResult();

private:

	friend class Parser;
	friend class Assembler;

	static LabelResult* labResult;

	std::string labName;
};

#endif
