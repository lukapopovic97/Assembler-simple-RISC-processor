#include "Assembler.h"

int main(int argc, char** argv) {
	Assembler* as = new Assembler(argv[1]);
	as->assemble();
	delete as;
	return 0;
}

