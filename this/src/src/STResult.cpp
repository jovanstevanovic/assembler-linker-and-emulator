// File: STResult.cpp
// Created by: Xerox
// Date: 27.05.2018
// Last modified: 02.06.2018.

#include "STResult.h"

STResult* STResult::result = nullptr;

STResult * STResult::getInstance() {
	if (result == nullptr) {
		result = new STResult();
	}
	return result;
}

STResult::STResult() {

}
