#ifndef  _RELOCATION_TABLE_H_
#define _RRELOCATION_TABLE_H_
#pragma once
using namespace std;
#include <iostream>
class Relocation_table {

private:
	
	friend class Assembler;

	struct entry {
		string section;
		char type;
		int offset;
		int sym_index;
		entry* next;
		entry(string s, char t, int o, int i) {
			section = s;
			type = t;
			offset = o;
			sym_index = i;
			next = nullptr;
		}
	};

	entry* first, * last;

public:

	Relocation_table();
	~Relocation_table();
	int insert(string section, char type, int offset, int sym_index);
};

#endif