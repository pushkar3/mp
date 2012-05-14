#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <math.h>
#include "3dbpp.h"

using namespace std;

struct configcomp {
	static bool compare(config_t c1, config_t c2) {
		float diff = fabs(c1.density() - c2.density())/max(c1.density(), c2.density());
		return (diff < 0.05 && c1.is_bound() && c2.is_bound());
	}
};

void help() {
	cout << "./stat Output stats for the database folder" << endl;
	cout << "Usage: ./stat <foldername>" << endl;
}

int main(int argc, char *argv[]) {
	int verbose = 0;

	if (argc < 2) {
		help();
		return 0;
	}

	if (argc > 2) {
		if(strcmp(argv[1], "-v") == 0) verbose = 1;
		if(strcmp(argv[1], "-vv") == 0) verbose = 2;
		if(strcmp(argv[1], "-vvv") == 0) verbose = 3;
	}

	string dir(argv[argc-1]);
	input i;
	output o;
	database d;

	i.load(dir.c_str());
	d.get_input(i);
	d.importdb();

	//o.set_database(&d);
	//o.importpl();

	d.printdb_stat();
	if(verbose > 0) i.print();
	if(verbose > 1) d.printdb();

	return 0;
}
