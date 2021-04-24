#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_
#pragma once

using namespace std;
#include <iostream>
#include <map>
#include <unordered_set>
#include <fstream>
#include "Symbol_table.h"
#include "Constants_table.h"

class Relocation_table;
class Constants_table;
class Undefined_symbol_table;

class Assembler {

private:
	
	static map<string, int> section_nums;
	static map<string, string> opcodes;
	string inFileName;
	string outFileName;
	static map<string, string> init_opcodes();
	static map<string, int> init_section_nums();
	unordered_set<string> instructions = { "halt", "xchg", "int", "mov", "add", "sub", "mul", "div", "cmp", "not", "and", "or", "xor", "test", "shl", "shr", "push", "pop", "jmp", "jeq", "jne", "jgt", "call", "ret", "iret" };
	unordered_set<string> sections = { ".text", ".data", ".bss", ".section" };
	unordered_set<string> directives = { ".byte", ".word", ".align", ".skip", ".end", ".equ", ".extern", ".global" };
	Symbol_table* sym_table;
	Relocation_table* rel_table;
	Constants_table* con_table;
	Undefined_symbol_table* undsym_table;
	ofstream outFile;
	string output[20];
	string toLowerCase(string s);
	int opLength(string op, char suf);
	int writeRegister(string op);
	void write8(int val);
	void write16(int val);
	void insert_reloc_a(Symbol_table::entry* ent);
	void insert_reloc_r(Symbol_table::entry* ent, int op2length);
	void writeToFile(string s);
	int process_expression(Constants_table::entry* con_table_entry, string exp);
	int process_expression_operand(string op, bool& opknown, char& optype, int& opval);

public:

	Assembler(string inFile);
	~Assembler();
	void assemble();
	//int check_for_errors(string line);
	int first_pass(string line);
	int second_pass(string line);
	int process_section(string sec, string rest, int pass);
	int process_directive(string dir, string rest, int pass);
	int process_instruction(string ins, string rest, int pass);
	int writeOp(string op, char suf, int op2length);
};

#endif