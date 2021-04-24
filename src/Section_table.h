#ifndef _SECTION_TABLE_H_
#define _SECTION_TABLE_H_
#pragma once

using namespace std;
#include <iostream>

class Section_table {

private:

	friend class Symbol_table;
	friend class Assembler;

	struct entry {
		string name;
		string rwx;
		int symbol_index;
		int location_counter;
		//int size;
		entry(string n, string bits) {
			name = n;
			rwx = bits;
			location_counter = 0;
			symbol_index = -1;
			//size = 0;
		}
	};

public:

	Section_table();
	~Section_table();
	entry* sections[20];
	void reset_counters();
	int add_section(string name, string bits);
	int find(string name);

};

#endif