#ifndef _CONSTANTS_TABLE_H_
#define _CONSTANTS_TABLE_H_
#pragma once

using namespace std;
#include <iostream>

class Constants_table {

private:
	
	friend class Assembler;
	friend class Undefined_symbol_table;

	struct entry {
		string name;
		int value;
		bool defined;
		entry* next;
		entry(string n) {
			name = n; value = 0; defined = false; next = nullptr;
		}
	};

	entry* first, * last;

public:

	Constants_table() { first = last = nullptr; }
	~Constants_table();
	entry* add_entry(string name);
	entry* find(string name);
	
};

#endif

