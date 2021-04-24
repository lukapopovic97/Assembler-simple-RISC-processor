#include "Symbol_table.h"
#include "Section_table.h"


Symbol_table::Symbol_table() {

	sec_table = new Section_table();
	first = last = new entry(this, "UND", "UND", 0, 0);
	index = 1;
	curr_section = 0;
}

Symbol_table::~Symbol_table() {

	delete sec_table;
	sec_table = nullptr;
	entry* del = first;
	while (del) {
		first = first->next;
		delete del;
		del = first;
	}
	first = last = nullptr;
}

int Symbol_table::add_extern(string sym){
	
	if (find(sym))return -1;
	entry* new_entry = new entry(this, sym, sec_table->sections[0]->name, 0, index);
	new_entry->locality = 2;
	index++;
	if (first) {
		last->next = new_entry;
		last = last->next;
	}
	else first = last = new_entry;
	return 0;
}

int Symbol_table::add_symbol(string sym) {

	if (curr_section == 0)return -1;// Labela ne sme biti u UND sekciji
	entry* found = find(sym);
	if (found) {
		if (found->section != sec_table->sections[0]->name)return -1;
		else {
			found->section = sec_table->sections[curr_section]->name;
			found->offset = sec_table->sections[curr_section]->location_counter;
			return 0;
		}
	}
	entry* new_entry = new entry(this, sym, sec_table->sections[curr_section]->name, sec_table->sections[curr_section]->location_counter, index);
	index++;
	if (first) {
		last->next = new_entry;
		last = last->next;
	}
	else first = last = new_entry;
	return 0;
}

Symbol_table::entry* Symbol_table::find(string sym) {
	entry* pom = first;
	for (entry* pom = first; pom; pom = pom->next) {
		if (pom->name == sym) return pom;
	}
	return nullptr;
}
