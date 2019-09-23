#ifndef _MACROS_H_
#define _MACROS_H_

// File: Macros.css
// Created by: Xerox
// Date: 06.05.2018
// Last modified: 05.06.2018.

#include <string>

// Symbol table type alias and enums.
#define NUM_OF_ATTR 3
#define UNDEFINED_VALUE -1

#define NUM_OF_INSRUCTION_OPCODES 16
#define NUM_OF_COND_TYPE 4
#define NUM_OF_ADDR_TYPE 4
#define NUM_OF_PSEUDO_OPCODES 2

#define OP_CODE_LENGTH 2
#define OFFSET_LENGTH 2

#define MAX_NUM_OF_SECTION 5
#define MAX_SECTION_SIZE 2048
#define MAX_NUM_OF_INPUT_FILES 15
#define MAX_FILE_NAME_LENGTH 128


// Macroes for emulator.
#define MEM_SIZE 65536 * 2
#define NUM_OF_REG 6

#define NUM_OF_INTERRUPT_ROUTINES 8
#define INTERRUPT_TIME 10

#define NO_ARG_OP_DURATION 0.05
#define UN_OP_DURATION 0.1
#define BIN_OP_DURATION 0.2

#define OUTPUT_REG 0xFFFE
#define INPUT_REG 0xFFFC

#define BEGIN_OF_INTER_ROUTINES 32

#define START_ROUTINE_ENTRY 0
#define PERIODIC_ROUTINE_ENTRY 1
#define INCORR_ROUTINE_ENTRY 2
#define INPUT_ROUTINE_ENTRY 3

#define START_ADDR_OF_IO_MEM 0xFF00
#define STACK_SEGMENT_ST_ADDR 0xFFB0

typedef unsigned long SER_NUM;
typedef unsigned long SER_NUM_SEED;
typedef std::string SYM_NAME;
typedef unsigned long VALUE;
typedef unsigned long SIZE;

enum SECTION {
	UND, TEXT, DATA, BSS, RODATA
};
enum VISIBILITY {
	GLOBAL, LOCAL
};
enum ATTR {
	AW, AWP, AXP, ARP, NO_ATTR
};

// Relocation table type alias and enums
typedef unsigned long OFFSET;

enum REL_TYPE {
	R_386_32, R_386_PC32
};

enum STATUS {
	LABEL, LABEL_OP, OP, SECT, IMPORT_EXPORT, DIRECTIVE, LABEL_DIR, ERROR
};

enum DIR_TYPE {
	CHAR, WORD, LONG, ALIGN, SKIP
};

enum CONDITION {
	EQ, NE, GT, AL
};

enum ADDR_TYPE {
	IMM, REG_DIR, MEM, REG_IND_CONST_OFFSET, REG_IND_MEM_OFFSET, VALUE_OF_SYM, PC_REL, ABS
};

enum OP_TYPE {
	NO_ARGS, UN, BIN
};

#endif



