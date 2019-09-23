// File: OPResult.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "OPResult.h"

OPResult* OPResult::opResult = nullptr;

OPResult* OPResult::getInstance() {
	if (opResult == nullptr) {
		opResult = new OPResult();
	}
	return opResult;
}

OPResult::OPResult() {

}
