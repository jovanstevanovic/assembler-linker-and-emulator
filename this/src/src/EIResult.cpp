// File: EIResult.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include "EIResult.h"

EIResult* EIResult::eiResult = nullptr;

EIResult * EIResult::getInstance() {
	if (eiResult == nullptr) {
		eiResult = new EIResult();
	}
	return eiResult;
}

EIResult::EIResult() {

}
