#include "SectionResult.h"
// File: SectionResult.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

SecResult* SecResult::secResult = nullptr;

SecResult * SecResult::getInstance() {
	if (secResult == nullptr) {
		secResult = new SecResult();
	}
	return secResult;
}

SecResult::SecResult() {
	
}
