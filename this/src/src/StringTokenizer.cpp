// File: StringTokenizer.h
// Created by: Xerox
// Date: 27.05.2018
// Last modified: 05.06.2018.

#include "StringTokenizer.h"
#include "STResult.h"

long StringTokenizer::getValue(std::string & token) {
	long valTemp = strtol(token.c_str(), 0, 16);
	if (valTemp > 0x7fff)
		valTemp -= 0x10000;

	return valTemp;
}

STResult* StringTokenizer::parseString(std::string & token) {
	long value = strtol(token.c_str(), 0, 16);

	STResult* res = STResult::getInstance();
	res->cond = (CONDITION)(value & ~(~0L << 2));
	res->mnm = (short)((value & ~(~0L << 6)) >> 2);
	res->addrType1 = (ADDR_TYPE)((value & ~(~0L << 11)) >> 9);
	if (res->addrType1 == REG_DIR || res->addrType1 == REG_IND_CONST_OFFSET)
		res->reg1 = (value & ~(~0L << 9)) >> 6;

	res->addrType2 = (ADDR_TYPE)(value >> 14);
	if (res->addrType2 == REG_DIR || res->addrType2 == REG_IND_CONST_OFFSET)
		res->reg2 = (value & ~(~0L << 14)) >> 11;

	res->numOfArgs = countNumOfArgs(res->mnm);
	return res;
}

OP_TYPE StringTokenizer::countNumOfArgs(short mnm) {
	switch (mnm) {
	case 0:		// ADD -- Binary operations
	case 1:		// SUB
	case 2:		// MUL
	case 3:		// DIV
	case 4:		// CMP
	case 5:		// AND
	case 6:		// OR
	case 7:		// NOT
	case 8:		// TEST
	case 13:	// MOV
	case 14:	// SHL
	case 15:	// SHR
		return BIN;
		break;
	case 9:		// PUSH -- Unary operations
	case 10:	// POP
	case 11:	// CALL
		return UN;
		break;
	case 12:	// IRET -- Noargs operation
		return NO_ARGS;
		break;
	}
	return BIN; // Cannot happen
}
