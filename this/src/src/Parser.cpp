// File: Parser.cpp
// Created by: Xerox
// Date: 22.05.2018.
// Last modified: 30.05.2018.

#include <iostream>

#include <regex>
#include <string>

#include "Parser.h"
#include "OPResult.h"
#include "LabelResult.h"
#include "EIResult.h"
#include "SectionResult.h"
#include "DirResult.h"

using namespace std;
using namespace std::regex_constants;

Parser::Parser() {

	this->correctImportExport = new std::regex("^[\\t*\\s*]*(\\.global)\\s+(\\w+)\\s*$|^[\\t*\\s*]*(\\.extern)\\s+(\\w+)\\s*$",
		std::regex_constants::ECMAScript | icase);

	this->correctLabelDef = new std::regex("^[\\t*\\s*]*(\\w+\\d*):\\s*(\\w+.*)$|^[\\t*\\s*]*(\\w+\\d*):\\s*$"
		"|^[\\t*\\s*]*(\\w+\\d*):\\s*(\\..+)\\s*$", std::regex_constants::ECMAScript | icase);

	this->correctSectionDef = new std::regex("^[\\t*\\s*]*(\\.data)\\s*$|^[\\t*\\s*]*(\\.text)\\s*$|^[\\t*\\s*]*(\\.bss)\\s*$|^[\\t*\\s*]*(\\.rodata)\\s*$",
		std::regex_constants::ECMAScript | icase);

	this->correctOperationDef = new std::regex("^[\\t*\\s*]*(add(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(sub(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(mul(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(div(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(cmp(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(and(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(or(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(not(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(test(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(push(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(pop(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(call(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(mov(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(shl(eq|ne|gt|al))\\s+(.+)$|^[\\t*\\s*]*(shr(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(iret(eq|ne|gt|al))\\s*$|^[\\t*\\s*]*(jmp(eq|ne|gt|al))\\s+(.+)$"
		"|^[\\t*\\s*]*(ret(eq|ne|gt|al))\\s*$", std::regex_constants::ECMAScript | icase);

	this->correctDirectives = new std::regex("^[\\t*\\s*]*(\\.char)\\s+(\\d+)\\s*$|^[\\t*\\s*]*(\\.word)\\s+(\\d+)\\s*$"
		"|^[\\t*\\s*]*(\\.long)\\s+(\\d+)\\s*$|^[\\t*\\s*]*(\\.align)\\s+(\\d+)\\s*$|^[\\t*\\s*]*(\\.skip)\\s+(\\d+)\\s*$", std::regex_constants::ECMAScript | icase);

	this->correctOperand = new std::regex("^(\\d+)\\s*$|^(&)(\\w+\\d*)\\s*$|^([a-qs-z0-9]+)\\s*$|^(r)([0-7])\\s*$|"
		"^(r)([0-7])\\[(\\d+)\\]\\s*$|^(r)([0-7])\\[(\\w+)\\]\\s*$|^(\\$)(\\w+\\d*)\\s*$|(\\*)(\\d+)\\s*", std::regex_constants::ECMAScript | icase);

	this->correctPopOperands = new std::regex("^(r)([0-7])\\s*$", std::regex_constants::ECMAScript | icase);

	this->correctOperBinOperation = new  std::regex("^(.+)\\s*,\\s*(.+)\\s*", std::regex_constants::ECMAScript | icase);
}

Parser::~Parser() {
	delete correctImportExport;
	delete correctLabelDef;
	delete correctSectionDef;
	delete correctOperationDef;
	delete correctDirectives;
	delete correctOperand;
	delete correctPopOperands;
	delete correctOperBinOperation;

	correctImportExport = correctLabelDef = correctSectionDef = correctOperationDef =
		correctDirectives = correctOperand = correctPopOperands = correctOperBinOperation = nullptr;

}

Result* Parser::parseString(const char * line) {
	cmatch m;
	std::regex_match(line, m, *correctImportExport);
	if (m.size() > 0) {
		EIResult* eir = EIResult::getInstance();
		eir->status = IMPORT_EXPORT;
		if (!m.str(1).compare(".global")) {
			eir->isGlobal = true; // Symbol is global.
			eir->EISymbolName = m.str(2);
		}
		else {
			eir->isGlobal = false; // Symbol is extern.
			eir->EISymbolName = m.str(4);
		}
		return eir;
	}
	else {
		std::regex_match(line, m, *correctSectionDef);
		if (m.size() > 0) {
			SecResult* sr = SecResult::getInstance();
			sr->status = SECT;
			if (!m.str(1).compare(".data")) {
				sr->sectionName = DATA;
			}
			else {
				if (!m.str(2).compare(".text")) {
					sr->sectionName = TEXT;
				}
				else {
					if (!m.str(3).compare(".bss")) {
						sr->sectionName = BSS;
					}
					else {
						sr->sectionName = RODATA;
					}
				}
			}
			return sr;
		}
		else {
			std::regex_match(line, m, *correctLabelDef);
			if (m.size() > 0) {
				LabelResult* labResult = LabelResult::getInstance();
				if (m.str(3).size() > 0) {
					labResult->labName = m.str(3);
					labResult->status = LABEL;
					return labResult;
				}
				else {
					if (m.str(4).size() > 0) {
						char arr[128];
						std::string ln = m.str(4);
						strcpy(arr, m.str(5).c_str());

						std::regex_match(arr, m, *correctDirectives);
						if (m.size() > 0) {
							DirResult* dir = DirResult::getInstance();
							dir->status = LABEL_DIR;
							int i = 1;
							for (; i < 12; i++) {
								if (m.str(i).size() == 0)
									continue;
								else
									break;
							}

							switch (i) {
							case 1:
								dir->type = CHAR;
								dir->value = stol(m.str(2));
								break;
							case 3:
								dir->type = WORD;
								dir->value = stol(m.str(4));
								break;
							case 5:
								dir->type = LONG;
								dir->value = stol(m.str(6));
								break;
							case 7:
								dir->type = ALIGN;
								dir->value = stol(m.str(8));
								break;
							case 9:
								dir->type = SKIP;
								dir->value = stol(m.str(10));
								break;
							default:
								break;
							}

							dir->labName = ln;
							return dir;
						}

						Result* r = Result::getInstance();
						r->status = ERROR;
						return r;
					}
					else {
						char arr[128];
						std::string ln = m.str(1);
						strcpy(arr, m.str(2).c_str());

						std::regex_match(arr, m, *correctOperationDef);
						if (m.size() > 0) {
							Result* r = extractOperands(m);
							if (r->status == OP) {
								r->status = LABEL_OP;
								OPResult* opr = (OPResult*)r;
								opr->labName = ln;
							}
							return r;
						}

						Result* r = Result::getInstance();
						r->status = ERROR;
						return r;
					}
				}
			}
			else {
				std::regex_match(line, m, *correctDirectives);
				if (m.size() > 0) {
					DirResult* dir = DirResult::getInstance();
					dir->status = DIRECTIVE;
					int i = 1;
					for (; i < 12; i++) {
						if (m.str(i).size() == 0)
							continue;
						else
							break;
					}

					switch (i) {
					case 1:
						dir->type = CHAR;
						dir->value = stol(m.str(2));
						break;
					case 3:
						dir->type = WORD;
						dir->value = stol(m.str(4));
						break;
					case 5:
						dir->type = LONG;
						dir->value = stol(m.str(6));
						break;
					case 7:
						dir->type = ALIGN;
						dir->value = stol(m.str(8));
						break;
					case 9:
						dir->type = SKIP;
						dir->value = stol(m.str(10));
						break;
					default:
						break;
					}
					return dir;
				}
				else {
					std::regex_match(line, m, *correctOperationDef);
					if (m.size() > 0) {
						Result* r = extractOperands(m);
						return r;
					}
					else {
						Result* r = Result::getInstance();
						r->status = ERROR;
						return r;
					}
				}

			}
		}
	}

	return nullptr; // Won't be used.
}

Result * Parser::extractOperands(std::cmatch& m) {
	OPResult* opr = OPResult::getInstance();
	bool success = true;
	opr->status = OP;

	int i = 1;
	for (; i < 53; i++) {
		if (m.str(i).size() == 0)
			continue;
		else
			break;
	}

	cmatch m1;
	cmatch m2;
	char arr[128];

	opr->mnm = m.str(i);
	opr->cond = extractCondition(m.str(i+1));
	switch (i) {
	case 1: case 4: case 7: case 10: case 13: case 16: case 19: case 22: case 25: case 37: case 40: case 43: // Binary operations
		strcpy(arr, m.str(i + 2).c_str());
		std::regex_match(arr, m2, *this->correctOperBinOperation); // Extract first and second operand
		if (m2.size() == 0) {
			success = false;
			break;
		}

		strcpy(arr, m2.str(1).c_str());
		std::regex_match(arr, m1, *correctOperand); // Check first operand

		if (m1.size() == 0) {
			success = false;
			break;
		}
		fillOPResult(m1, opr, 0); // Fill return struct for first operand - OPResult.

		strcpy(arr, m2.str(2).c_str());
		std::regex_match(arr, m1, *correctOperand); // Check second operand
		if (m1.size() == 0) {
			success = false;
			break;
		}
		fillOPResult(m1, opr, 1); // Fill return struct for second operand - OPResult.
		opr->opType = BIN;

		success = checkCorrectness(opr);
		break;
	case 28: case 34: case 48: // Push, Call and Jmp
		strcpy(arr, m.str(i + 2).c_str());
		std::regex_match(arr, m1, *correctOperand); // Check first operand
		if (m1.size() == 0) {
			success = false;
			break;
		}

		fillOPResult(m1, opr, 0);
		opr->opType = UN;
		break;
	case 31: // Pop
		strcpy(arr, m.str(i + 2).c_str());
		std::regex_match(arr, m1, *correctPopOperands); // Check first operand
		if (m1.size() == 0) {
			success = false;
			break;
		}

		opr->addrTypeOper1 = REG_DIR;
		opr->op1 = m1.str(2);
		opr->opType = UN;
		break;
	case 46: case 51: // Iret and Ret
		opr->opType = NO_ARGS;
		break;
	}
	if (!success) {
		Result* r = Result::getInstance();
		r->status = ERROR;
		return r;
	}
	else
		return opr;
}

CONDITION Parser::extractCondition(const std::string cond) {
	if (!cond.compare("eq"))
		return EQ;
	else {
		if (!cond.compare("ne"))
			return NE;
		else {
			if (!cond.compare("gt"))
				return GT;
			else
				return AL;
		}
	}
}

void Parser::fillOPResult(std::cmatch & m, OPResult* opr, int num) {
	if (m.str(1).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = IMM;
			opr->op1 = m.str(1);
		}
		else {
			opr->addrTypeOper2 = IMM;
			opr->op2 = m.str(1);
		}
		return;
	}

	if (m.str(2).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = VALUE_OF_SYM;
			opr->op1 = m.str(3);
		}
		else {
			opr->addrTypeOper2 = VALUE_OF_SYM;
			opr->op2 = m.str(3);
		}
		return;
	}

	if (m.str(4).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = MEM;
			opr->op1 = m.str(4);
		}
		else {
			opr->addrTypeOper2 = MEM;
			opr->op2 = m.str(4);
		}
		return;
	}

	if (m.str(5).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = REG_DIR;
			opr->op1 = m.str(6);
		}
		else {
			opr->addrTypeOper2 = REG_DIR;
			opr->op2 = m.str(6);
		}
	}

	if (m.str(7).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = REG_IND_CONST_OFFSET;
			opr->op1 = m.str(8);
			opr->constoffsetValue = std::stol(m.str(9));
		}
		else {
			opr->addrTypeOper2 = REG_IND_CONST_OFFSET;
			opr->op2 = m.str(8);
			opr->constoffsetValue = std::stol(m.str(9));
		}
		return;
	}

	if (m.str(10).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = REG_IND_MEM_OFFSET;
			opr->op1 = m.str(11);
			opr->memOffsetValue = m.str(12);
		}
		else {
			opr->addrTypeOper2 = REG_IND_MEM_OFFSET;
			opr->op2 = m.str(11);
			opr->memOffsetValue = m.str(12);
		}
		return;
	}

	if (m.str(13).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = PC_REL;
			opr->op1 = m.str(14);
		}
		else {
			opr->addrTypeOper2 = PC_REL;
			opr->op2 = m.str(14);
		}
	}

	if (m.str(15).size() > 0) {
		if (num == 0) {
			opr->addrTypeOper1 = ABS;
			opr->op1 = m.str(16);
		}
		else {
			opr->addrTypeOper2 = ABS;
			opr->op2 = m.str(16);
		}
	}
}

bool Parser::checkCorrectness(OPResult * opr) {
	if (opr->addrTypeOper1 == IMM)
		return false;

	if (opr->addrTypeOper1 == VALUE_OF_SYM)
		return false;

	if (opr->addrTypeOper1 != REG_DIR && opr->addrTypeOper2 != REG_DIR) // At least one should be direct register address type.
		return false;

	return true;
}
