#include "Relocation_table.h"



Relocation_table::Relocation_table() {
	first = last = nullptr;
}



Relocation_table::~Relocation_table() {
	
	entry* del = first;
	while (del) {
		first = first->next;
		delete del;
		del = first;
	}
	first = last = nullptr;
}

int Relocation_table::insert(string section, char type, int offset, int sym_index) {

	entry* new_entry = new entry(section, type, offset, sym_index);
	if (first) {
		last->next = new_entry;
		last = last->next;
	}
	else first = last = new_entry;

	return 0;
}
