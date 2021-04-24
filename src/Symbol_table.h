#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_
#pragma once

using namespace std;
#include <iostream>

class Section_table;

class Symbol_table {

private:

	friend class Assembler;
	friend class Undefined_symbol_table;

	struct entry {
		Symbol_table* sym_table;
		string name;
		string section;
		int offset;
		int locality; //1-local, 2-global
		int index;
		entry* next;
		entry(Symbol_table* s, string n, string c, int o, int i) {
			sym_table = s;
			name = n;
			section = c;
			offset = o;
			locality = 1;
			index = i;
			next = nullptr;
		}
	};

	entry* first, *last;

	int index;
	int curr_section;
	Section_table* sec_table;

public:

	Symbol_table();
	~Symbol_table();

	int add_extern(string sym);
	int add_symbol(string sym);
	entry* find(string sym);
};

#endif