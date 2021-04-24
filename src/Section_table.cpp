#include "Section_table.h"

Section_table::Section_table() {

	sections[0] = new entry("UND", "r");
	sections[0]->symbol_index = 0;
	sections[1] = new entry(".text", "rx");
	sections[2] = new entry(".data", "rw");
	sections[3] = new entry(".bss", "rw");
	for (int i = 4; i < 20; i++) {
		sections[i] = nullptr;
	}
}

Section_table::~Section_table() {
	for (int i = 0; i < 20; i++) {
		if (sections[i] != nullptr) {
			delete sections[i];
			sections[i] = nullptr;
		}
	}
}

void Section_table::reset_counters(){
	for (int i = 0; i < 20; i++) {
		if (sections[i])sections[i]->location_counter = 0;
	}
}

int Section_table::add_section(string name, string bits) {

	if (bits.length() > 3)return -1;
	for (int i = 0; i < bits.length(); i++) {
		if (bits[i] != 'r' && bits[i] != 'w' && bits[i] != 'x')return -1;
	}
	for (int i = 4; i < 20; i++) {
		if (!sections[i]) {
			sections[i] = new entry(name, bits);
			return i;
		}
	}
	return -1;
}

int Section_table::find(string name) {
	for (int i = 0; i < 20; i++) {
		if (!sections[i])return -1;
		if (sections[i]->name == name)return i;
	}
	return -1;
}