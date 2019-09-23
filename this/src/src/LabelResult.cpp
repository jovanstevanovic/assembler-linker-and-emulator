// File: LabResult.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "LabelResult.h"

LabelResult* LabelResult::labResult = nullptr;

LabelResult * LabelResult::getInstance() {
	if (labResult == nullptr) {
		labResult = new LabelResult();
	}
	return labResult;
	
}

LabelResult::LabelResult() {

}
