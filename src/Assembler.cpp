#include "Assembler.h"
#include <string>
#include <sstream>
#include <regex>
#include <bitset>
#include "Section_table.h"
#include "Relocation_table.h"
#include "Undefined_symbol_table.h"

map<string, string> Assembler::opcodes = Assembler::init_opcodes();
map<string, int> Assembler::section_nums = Assembler::init_section_nums();

map<string, string> Assembler::init_opcodes() {

	map <string,string> opcodes;
	opcodes["halt"] = "00001";
	opcodes["xchg"] = "00010";
	opcodes["int"] = "00011";
	opcodes["mov"] = "00100";
	opcodes["add"] = "00101";
	opcodes["sub"] = "00110";
	opcodes["mul"] = "00111";
	opcodes["div"] = "01000";
	opcodes["cmp"] = "01001";
	opcodes["not"] = "01010";
	opcodes["and"] = "01011";
	opcodes["or"] = "01100";
	opcodes["xor"] = "01101";
	opcodes["test"] = "01110";
	opcodes["shl"] = "01111";
	opcodes["shr"] = "10000";
	opcodes["push"] = "10001";
	opcodes["pop"] = "10010";
	opcodes["jmp"] = "10011";
	opcodes["jeq"] = "10100";
	opcodes["jne"] = "10101";
	opcodes["jgt"] = "10110";
	opcodes["call"] = "10111";
	opcodes["ret"] = "11000";
	opcodes["iret"] = "11001";

	return opcodes;
}

map<string, int> Assembler::init_section_nums() {

	map<string, int> section_nums;
	section_nums["UND"] = 0;
	section_nums[".text"] = 1;
	section_nums[".data"] = 2;
	section_nums[".bss"] = 3;
	return section_nums;
}

string Assembler::toLowerCase(string s) {
	transform(s.begin(), s.end(), s.begin(), ::tolower);
		return s;
}

int Assembler::opLength(string op, char suf) {

	regex immed_dec("^[-]?[0-9]{1,5}$");
	regex immed_hex("^0x[a-f0-9]{1,4}$");
	regex immed_sym("^&[\\.]?[a-z]+[\\w]?$");
	regex regdir("^r[0-7]{1}[hl]?$|^pc[hl]?$|^sp[hl]?$|^psw[hl]?$");
	regex regind_sym("^r[0-7]{1}\\[[\\.]?[a-z]+[\\w]?\\]$|^pc\\[[\\.]?[a-z]+[\\w]?\\]$|^sp\\[[\\.]?[a-z]+[\\w]?\\]$|^psw\\[[\\.]?[a-z]+[\\w]?\\]$");
	regex regind_dec("^r[0-7]{1}\\[[-]?[0-9]{1,5}\\]$|^pc\\[[-]?[0-9]{1,5}\\]$|^sp\\[[-]?[0-9]{1,5}\\]$|^psw\\[[-]?[0-9]{1,5}\\]$");
	regex regind_hex("^r[0-7]{1}\\[0x[0-9a-f]{1,4}\\]$|^pc\\[0x[0-9a-f]{1,4}\\]$|^sp\\[0x[0-9a-f]{1,4}\\]$|^psw\\[0x[0-9a-f]{1,4}\\]$");
	regex pc_rel("^\\$[\\.]?[a-z]+[\\w]?$");
	regex abs("^\\*[0-9]{1,5}$|^\\*0x[a-f0-9]{1,4}$");
	regex abs_dec("^\\*[0-9]{1,5}$");
	regex abs_hex("^\\*0x[a-f0-9]{1,4}$");
	regex abs_sym("^[\\.]?[a-z]+[\\w]?$");
	smatch matches;

	if (regex_search(op, matches, immed_dec)) {		//podrazumevano 16bitna velicina
		int val = stoi(op, 0, 10);
		if (val < -32768 || val>32767)return -1;
		if (val >= -128 && val <= 127) {
			if (suf == 'b')return 2;
			else return 3;
		}
		else {
			if (suf == 'b')return -1;
			else return 3;
		}
	}
	if (regex_search(op, matches, immed_hex)) {
		int val = stoi(op, 0, 0);
		if (val > 65535)return -1;
		if (val <= 255) {
			if (suf == 'b')return 2;
			else return 3;
		}
		else {
			if (suf == 'b')return -1;
			else return 3;
		}
	}
	if (regex_search(op, matches, immed_sym)) {
		if (suf == 'b')return -1;			//Simbol uvek ima 16-bitnu vrednost
		return 3;
	}
	if (regex_search(op, matches, regdir)) {
		if (suf == 'b') {
			if (op[op.length() - 1] != 'h' && op[op.length() - 1] != 'l')return -1;
		}
		else {
			if (op[op.length() - 1] == 'h' || op[op.length() - 1] == 'l')return -1;
		}
		return 1;
	}

	if (regex_search(op, matches, regind_dec)) {
		int pos = op.find('[') + 1;
		int len = op.find(']') - pos;
		string num = op.substr(pos, len);
		int val = stoi(num, 0, 10);
		if (val < -32768 || val>32767)return -1;
		if (val == 0)return 1;
		if (val >= -128 && val <= 127)return 2;
		else return 3;
	}
	if (regex_search(op, matches, regind_hex)) {
		int pos = op.find('[') + 1;
		int len = op.find(']') - pos;
		string num = op.substr(pos, len);
		int val = stoi(num, 0, 16);
		if (val > 65535)return -1;
		if (val == 0)return 1;
		if (val <= 255)return 2;
		else return 3;
	}
	if (regex_search(op, matches, regind_sym)) {
		return 3;
	}
	if (regex_search(op, matches, pc_rel)) {
		return 3;
	}
	if (regex_search(op, matches, abs_dec)) {
		return 3;
	}
	if (regex_search(op, matches, abs_hex)) {
		return 3;
	}
	if (regex_search(op, matches, abs_sym)) {
		return 3;
	}
	return -1;
}

int Assembler::writeRegister(string op) {
	if (op == "r0") {
		output[sym_table->curr_section].append("0000");
		return 0;
	}
	if (op == "r1") {
		output[sym_table->curr_section].append("0001");
		return 0;
	}
	if (op == "r2") {
		output[sym_table->curr_section].append("0010");
		return 0;
	}
	if (op == "r3") {
		output[sym_table->curr_section].append("0011");
		return 0;
	}
	if (op == "r4") {
		output[sym_table->curr_section].append("0100");
		return 0;
	}
	if (op == "r5") {
		output[sym_table->curr_section].append("0101");
		return 0;
	}
	if (op == "r6" || op == "sp") {
		output[sym_table->curr_section].append("0110");
		return 0;
	}
	if (op == "r7" || op == "pc") {
		output[sym_table->curr_section].append("0111");
		return 0;
	}
	if (op == "psw") {
		output[sym_table->curr_section].append("1111");
		return 0;
	}
	return -1;
}

void Assembler::write8(int val) {
	bitset<8>x(val);
	output[sym_table->curr_section].append(x.to_string() + " ");
}

void Assembler::write16(int val) {
	bitset<16>y(val);
	string bits = y.to_string();
	output[sym_table->curr_section].append(bits.substr(8, 8) + " ");
	output[sym_table->curr_section].append(bits.substr(0, 8) + " ");
}

void Assembler::insert_reloc_a(Symbol_table::entry* ent) {

	if (ent->locality == 1) {  //local
		rel_table->insert(sym_table->sec_table->sections[sym_table->curr_section]->name, 'a', sym_table->sec_table->sections[sym_table->curr_section]->location_counter, sym_table->find(ent->section)->index);
		write16(ent->offset);
	}
	else {  //global
		rel_table->insert(sym_table->sec_table->sections[sym_table->curr_section]->name, 'a', sym_table->sec_table->sections[sym_table->curr_section]->location_counter, ent->index);
		write16(0);
	}
	sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
}

void Assembler::insert_reloc_r(Symbol_table::entry* ent, int op2length) {
	if (ent->locality == 1) {  //local
		if (sym_table->sec_table->sections[sym_table->curr_section]->name != ent->section) {
			rel_table->insert(sym_table->sec_table->sections[sym_table->curr_section]->name, 'r', sym_table->sec_table->sections[sym_table->curr_section]->location_counter, sym_table->find(ent->section)->index);
			write16(ent->offset - 2 - op2length);
		}		
		else {
			write16(ent->offset - 2 - op2length - sym_table->sec_table->sections[sym_table->curr_section]->location_counter);
		}
	}
	else {  //global
		rel_table->insert(sym_table->sec_table->sections[sym_table->curr_section]->name, 'r', sym_table->sec_table->sections[sym_table->curr_section]->location_counter, ent->index);
		write16(-2-op2length);
	}
	sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
	//cout << endl << endl << ent->name << endl << endl;
}

void Assembler::writeToFile(string s){

	istringstream instr(s);
	string wr, wr1, wr2;
	int a, b;
	while (instr >> wr) {
		wr1 = wr.substr(0, 4);
		wr2 = wr.substr(4, 4);
		a = stoi(wr1, 0, 2);
		b = stoi(wr2, 0, 2);
		outFile << hex << a;
		outFile << hex << b;
	}
	outFile << endl;
}

int Assembler::process_expression(Constants_table::entry* con_table_entry, string exp) {
	
	int pos = -1, err = 0, op1val = 0, op2val = 0;
	char operation;
	string op1 = "", op2 = "", other;
	bool op1known = false, op2known = false;
	char op1type = 'u', op2type = 'u';

	if ((pos = exp.find('+')) > 0)operation = '+';
	else if ((pos=exp.find('-'))>0)operation = '-';
	else operation = 'o';

	if (operation!='o') {
		exp.replace(pos, 1, " ");
		istringstream instr(exp);
		if (!(instr >> op1)) return -1;
		if (!(instr >> op2)) return -1;
		if (instr >> other) return -1;

		err = process_expression_operand(op1, op1known, op1type, op1val);
		if (err != 0)return err;
		err = process_expression_operand(op2, op2known, op2type, op2val);
		if (err != 0)return err;

		if (op1known && op2known) {
			if (op1type == 's' && op2type == 's') {
				Symbol_table::entry* s1 = sym_table->find(op1);
				Symbol_table::entry* s2 = sym_table->find(op2);
				if (!s1 || !s2)return -1;
				if (s1->section != s2->section)return -1;
			}
			else if (op1type == 's' || op2type == 's')return -1;
			con_table_entry->defined = true;
			int const_value = (operation == '+' ? op1val + op2val : op1val - op2val);
			if (const_value < -32768 || const_value>32767)return -1;
			con_table_entry->value = const_value;
			return 0;
		}
		else {
			undsym_table->add_entry(con_table_entry, op1, op2, operation, op1known, op2known, op1type,op2type , op1val, op2val);
			return 0;
		}
	}
	else {
		istringstream instr(exp);
		if (!(instr >> op1))return-1;
		if (instr >> other)return -1;

		err = process_expression_operand(op1, op1known, op1type, op1val);
		if (err != 0)return err;

		if (op1known) {
			if (op1type == 's')return -1;
			con_table_entry->defined = true;
			if (op1val < -32768 || op1val > 32767)return -1;
			con_table_entry->value = op1val;
			return 0;
		}
		else {
			undsym_table->add_entry(con_table_entry, op1, op2, operation, op1known, op2known, op1type, op2type, op1val, op2val);
			return 0;
		}
	}
	return 0;
}

int Assembler::process_expression_operand(string op, bool& opknown, char& optype, int& opval) {

	regex symbol("^[\\.]?[a-z]+[\\w]?$");
	regex hex("^0x[0-9a-f]{1,4}$");
	regex dec("^[-]?[0-9]{1,5}$");
	smatch matches;

	if (regex_search(op, matches, dec)) {
		opknown = true;
		optype = 'd';
		opval = stoi(op, 0, 10);
		if (opval < -32768 || opval>32767)return -1;
	}
	else if (regex_search(op, matches, hex)) {
		opknown = true;
		optype = 'h';
		opval = stoi(op, 0, 16);
		if (opval > 65535)return -1;
		if (opval > 32767)opval -= 65536;
	}
	else if (regex_search(op, matches, symbol)) {
		Symbol_table::entry* ste = sym_table->find(op);
		if (ste) {
			if (ste->section == sym_table->sec_table->sections[0]->name)return -1;
			if (ste->locality == 1) {
				opknown = true;
				optype = 's';
				opval = ste->offset;
			}
			else return -1;
		}
		else {
			Constants_table::entry* cte = con_table->find(op);
			if (cte) {
				if (cte->defined) {
					opknown = true;
					opval = cte->value;
				}
				optype = 'c';
			}
		}
	}
	else return -1;
	return 0;
}

Assembler::Assembler(string inFile) {
	
	inFileName = inFile;
	inFileName.append(".txt");
	sym_table = new Symbol_table();
	rel_table = new Relocation_table();
	con_table = new Constants_table();
	undsym_table = new Undefined_symbol_table(sym_table, con_table);
	outFile.open(inFile.append("_output.txt"), ios::trunc);
	if (outFile.fail())cout << "Failed to open outfile" << endl;
}

Assembler::~Assembler() {

	delete sym_table;
	sym_table = nullptr;
	delete rel_table;
	rel_table = nullptr;
	delete con_table;
	con_table = nullptr;
	delete undsym_table;
	undsym_table = nullptr;
	outFile.close();
}

 void Assembler::assemble() {
	
	ifstream inFile;
	inFile.open(inFileName);

	if (inFile.fail()) {
		cerr << "Error while opening file '" << inFileName << "'!" << endl;
		exit(1);
	}
	
	string line;
	int count = 0;
	int err=0;

	while (!inFile.eof()) {
		getline(inFile, line);
		count++;
		err = first_pass(line);
		if (err == 9999)break;
		if (err != 0) {
			cerr << "Error on line " << count << "!" << endl;
			cerr << line << endl;
			exit(2);
		}
		//cout << line << endl;
	}

	err = undsym_table->pass();
	if (err != 0) {
		cerr << "Error in constant definition " << endl;
		exit(4);
	}

	inFile.seekg(0);
	if (inFile.fail()) {
		cerr << "Error in positioning in file " << inFileName << "'!" << endl;
		exit(1);
	}

	count = 0;
	err = 0;
	sym_table->sec_table->reset_counters();
	sym_table->curr_section = 0;

	while (!inFile.eof()) {
		getline(inFile, line);
		count++;
		err = second_pass(line);
		if (err == 9999)break;
		if (err != 0) {
			cerr << "Error on line " << count << "!" << endl;
			cerr << line << endl;
			exit(3);
		}
		//cout << line << endl;
	}

	for (int i = 0; i < 20; i++) {
		if (sym_table->sec_table->sections[i]) {
			outFile << sym_table->sec_table->sections[i]->name << endl;
			/*
			istringstream is(output[i]);
			string out;
			int nl = 0;
			while (is >> out) {
				outFile << out << " ";
				nl++;
				if (nl % 8 == 0) outFile << endl;
			}
			*/
			writeToFile(output[i]);
			outFile << endl << endl;
		} 
	}
	outFile << dec << endl;
	
	outFile << "symbol table" << endl;
	for (Symbol_table::entry* e = sym_table->first; e; e = e->next) {
		outFile << e->index << " " << e->name << " " << e->section << " " << e->offset << " " << e->locality << endl;
	}
	outFile << endl << endl;

	outFile << "relocation table" <<endl;
	for (Relocation_table::entry* e = rel_table->first; e; e = e->next) {
		outFile << e->section << " " << e->type << " " << e->offset << " " << e->sym_index << endl;
	}
	outFile << endl << endl;
	
	outFile << "constants table" << endl;
	for (Constants_table::entry* pom = con_table->first; pom; pom = pom->next) {
		outFile << pom->name << " " << pom->value << " " << pom->defined << endl;
	}
	outFile << endl << endl;

	outFile << "section table:" << endl;
	for (int i = 0; i < 20; i++) {
		if (!sym_table->sec_table->sections[i])break;
		outFile << sym_table->sec_table->sections[i]->name << " " << sym_table->sec_table->sections[i]->rwx << " "
			<< sym_table->sec_table->sections[i]->symbol_index << " " << sym_table->sec_table->sections[i]->location_counter << endl;
	}
/*
	outFile << "TEXT:" << endl;
	writeToFile(output[1]);
	outFile << endl << endl << endl;
	outFile << "DATA:" << endl;
	writeToFile(output[2]);
	outFile << endl << endl << endl;
	for (Symbol_table::entry* e = sym_table->first; e; e = e->next) {
		outFile << e->index << " " << e->name << " " << e->section << " " << e->offset << " " << e->locality << endl;
	}
	outFile << endl << endl << endl;
	for (Relocation_table::entry* e = rel_table->first; e; e = e->next) {
		outFile << e->section << " " << e->type << " " << e->offset << " " << e->sym_index << endl;
	}

	*/

	cout << "Completed!" << endl;
	outFile.close();
}

int Assembler::first_pass(string line){

	regex non_blank("[^[:space:]]");
	smatch non_blanks;
	if (!regex_search(line, non_blanks, non_blank)) {
	//	cout << "empty line" << endl;
		return 0;
	}

	istringstream instr(line);
	string str;
	instr >> str;

	regex label("^[a-z]+[\\w]?:$");
	smatch match;
	if (regex_search(str, match, label)) {
		if (sym_table->curr_section == 0)return -1;
		str = match.str();
		str = str.substr(0, str.length() - 1);
		if (con_table->find(str))return -1;
		if (sym_table->add_symbol(str) != 0)return -1;
 		if (!(instr >> str)) {
			return 0;
		}
	}
	
	string rest;
	getline(instr, rest);


	if (directives.find(str) != directives.end()) {
		return process_directive(str, rest, 1);
	}

	if (sections.find(str) != sections.end()) {
		return process_section(str, rest, 1);
	}
	
	string without_suffix = str.substr(0, str.length() - 1);
	if ((instructions.find(str) != instructions.end()) || (instructions.find(without_suffix) != instructions.end())) {
		return process_instruction(str, rest, 1);
	}
	
	return -1; //error
 }

int Assembler::second_pass(string line) {

	regex non_blank("[^[:space:]]");
	smatch non_blanks;
	if (!regex_search(line, non_blanks, non_blank)) {
		//	cout << "empty line" << endl;
		return 0;
	}

	istringstream instr(line);
	string str;
	instr >> str;

	regex label("^[a-z]+[\\w]?:$");
	smatch match;
	if (regex_search(str, match, label)) {
		if (!(instr >> str)) {
			return 0;
		}
	}

	string rest;
	getline(instr, rest);


	if (directives.find(str) != directives.end()) {
		return process_directive(str, rest, 2);
	}

	if (sections.find(str) != sections.end()) {
		return process_section(str, rest, 2);
	}

	string without_suffix = str.substr(0, str.length() - 1);
	if ((instructions.find(str) != instructions.end()) || (instructions.find(without_suffix) != instructions.end())) {
		return process_instruction(str, rest, 2);
	}

	return -1; //error
}

int Assembler::process_section(string sec, string rest, int pass) {

	regex check("[^[:space:]]");
	regex section_regex("^\\.[a-z]+[\\w]?$");
	smatch matches;

	if (sec != ".section") {
		//cout << "if" << endl;
		if (regex_search(rest, matches, check)) {
			cout << "error" << endl;
			return -1;
		}
		if (sec == "UND")return -1;
		sym_table->curr_section = section_nums[sec];
		if (pass == 1) {
			sym_table->add_symbol(sec);
			sym_table->sec_table->sections[section_nums[sec]]->symbol_index = sym_table->find(sec)->index;
		}
		//cout << "ok" << endl;
		return 0;
	}
	else {
		//cout << "else" << endl;
		string real_sec;
		istringstream instr(rest);
		
		if (instr >> real_sec) {
			if (real_sec != ".section") {
				if (!regex_search(real_sec, matches, section_regex)) {
					return -1;
				}
				if (real_sec == "UND")return -1;
				string bits;
				string other;
				int sec_num = sym_table->sec_table->find(real_sec);
				if (pass == 1) {
					if (sec_num >= 0) {
						if (instr >> other)return -1;
					}
					else {
						if (!(instr >> bits))return -1;
						if (instr >> other)return -1;
						sec_num = sym_table->sec_table->add_section(real_sec, bits);
					}
					sym_table->curr_section = sec_num;
					sym_table->add_symbol(real_sec);
					sym_table->sec_table->sections[sec_num]->symbol_index = sym_table->find(real_sec)->index;
				}
				else {
					sym_table->curr_section = sec_num;
				}
				return 0;
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}
	}
	
}

int Assembler::process_directive(string dir, string rest, int pass) {
	
	regex hex("^0x[0-9a-f]{1,4}$");
	regex dec("^[-]?[0-9]{1,5}$");
	regex symbol("^[\\.]?[a-z]+[\\w]?$");
	smatch matches;
	int pos;
	while ((pos = rest.find(',')) >= 0) {
		rest.replace(pos, 1, " ");
	}
	rest = toLowerCase(rest);
	string other;

	istringstream instr(rest);

	if (dir == ".align") {
		if (sym_table->curr_section == 0)return -1;
		string r;
		if (!(instr >> r))return -1;
		if (instr >> other)return -1;

		int al;
		
		if (regex_search(r, matches, hex)) {
			al = stoi(r, 0, 16);
			if (al > 8)return -1;  //over max
			while (sym_table->sec_table->sections[sym_table->curr_section]->location_counter % al)
				if(pass==2)write8(0);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter++;
			return 0;
		}
		if (regex_search(r, matches, dec)) {
			al = stoi(r, 0, 10);
			if (al > 8)return -1;  //over max
			while (sym_table->sec_table->sections[sym_table->curr_section]->location_counter % al)
				if(pass==2)write8(0);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter++;
			return 0;
		}
		return -1;
	}


	if (dir == ".skip") {
		if (sym_table->curr_section == 0)return -1;
		string r;
		if (!(instr >> r))return -1;
		if (instr >> other)return -1;

		int sk;

		if (regex_search(r, matches, hex)) {
			sk = stoi(r, 0, 16);
			//if (sk > 16)return -1;  //over max
			for (int i = 0; i < sk; i++) {
				if(pass==2)write8(0);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter++;
			}
			return 0;
		}
		if (regex_search(r, matches, dec)) {
			sk = stoi(r, 0, 10);
			//if (sk > 16)return -1;  //over max
			for (int i = 0; i < sk; i++) {
				if(pass==2)write8(0);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter++;
			}
			return 0;
		}
		return -1;
	}

	if (dir == ".end") {
		if (instr >> other)return -1;
		return 9999;   //end_of_program code number
	}
	
	if (dir == ".extern") {//dozvoliti samo u UND sekciji
		if (sym_table->curr_section != 0)return -1;
		if (pass == 1) {
			string sym;
			int ret;
			while (instr >> sym) {
				if (con_table->find(sym))return -1;
				if (regex_search(sym, matches, symbol)) {
					ret = sym_table->add_extern(sym);
					if (ret != 0)return -1;
				}
				else return -1;
			}
			return 0;
		}
		else return 0;
	}

	if (dir == ".global") {//dozvoliti samo u UND sekciji
		if (sym_table->curr_section != 0)return -1;
		string sym;
		if (pass == 1) {
			int ret;
			while (instr >> sym) {
				if (con_table->find(sym))return -1;
				if (regex_search(sym, matches, symbol)) {
					ret = sym_table->add_extern(sym);
					if (ret != 0)return -1;
				}
				else return -1;
			}
			return 0;
		}
		else {
			Symbol_table::entry* ent;
			while (instr >> sym) {
				ent = sym_table->find(sym);
				if (ent) {
					if (ent->section == sym_table->sec_table->sections[0]->name)return -1;
					else return 0;
				}
				else return -1;
			}
		}
		
	}

	if (dir == ".byte") {
		if (sym_table->curr_section == 0)return -1;
		if (pass == 1) {
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
			return 0;
		}
		else {
			string v;
			int val;
			if (!(instr >> v))return -1;
			if (instr >> other)return -1;
			if (regex_search(v, matches, dec)) {
				val = stoi(v, 0, 10);
				if (val < -128 || val>127)return -1;
				write8(val);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
				return 0;
			}
			if (regex_search(v, matches, hex)) {
				val = stoi(v, 0, 16);
				if (val > 255)return -1;
				if (val > 127)val = val - 256;
				write8(val);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
				return 0;
			}

			return -1;
		}
	}

	if (dir == ".word") {
		if (sym_table->curr_section == 0)return -1;
		if (pass == 1) {
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
			return 0;
		}
		else {
			string v;
			int val;
			if (!(instr >> v))return -1;
			if (instr >> other)return -1;
			if (regex_search(v, matches, dec)) {
				val = stoi(v, 0, 10);
				if (val < -32768 || val>32767)return -1;
				write16(val);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
				return 0;
			}
			if (regex_search(v, matches, hex)) {
				val = stoi(v, 0, 16);
				if (val > 65535)return -1;
				if (val > 32767)val = val - 65536;
				write16(val);
				sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
				return 0;
			}
			if (regex_search(v, matches, symbol)) {
				Constants_table::entry* con = con_table->find(v);
				if (con) {
					write16(con->value);
					sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
					return 0;
				}
				Symbol_table::entry* ent;
				ent = sym_table->find(v);
				if (!ent)return -1;
				insert_reloc_a(ent);
				return 0;
			}

			return -1;
		}
	}

	if (dir == ".equ") {
		if (pass == 1) {
			string sym, exp;
			if (!(instr >> sym))return -1;
			if (!(instr >> exp))return -1;
			if (instr >> other)return -1;
			if (sym_table->find(sym))return -1;
			Constants_table::entry* con_table_entry = con_table->add_entry(sym);
			if (!con_table_entry)return -1;
			else return(process_expression(con_table_entry, exp));
		}
		else return 0;
	}

	return -1;
}

int Assembler::process_instruction(string ins, string rest, int pass) {

	int access=sym_table->sec_table->sections[sym_table->curr_section]->rwx.find('x');
	if (access < 0)return -1;
	
	int pos;
	if ((pos = rest.find(',')) >= 0) {
		rest.replace(pos, 1, " ");
	}

	istringstream instr(rest);
	string op1, op2, other;
	regex not_whitespace("[^[:space:]]");
	regex immed("^&[\\.]?[a-z]+[\\w]?$|^[-]?[0-9]{1,5}$|^0x[a-f0-9]{1,4}$");
	smatch matches;
	int sw;
	char suf='o';
	int op1_length, op2_length;

	if (instructions.find(ins) == instructions.end()) {
		suf = ins[ins.length() - 1];
		if (suf != 'b' && suf != 'w') {
			return -1;
		}
		ins = ins.substr(0, ins.length() - 1);
	}

	if (!(instr >> op1))op1_length = 0;
	else {
		op1 = toLowerCase(op1);
		op1_length = opLength(op1, suf);
	}
	if (!(instr >> op2))op2_length = 0;
	
	else {
		op2 = toLowerCase(op2);
		op2_length = opLength(op2, suf);
	}
	if (op1_length < 0 || op2_length < 0)return -1;
	
	if (instr >> other)return -1;
	
	sw = stoi(opcodes[ins], 0, 2);

	if (pass == 1) {
		switch (sw) {
		case 1:	case 24: case 25:
			if (op1_length)return -1;
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
			return 0;
		case 3:	case 10: case 18: case 19: case 20:	case 21: case 22: case 23:
			if (!op1_length)return -1;
			if (op2_length)return -1;
			if (regex_search(op1, matches, immed)) {
				return -1;
			}
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += (1 + op1_length);
			return 0;
		case 17:
			if (!op1_length)return -1;
			if (op2_length)return -1;
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += (1 + op1_length);
			return 0;
		case 2: case 4:	case 5:	case 6:	case 7:	case 8:	case 9:	case 11: case 12: case 13: case 14:	case 15: case 16:
			if (!op1_length)return -1;
			if (!op2_length)return -1;
			if (regex_search(op1, matches, immed)) {
				return -1;
			}
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += (1 + op1_length + op2_length);
			return 0;
		}
	}

	if (pass == 2) {
		output[sym_table->curr_section].append(opcodes[ins]);
		if (suf == 'b') output[sym_table->curr_section].append("000 ");
		else output[sym_table->curr_section].append("100 ");
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		int ret;

		if (op1_length) {
			ret = writeOp(op1, suf, op2_length);
			if (ret != 0)return -1;
		}
		if (op2_length) {
			ret = writeOp(op2, suf, 0);
			if (ret != 0)return -1;
		}
		return 0;
	}
	
	return -1;
}

int Assembler::writeOp(string op, char suf, int op2length) {
	
	regex immed_dec("^[-]?[0-9]{1,5}$");
	regex immed_hex("^0x[a-f0-9]{1,4}$");
	regex immed_sym("^&[\\.]?[a-z]+[\\w]?$");
	regex regdir("^r[0-7]{1}[hl]?$|^pc[hl]?$|^sp[hl]?$|^psw[hl]?$");
	regex regind_sym("^r[0-7]{1}\\[[\\.]?[a-z]+[\\w]?\\]$|^pc\\[[\\.]?[a-z]+[\\w]?\\]$|^sp\\[[\\.]?[a-z]+[\\w]?\\]$|^psw\\[[\\.]?[a-z]+[\\w]?\\]$");
	regex regind_dec("^r[0-7]{1}\\[[-]?[0-9]{1,5}\\]$|^pc\\[[-]?[0-9]{1,5}\\]$|^sp\\[[-]?[0-9]{1,5}\\]$|^psw\\[[-]?[0-9]{1,5}\\]$");
	regex regind_hex("^r[0-7]{1}\\[0x[0-9a-f]{1,4}\\]$|^pc\\[0x[0-9a-f]{1,4}\\]$|^sp\\[0x[0-9a-f]{1,4}\\]$|^psw\\[0x[0-9a-f]{1,4}\\]$");
	regex pc_rel("^\\$[\\.]?[a-z]+[\\w]?$");
	regex abs("^\\*[0-9]{1,5}$|^\\*0x[a-f0-9]{1,4}$");
	regex abs_dec("^\\*[0-9]{1,5}$");
	regex abs_hex("^\\*0x[a-f0-9]{1,4}$");
	regex abs_sym("^[\\.]?[a-z]+[\\w]?$");
	smatch matches;

	if (regex_search(op, matches, immed_dec)) {		//podrazumevano 16bitna velicina
		output[sym_table->curr_section].append("00000000 ");
		int val = stoi(op, 0, 10);
		if (suf == 'b') {
			write8(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
		}
		else {
			write16(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 3;
		}
		return 0;
	}

	if (regex_search(op, matches, immed_hex)) {
		output[sym_table->curr_section].append("00000000 ");
		int val = stoi(op, 0, 0);
		if (op.length() <= 4) {
			if (val > 127)val = val - 256;
		}
		else {
			if (val > 32767) val = val - 65536;
		}
		if (suf == 'b') {
			write8(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
		}
		else {
			write16(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 3;
		}
		return 0;
	}

	if (regex_search(op, matches, immed_sym)) {    //Simbol uvek ima 16-bitnu vrednost
		output[sym_table->curr_section].append("00000000 ");
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		string sym = op.substr(1);
		Constants_table::entry* con = con_table->find(sym);
		if (con) {
			write16(con->value);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
			return 0;
		}
		Symbol_table::entry* ent;
		ent = sym_table->find(sym);
		if (!ent)return -1;
		insert_reloc_a(ent);
		//cout << "op " << op << endl;
		//cout << "reloc a" << ent->name << endl;
		return 0;
	}

	if (regex_search(op, matches, regdir)) {
		output[sym_table->curr_section].append("001");
		string app = "0";
		if (suf == 'b') {
			if (op[op.length() - 1] == 'h')app = "1";
			op = op.substr(0, op.length() - 1);
		}
		int wreg = writeRegister(op);
		if (wreg != 0)return -1;
		output[sym_table->curr_section].append(app + " ");
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		return 0;
	}
	
	if (regex_search(op, matches, regind_dec)) {
		int pos = op.find('[') + 1;
		int len = op.find(']') - pos;
		string num = op.substr(pos, len);
		int val = stoi(num, 0, 10);
		if (val == 0) output[sym_table->curr_section].append("010");
		else if (val >= -128 && val <= 127) output[sym_table->curr_section].append("011");
		else output[sym_table->curr_section].append("100");
		string reg = op.substr(0, pos-1);
		int wreg = writeRegister(reg);
		if (wreg != 0)return -1;
		output[sym_table->curr_section].append("0 ");
		if (val == 0) {
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		}
		else if (val >= -128 && val <= 127) {
			write8(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
		}
		else {
			write16(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 3;
		}
		return 0;
	}

	if (regex_search(op, matches, regind_hex)) {
		int pos = op.find('[') + 1;
		int len = op.find(']') - pos;
		string num = op.substr(pos, len);
		int val = stoi(num, 0, 16);
		if (len <= 4) {
			if (val > 127)val = val - 256;
		}
		else {
			if (val > 32767) val = val - 65536;
		}
		if (val == 0) output[sym_table->curr_section].append("001");
		else if (val >= -128 && val <= 127) output[sym_table->curr_section].append("011");
		else output[sym_table->curr_section].append("100");
		string reg = op.substr(0, pos-1);
		int wreg = writeRegister(reg);
		if (wreg != 0)return -1;
		output[sym_table->curr_section].append("0 ");
		if (val == 0) {
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		}
		else if (val >= -128 && val <= 127) {
			write8(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
		}
		else {
			write16(val);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 3;
		}
		return 0;
	}

	if (regex_search(op, matches, regind_sym)) {
		int pos = op.find('[') + 1;
		int len = op.find(']') - pos;
		string sym = op.substr(pos, len);
		string reg = op.substr(0, pos - 1);
		output[sym_table->curr_section].append("100");
		int wreg = writeRegister(reg);
		if (wreg != 0)return -1;
		output[sym_table->curr_section].append("0 ");
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;

		Constants_table::entry* con = con_table->find(sym);
		if (con) {
			write16(con->value);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
			return 0;
		}

		Symbol_table::entry* ent;
		ent = sym_table->find(sym);
		if (!ent)return -1;

		if (reg != "pc" && reg != "r7") {
			insert_reloc_a(ent);
			//cout << "op " << op << endl;
			//cout << "reloc a" << ent->name << endl;
		}
		else {
			insert_reloc_r(ent, op2length);
			//cout << "op " << op << endl;
			//cout << "reloc r" << ent->name << endl;
		}
		return 0;
	}

	if (regex_search(op, matches, pc_rel)) {		
		output[sym_table->curr_section].append("10001110 ");
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		string sym = op.substr(1);

		Constants_table::entry* con = con_table->find(sym);
		if (con) {
			write16(con->value);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
			return 0;
		}

		Symbol_table::entry* ent;
		ent = sym_table->find(sym);
		if (!ent)return -1;
		insert_reloc_r(ent, op2length);
		//cout << "op " << op << endl;
		//cout << "reloc r" << ent->name << endl;
		return 0;
	}

	if (regex_search(op, matches, abs_dec)) {
		string num = op.substr(1, op.length() - 1);
		int val = stoi(num, 0, 10);
		if (val > 65535)return -1;
		output[sym_table->curr_section].append("10100000 ");
		write16(val);
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 3;
		return 0;
	}

	if (regex_search(op, matches, abs_hex)) {
		string num = op.substr(1, op.length() - 1);
		int val = stoi(num, 0, 16);
		if (val > 65535)return -1;
		output[sym_table->curr_section].append("10100000 ");
		write16(val);
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 3;
		return 0;
	}

	if (regex_search(op, matches, abs_sym)) {
		output[sym_table->curr_section].append("10100000 ");
		sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 1;
		string sym = op;

		Constants_table::entry* con = con_table->find(sym);
		if (con) {
			write16(con->value);
			sym_table->sec_table->sections[sym_table->curr_section]->location_counter += 2;
			return 0;
		}

		Symbol_table::entry* ent;
		ent = sym_table->find(sym);
		if (!ent)return -1;
		insert_reloc_a(ent);
		//cout << "op " << op << endl;
		//cout << "reloc a" << ent->name << endl;
		return 0;
	}

	return -1;
}
