#include "Result.h"
// File: Result.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

Result* Result::result = nullptr;

Result * Result::getInstance() {
	if (result == nullptr) {
		result = new Result();
	}
	return result;
}

Result::Result() {
	
}
