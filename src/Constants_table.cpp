#include "Constants_table.h"

Constants_table::~Constants_table() {
	
	entry* del = first;
	while (del) {
		first = first->next;
		delete del;
		del = first;
	}
	first = last = nullptr;
}

Constants_table::entry* Constants_table::add_entry(string name) {
	
	if (find(name))return nullptr;;
	entry* new_entry = new entry(name);
	if (first) {
		last->next = new_entry;
		last = last->next;
	}
	else first = last = new_entry;
	return new_entry;
}

Constants_table::entry* Constants_table::find(string name) {

	entry* pom = first;
	for (entry* pom = first; pom; pom = pom->next) {
		if (pom->name == name) return pom;
	}
	return nullptr;
}
