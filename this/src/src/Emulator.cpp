// File: Emulator.cpp
// Created by: Xerox
// Date: 23.05.2018
// Last modified: 05.06.2018.

#include <iostream>
#include <array>
#include <thread>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <chrono>

#include "Emulator.h"
#include "Linker.h"
#include "StringTokenizer.h"
#include "STResult.h"
#include "Assembler.h"
#include "SymbolTable.h"
#include "RelTable.h"

bool endListener;
bool endTimer;
bool interruptHappen;
bool timersInterruptHappen;

char* itoa(unsigned long num, char* str, int base);

void preparePolling() {
    termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~(ICANON | ECHO|0);
    tcsetattr(0, TCSANOW, &t);
}

void finishPolling() {
    termios t;
    tcgetattr(0, &t);
    t.c_lflag |= ICANON | ECHO|0;
    tcsetattr(0, TCSANOW, &t);
}

bool kbhit() {
    int waitingInput;
    ioctl(0, FIONREAD, &waitingInput);
    return waitingInput > 0;
}


void inputListener() {
	preparePolling();
	while (!endListener) {
		while (!kbhit()) {
			if (endListener)
				break;
		}
		
		char c = getchar();
		std::cout << "\n**** Karakter je = " << c << " ****\n" << std::endl;
		interruptHappen = true;
	}
	finishPolling();
}

void timerThread() {
    while(!endTimer) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timersInterruptHappen = true;
    }
}

Emulator::Emulator(const char * filesName[], int numOfObjFiles) {
	this->linker = new Linker();

	this->st = new StringTokenizer();

	this->psw = new PSW();
	this->oldPsw = new PSW();

	this->clockTime = INTERRUPT_TIME;

	for (int i = 0; i < NUM_OF_REG; i++)
		regs[i] = 0;

	SP = START_ADDR_OF_IO_MEM;

	this->end = false;

	this->dividingByZero = false;

    endListener = false;
    endTimer = false;
	interruptHappen = false;
    timersInterruptHappen = false;
}

Emulator::~Emulator() {
	delete linker;
	delete st;
	delete psw;

	linker = nullptr;
	st = nullptr;
	psw = nullptr;


	for (int i = 0; i < Assembler::curNumber; i++) {
		delete Assembler::listOfSymTables[i];
		delete Assembler::listOfRelTables[i];

		Assembler::listOfSymTables[i] = nullptr;
		Assembler::listOfRelTables[i] = nullptr;
	}
	Assembler::curNumber = 0;
}

bool Emulator::execute() {

	for (int i = 0; i < Assembler::curNumber; i++) {
		for (int j = 0; j < Assembler::curNumber; j++) {
			if(((Assembler::listOfSymTables[i]->startAddress <= Assembler::listOfSymTables[j]->startAddress &&
				Assembler::listOfSymTables[i]->startAddress + Assembler::listOfSymTables[i]->allSectionsSize >= Assembler::listOfSymTables[j]->startAddress &&
			    Assembler::listOfSymTables[i]->startAddress + Assembler::listOfSymTables[i]->allSectionsSize <=
				Assembler::listOfSymTables[j]->startAddress + Assembler::listOfSymTables[j]->allSectionsSize) ||

			   (Assembler::listOfSymTables[i]->startAddress <= Assembler::listOfSymTables[j]->startAddress &&
			    Assembler::listOfSymTables[i]->startAddress + Assembler::listOfSymTables[i]->allSectionsSize >=
			    Assembler::listOfSymTables[j]->startAddress + Assembler::listOfSymTables[j]->allSectionsSize) ||

			   (Assembler::listOfSymTables[i]->startAddress >= Assembler::listOfSymTables[j]->startAddress &&
		        Assembler::listOfSymTables[i]->startAddress + Assembler::listOfSymTables[i]->allSectionsSize <=
		        Assembler::listOfSymTables[j]->startAddress + Assembler::listOfSymTables[j]->allSectionsSize) ||

			   (Assembler::listOfSymTables[i]->startAddress >= Assembler::listOfSymTables[j]->startAddress &&
				Assembler::listOfSymTables[i]->startAddress <= Assembler::listOfSymTables[j]->startAddress + Assembler::listOfSymTables[j]->allSectionsSize &&
			    Assembler::listOfSymTables[i]->startAddress + Assembler::listOfSymTables[i]->allSectionsSize >=
			    Assembler::listOfSymTables[j]->startAddress + Assembler::listOfSymTables[j]->allSectionsSize)) && i != j
			) {
				std::cout << "\n**** EMULACIJA NIJE USPELA! SEKCIJE SE PREKLAPAJU! ****\n" << std::endl;
				return false;
			  }
		}
	}

	loadIVTRoutineContentIntoMemory();
	bool succ = checkIfMemCntWillCorrupt();
	if (!succ) {
		std::cout << "\n**** EMULACIJA NIJE USPELA! SISTEMSKA MEMORIJA JE OSTECENJA! ****\n" << std::endl;
		return false;
	}

	succ = linker->execute();
	if (!succ) {
		std::cout << "\n**** EMULACIJA NIJE USPELA! GRESKA PRI LINKOVANJU! ****\n" << std::endl;
		return false;
	}

	PC = linker->indexOfBeginTextSection;
	loadIntoMemory();
	std::string nextToken("");
	std::thread inputThread(inputListener);
    std::thread timer(timerThread);

	while (!end) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
		readFromMemory(nextToken);
		STResult* res = st->parseString(nextToken);

		short a, b;
		long dstAddr;
		switch (res->numOfArgs) {
			case NO_ARGS:	// No operands to be extracted.
				break;
			case UN:		// One operand to be extracted.
				a = exctractOperand(0, res, dstAddr);
				break;
			case BIN:		// Two operands to be extracted.
				a = exctractOperand(0, res, dstAddr);
				b = exctractOperand(1, res, dstAddr);
				break;
		}

		if (!isConditionRight(res->cond)) {
			if (timersInterruptHappen) {
                std::cout << "\n**** Tajmer je otkucao! ****\n" << std::endl;
                timersInterruptHappen = false;
				if (psw->I) {
					writeInStack(PC);
					unsigned short psw1 = insertPSWIntoShort();
					psw->I = false;
					writeInStack(psw1);
					PC = IVT_TABLE[PERIODIC_ROUTINE_ENTRY];
				}
			}
			continue;
		}

		short oldFirstOperandValue = 0;
		switch (res->mnm) {
            case 0: // ADD
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                oldFirstOperandValue = SP;
                                SP += b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue + b);
                                break;
                            case 7: // PC
                                oldFirstOperandValue = PC;
                                PC += b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue + b);
                                break;
                            default:
                                oldFirstOperandValue = regs[res->reg1];
                                regs[res->reg1] += b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue + b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a + b);
                        updatePSWStateZNOC(res->mnm, a, b, a + b);
                        break;
                    default:
                        break;
                }
                break;
            case 1: // SUB
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                oldFirstOperandValue = SP;
                                SP -= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue - b);
                                break;
                            case 7: // PC
                                oldFirstOperandValue = PC;
                                PC -= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue - b);
                                break;
                            default:
                                oldFirstOperandValue = regs[res->reg1];
                                regs[res->reg1] -= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue - b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a - b);
                        updatePSWStateZNOC(res->mnm, a, b, a - b);
                        break;
                    default:
                        break;
                }
                break;
            case 2: // MUL
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                oldFirstOperandValue = SP;
                                SP *= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue * b);
                                break;
                            case 7: // PC
                                oldFirstOperandValue = PC;
                                PC *= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue * b);
                                break;
                            default:
                                oldFirstOperandValue = regs[res->reg1];
                                regs[res->reg1] *= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue * b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a * b);
                        updatePSWStateZN(a * b);
                        break;
                    default:
                        break;
                }
                break;
            case 3: // DIV
                if (b == 0) {
                    if (psw->I) {
                        writeInStack(PC);
                        unsigned short psw1 = insertPSWIntoShort();
                        psw->I = false;
                        writeInStack(psw1);
                        PC = IVT_TABLE[INCORR_ROUTINE_ENTRY];
                    }
                    this->dividingByZero = true;
                    break;
                }

                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                oldFirstOperandValue = SP;
                                SP /= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue / b);
                                break;
                            case 7: // PC
                                oldFirstOperandValue = PC;
                                PC /= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue / b);
                                break;
                            default:
                                oldFirstOperandValue = regs[res->reg1];
                                regs[res->reg1] /= b;
                                updatePSWStateZNOC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue / b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a / b);
                        updatePSWStateZN(a / b);
                        break;
                    default:
                        break;
                }
                break;
            case 4: // CMP
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                updatePSWStateZNOC(res->mnm, SP, b, SP - b);
                                break;
                            case 7: // PC
                                updatePSWStateZNOC(res->mnm, PC, b, PC - b);
                                break;
                            default:
                                updatePSWStateZNOC(res->mnm, regs[res->reg1], b, regs[res->reg1] - b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        updatePSWStateZNOC(res->mnm, a, b, a - b);
                        break;
                    default:
                        break;
                }
                break;
            case 5: // AND
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                SP &= b;
                                updatePSWStateZN(SP);
                                break;
                            case 7: // PC
                                PC &= b;
                                updatePSWStateZN(PC);
                                break;
                            default:
                                regs[res->reg1] &= b;
                                updatePSWStateZN(regs[res->reg1]);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a & b);
                        updatePSWStateZN(a & b);
                        break;
                    default:
                        break;
                }
                break;
            case 6: // OR
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                SP |= b;
                                updatePSWStateZN(SP);
                                break;
                            case 7: // PC
                                PC |= b;
                                updatePSWStateZN(PC);
                                break;
                            default:
                                regs[res->reg1] |= b;
                                updatePSWStateZN(regs[res->reg1]);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a | b);
                        updatePSWStateZN(a | b);
                        break;
                    default:
                        break;
                }
                break;
            case 7: // NOT
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                SP = ~b;
                                updatePSWStateZN(SP);
                                break;
                            case 7: // PC
                                PC = ~b;
                                updatePSWStateZN(PC);
                                break;
                            default:
                                regs[res->reg1] = ~b;
                                updatePSWStateZN(regs[res->reg1]);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, ~b);
                        updatePSWStateZN(~b);
                        break;
                    default:
                        break;
                }
                break;
            case 8: // TEST
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                updatePSWStateZN(SP & b);
                                break;
                            case 7: // PC
                                updatePSWStateZN(PC & b);
                                break;
                            default:
                                updatePSWStateZN(regs[res->reg1] & b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        updatePSWStateZN(a & b);
                        break;
                    default:
                        break;
                }
                break;
            case 9: // PUSH
                switch (res->addrType1) {
                    case IMM:
                        writeInStack(a);
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                writeInStack(SP);
                                break;
                            case 7: // PC
                                writeInStack(PC);
                                break;
                            default:
                                writeInStack(regs[res->reg1]);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInStack(a);
                        break;
                    default:
                        break;
                }
                break;
            case 10: // POP
                switch (res->reg1) {
                    case 6: // SP
                        SP = readFromStack();
                        break;
                    case 7: // PC
                        PC = readFromStack();
                        break;
                    default:
                        regs[res->reg1] = readFromStack();
                        break;
                }
                break;
            case 11: // CALL

                switch (res->addrType1) {
                    case IMM:
                        writeInStack(PC);
                        PC = a;
                        break;
                    case REG_DIR:
                        writeInStack(PC);
                        switch (res->reg1) {
                            case 6: // SP
                                PC = SP;
                                break;
                            case 7: // PC
                                break; // No effect.
                            default:
                                PC = regs[res->reg1];
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInStack(PC);
                        PC = a;
                        break;
                    default:
                        break;
                }
                break;
            case 12: { // IRET
                unsigned short psw = readFromStack();
                PC = readFromStack();
                extractPSWfromShort(psw);

                if (dividingByZero) {
                    std::cout << "\n**** POKUSAJ DELJENJA NULOM! ****\n" << std::endl;
                    return false;
                }
            }
                break;
            case 13: // MOV
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        if (res->addrType2 == IMM && res->reg1 == 7 && b == 0) { // Check if its end.
                            endListener = true;
                            endTimer = true;
                            this->end = true;
                            break;
                        }

                        switch (res->reg1) {
                        case 6: // SP
                            SP = b;
                            updatePSWStateZN(SP);
                            break;
                        case 7: // PC
                            PC = b;
                            updatePSWStateZN(PC);
                            break;
                        default:
                            regs[res->reg1] = b;
                            updatePSWStateZN(regs[res->reg1]);
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, b);
                        updatePSWStateZN(b);

                        if ((unsigned short)dstAddr == OUTPUT_REG) {
                            if (b == 0x10)
                                std::cout << std::endl;
                            else
                                std::cout << b << std::endl;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case 14: // SHL
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                oldFirstOperandValue = SP;
                                SP <<= b;
                                updatePSWStateC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue << b);
                                updatePSWStateZN(oldFirstOperandValue << b);
                                break;
                            case 7: // PC
                                oldFirstOperandValue = PC;
                                PC <<= b;
                                updatePSWStateC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue << b);
                                updatePSWStateZN(oldFirstOperandValue << b);
                                break;
                            default:
                                oldFirstOperandValue = regs[res->reg1];
                                regs[res->reg1] <<= b;
                                updatePSWStateC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue << b);
                                updatePSWStateZN(oldFirstOperandValue << b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a << b);
                        updatePSWStateC(res->mnm, a, b, a << b);
                        updatePSWStateZN(a << b);
                        break;
                    default:
                        break;
                }
                break;
            case 15: // SHR
                switch (res->addrType1) {
                    case IMM: // Cannot happen.
                        break;
                    case REG_DIR:
                        switch (res->reg1) {
                            case 6: // SP
                                oldFirstOperandValue = SP;
                                SP >>= b;
                                updatePSWStateC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue >> b);
                                updatePSWStateZN(oldFirstOperandValue >> b);
                                break;
                            case 7: // PC
                                oldFirstOperandValue = PC;
                                PC >>= b;
                                updatePSWStateC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue >> b);
                                updatePSWStateZN(oldFirstOperandValue >> b);
                                break;
                            default:
                                oldFirstOperandValue = regs[res->reg1];
                                regs[res->reg1] >>= b;
                                updatePSWStateC(res->mnm, oldFirstOperandValue, b, oldFirstOperandValue >> b);
                                updatePSWStateZN(oldFirstOperandValue >> b);
                                break;
                        }
                        break;
                    case ADDR_TYPE::MEM: case REG_IND_CONST_OFFSET:
                        writeInMemory(dstAddr, a >> b);
                        updatePSWStateC(res->mnm, a, b, a >> b);
                        updatePSWStateZN(a >> b);
                        break;
                    default:
                        break;
                }
                break;
		}

		if (interruptHappen) {
			interruptHappen = false;
			if (psw->I) {
				
				writeInStack(PC);
				unsigned short psw1 = insertPSWIntoShort();
				psw->I = false;
				writeInStack(psw1);
				PC = IVT_TABLE[INPUT_ROUTINE_ENTRY];
			}
		}

		if (timersInterruptHappen) {
            std::cout << "\n**** Tajmer je otkucao! ****\n" << std::endl;
            timersInterruptHappen = false;
			if (psw->I) {
				writeInStack(PC);
				unsigned short psw1 = insertPSWIntoShort();
				psw->I = false;
				writeInStack(psw1);
				PC = IVT_TABLE[PERIODIC_ROUTINE_ENTRY];
			}
		}
	}

	std::cout << "\n**** EMULACIJA USPELA ****\n" << std::endl;
	inputThread.join();
	timer.join();
	return true;
}

bool Emulator::checkIfMemCntWillCorrupt() {
	for (int i = 0; i < Assembler::curNumber; i++) {
		if (Assembler::listOfSymTables[i]->startAddress * 2 <= endAddressOfIVTRoutineContent)
			return false;
	}

	return true;
}

long Emulator::exctractOperand(int index, STResult * res, long& dstAddr) {
	std::string nextToken;
	if (index == 0) {
		switch (res->addrType1) {
			case IMM:
				readFromMemory(nextToken);
				return st->getValue(nextToken);
			case REG_DIR: // No effect.
				break;
			case ADDR_TYPE::MEM:
				readFromMemory(nextToken);
				dstAddr = st->getValue(nextToken);
				return getValueFromMemory(dstAddr);
			case REG_IND_CONST_OFFSET:
				readFromMemory(nextToken);

				switch (res->reg1) {
					case 6: // SP
						dstAddr = st->getValue(nextToken) + SP;
						return getValueFromMemory(dstAddr);
					case 7: // PC
						dstAddr = st->getValue(nextToken) + PC;
						return getValueFromMemory(dstAddr);
					default:
						dstAddr = st->getValue(nextToken) + regs[res->reg1];
						return getValueFromMemory(dstAddr);
				}
				break;
            default:
                break;
		}
	}
	else {
		switch (res->addrType2) {
			case IMM:
				readFromMemory(nextToken);
				return st->getValue(nextToken);
			case REG_DIR:
				return regs[res->reg2];
			case ADDR_TYPE::MEM:
				readFromMemory(nextToken);
				return getValueFromMemory(st->getValue(nextToken));
			case REG_IND_CONST_OFFSET:
				readFromMemory(nextToken);

				switch (res->reg2) {
					case 6: // SP
						return getValueFromMemory(st->getValue(nextToken) + SP);
					case 7: // PC
						return getValueFromMemory(st->getValue(nextToken) + PC);
					default:
						return getValueFromMemory(st->getValue(nextToken) + regs[res->reg2]);
				}
				break;
            default:
                break;
		}
	}

	return 0;
}

long Emulator::getValueFromMemory(unsigned short off) {
	long offset = off * 2;
	char* data = new char[5];
	data[2] = MEM[offset];
	data[3] = MEM[offset + 1];
	data[0] = MEM[offset + 2];
	data[1] = MEM[offset + 3];
	data[4] = '\0';

	long valTemp = strtol(data, 0, 16);
	if (valTemp > 0x7fff)
		valTemp -= 0x10000;
	valTemp &= ~(~0L << 16);
	delete data;

	return valTemp;
}

void Emulator::writeInMemory(unsigned short off, long newValue) {
	long offset = off * 2;
	char* num = new char[5];

	num = itoa(newValue, num, 16);
	std::string str(num);

	switch (str.size()) {
		case 1:
			str = "000" + str;
			break;
		case 2:
			str = "00" + str;
			break;
		case 3:
			str = "0" + str;
			break;
	}

	MEM[offset] = str[2];
	MEM[offset + 1] = str[3];
	MEM[offset + 2] = str[0];
	MEM[offset + 3] = str[1];

	delete num;
}

void Emulator::writeInStack(long newValue) {
	char* num = new char[5];

	num = itoa(newValue, num, 16);
	std::string str(num);

	switch (str.size()) {
	case 1:
		str = "000" + str;
		break;
	case 2:
		str = "00" + str;
		break;
	case 3:
		str = "0" + str;
		break;
	}

	SP -= 2;
	MEM[SP * 2 + 3] = str[1];
	MEM[SP * 2 + 2] = str[0];
	MEM[SP * 2 + 1] = str[3];
	MEM[SP * 2] = str[2];

	delete num;
}

short Emulator::readFromStack() {
	char* data = new char[5];

	data[2] = MEM[SP * 2];
	data[3] = MEM[SP * 2 + 1];
	data[0] = MEM[SP * 2 + 2];
	data[1] = MEM[SP * 2 + 3];
	data[4] = '\0';
	SP += 2;

	long valTemp = strtol(data, 0, 16);
	if (valTemp > 0x7fff)
		valTemp -= 0x10000;
	valTemp &= ~(~0L << 16);
	delete data;

	return (short)valTemp;
}

void Emulator::extractPSWfromShort(unsigned short psw) {
	if (psw & (1 << 15))
		this->psw->I = true;
	else
		this->psw->I = false;

	if (psw & (1 << 3))
		this->psw->N = true;
	else
		this->psw->N = false;

	if (psw & (1 << 2))
		this->psw->C = true;
	else
		this->psw->C = false;

	if (psw & 2)
		this->psw->V = true;
	else
		this->psw->V = false;

	if (psw & 1)
		this->psw->Z = true;
	else
		this->psw->Z = false;
}

unsigned short Emulator::insertPSWIntoShort() {
	unsigned short psw1 = 0;
	if (psw->I)
		psw1 |= 1 << 15;

	if (psw->N)
		psw1 |= 1 << 3;

	if (psw->C)
		psw1 |= 1 << 2;

	if (psw->V)
		psw1 |= 2;

	if (psw->Z)
		psw1 |= 1;
	return psw1;
}

void Emulator::readFromMemory(std::string & nextToken) {
	char* data = new char[5];
	data[2] = MEM[PC * 2];
	data[3] = MEM[PC * 2 + 1];
	data[0] = MEM[PC * 2 + 2];
	data[1] = MEM[PC * 2 + 3];
	data[4] = '\0';
	PC += 2;

	nextToken = data;
	delete data;
}

bool Emulator::isConditionRight(CONDITION cond) {
	switch (cond) {
		case EQ:
			if (psw->Z)
				return true;
			else
				return false;
		case NE:
			if (!psw->Z)
				return true;
			else
				return false;
		case GT:
			if (!((psw->V ^ psw->N) || psw->Z))
				return true;
			else
				return false;
		case AL:
			return true;
	}

	return false; // Cannot happen.
}

void Emulator::updatePSWStateZNOC(short mnm, short firstOperand, short secondOperand, short res) {
	updatePSWStateZN(res);
	updatePSWStateC(mnm, firstOperand, secondOperand, res);

	switch (mnm) {
		case 0: // ADD
			if ((firstOperand < 0 && secondOperand < 0 && res >= 0) || (firstOperand > 0 && secondOperand > 0 && res <= 0))
				psw->V = true;
			else
				psw->V = false;
			break;
		case 1: case 4: // SUB, CMP
			if((firstOperand < 0 && secondOperand > 0 && res >= 0) || (firstOperand > 0 && secondOperand < 0 && res <= 0))
				psw->V = true;
			else
				psw->V = false;
			break;
	}
}

void Emulator::updatePSWStateC(short mnm, short firstOperand, short secondOperand, short res) {
	switch (mnm) {
		case 0: {
			long res1 = (long)firstOperand + (long)secondOperand;
			if (res1 & (1L << 16))
				psw->C = true;
			else
				psw->C = false;
		}
			break;
		case 1: case 4: // SUB, CMP
			if(firstOperand < secondOperand)
				psw->C = true;
			else
				psw->C = false;
			break;
		case 14: // SHL
			if (firstOperand & (1 << 15))
				psw->C = true;
			else
				psw->C = false;
			break;
		case 15: // SHR
			if (firstOperand & 1)
				psw->C = true;
			else
				psw->C = false;
			break;
	}
}

void Emulator::loadIVTRoutineContentIntoMemory() {
	short startAddress = BEGIN_OF_INTER_ROUTINES;
	short startAddressIVTTable = 0;

	for (int i = 0; i < NUM_OF_INTERRUPT_ROUTINES; i++) {
		char* destination = &MEM[startAddress];
		strcpy(destination, IVTRoutineContent[i]);
		writeInMemory(startAddressIVTTable, startAddress / 2);

		IVT_TABLE[i] = startAddress / 2;
		startAddressIVTTable += 2;
		startAddress += strlen(IVTRoutineContent[i]);
	}

	endAddressOfIVTRoutineContent = startAddress;
}

void Emulator::updatePSWStateZN(short res) {
	if (res < 0)
		psw->N = true;
	else
		psw->N = false;

	if (res == 0)
		psw->Z = true;
	else
		psw->Z = false;
}

void Emulator::loadIntoMemory() {
	for (int i = 0; i < Assembler::curNumber; i++) {
		unsigned long startAddress = Assembler::listOfSymTables[i]->startAddress * 2;
		unsigned long curSectionSize = 0;
		for (int j = 1; j < MAX_NUM_OF_SECTION; j++) {
			if (Assembler::orderOfListSections[i * (MAX_NUM_OF_SECTION - 1) + j - 1] != UND) {
				char* destination = &MEM[startAddress + curSectionSize];
				int firstIndex = i * (MAX_NUM_OF_SECTION - 1) + j - 1;
				int secondIndex = i * (MAX_NUM_OF_SECTION - 1);
				strcpy(destination, Assembler::listOfSecCont[Assembler::orderOfListSections[firstIndex] - 1 + secondIndex]->c_str());
				curSectionSize += Assembler::listOfSecCont[Assembler::orderOfListSections[firstIndex] - 1 + secondIndex]->size();
			}
		}
	}
}
