#include "Undefined_symbol_table.h"
#include "Symbol_table.h"

Undefined_symbol_table::~Undefined_symbol_table() {

	entry* del = first;
	while (del) {
		first = first->next;
		delete del;
		del = first;
	}
	first = last = nullptr;
	sym_table = nullptr;
	con_table = nullptr;
}

int Undefined_symbol_table::add_entry(Constants_table::entry* ctentry, string op1, string op2, char operation, bool op1known, bool op2known, char op1type, char op2type, int op1val, int op2val) {
	entry* new_entry = new entry(ctentry, op1, op2, operation, op1known, op2known, last, op1type, op2type, op1val, op2val);
	if (first) {
		last->next = new_entry;
		last = last->next;
	}
	else first = last = new_entry;
	size++;
	return 0;
}

int Undefined_symbol_table::delete_entry(entry* ent) {
	
	if (!ent)return -1;
	if (ent->prev)ent->prev->next = ent->next;
	if (ent->next)ent->next->prev = ent->prev;
	if (first == ent)first = ent->next;
	if (last == ent)last = ent->prev;
	delete ent;
	size--;
	return 0;
}

int Undefined_symbol_table::pass() {

	while (size) {

		int err = 0;
		entry* iter = first;
		int old_size = size;

		while (iter) {
			if (iter->operation != 'o') {
				if (!iter->op1known)err = check_op(iter->op1, iter->op1known, iter->op1type, iter->op1val);
				if (err != 0)return err;
				if (!iter->op2known)err = check_op(iter->op2, iter->op2known, iter->op2type, iter->op2val);
				if (err != 0)return err;
				if (iter->op1known && iter->op2known) {
					if (iter->op1type == 's' && iter->op2type == 's') {
						Symbol_table::entry* s1 = sym_table->find(iter->op1);
						Symbol_table::entry* s2 = sym_table->find(iter->op2);
						if (!s1 || !s2)return -1;
						if (s1->section != s2->section)return -1;
					}
					else if (iter->op1type == 's' || iter->op2type == 's')return -1;
					iter->ctentry->defined = true;
					int const_value = (iter->operation == '+' ? iter->op1val + iter->op2val : iter->op1val - iter->op2val);
					if (const_value < -32768 || const_value>32767)return -1;
					iter->ctentry->value = const_value;
					iter = iter->next;
					if (iter)delete_entry(iter->prev);
					else delete_entry(last);
				}
				else iter = iter->next;
			}
			else {
				if (!iter->op1known)err = check_op(iter->op1, iter->op1known, iter->op1type, iter->op1val);
				if (err != 0)return err;
				if (iter->op1known) {
					if (iter->op1type == 's')return -1;
					iter->ctentry->defined = true;
					if (iter->op1val < -32768 || iter->op1val>32767)return -1;
					iter->ctentry->value = iter->op1val;
					iter = iter->next;
					if (iter)delete_entry(iter->prev);
					else delete_entry(last);
				}
				else iter = iter->next;
			}
		}

		if (size == old_size)return -1;
	}
	
	return 0;
}

int Undefined_symbol_table::check_op(string op, bool& opknown, char& optype, int& opval) {

	Symbol_table::entry* ste = sym_table->find(op);
	if (ste) {
		if (ste->section == "UND")return -1;
		if (ste->locality == 1) {
			opknown = true;
			optype = 's';
			opval = ste->offset;
			return 0;
		}
		else return -1;
	}
	Constants_table::entry* cte = con_table->find(op);
	if (cte) {
		if (cte->defined) {
			opknown = true;
			opval = cte->value;
		}
		optype = 'c';
		return 0;
	}
	return -1;
}
