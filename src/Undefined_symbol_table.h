#ifndef _UNDEFINED_SYMBOL_TABLE_H_
#define _UNDEFINED_SYMBOL_TABLE_H_
#pragma once

using namespace std;
#include <iostream>
#include "Constants_table.h"

class Symbol_table;

class Undefined_symbol_table {

private:

	friend class Assembler;

	struct entry {
		Constants_table::entry* ctentry;
		string op1, op2;
		char operation;
		bool op1known, op2known;
		int op1val, op2val;
		char op1type, op2type;
		entry* next;
		entry* prev;
		entry(Constants_table::entry* cte, string o1, string o2, char o, bool k1, bool k2, entry* p, char t1, char t2, int v1, int v2) {
			ctentry = cte;
			op1 = o1;
			op2 = o2;
			operation = o;
			op1known = k1;
			op2known = k2;
			op1type = t1;
			op2type = t2;
			op1val = v1;
			op2val = v2;
			next = nullptr;
			prev = p;
		}
	};

	Symbol_table* sym_table;
	Constants_table* con_table;
	entry* first, * last;
	int size;

	Undefined_symbol_table(Symbol_table* st, Constants_table* ct) { first = last = nullptr; size = 0; sym_table = st; con_table = ct; }
	~Undefined_symbol_table();

	int add_entry(Constants_table::entry* ctentry, string op1, string op2, char operation, bool op1known, bool op2known, char op1type, char op2type, int op1val, int op2val);
	int delete_entry(entry* ent);
	int pass();
	int check_op(string op, bool& opknown, char& optype, int& opval); 
		
};
#endif