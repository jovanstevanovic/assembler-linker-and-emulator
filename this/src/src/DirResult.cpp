// File: DirResult.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "DirResult.h"

DirResult* DirResult::dirResult = nullptr;

DirResult * DirResult::getInstance() {
	if (dirResult == nullptr) {
		dirResult = new DirResult();
	}
	return dirResult;
}

DirResult::DirResult() {

}