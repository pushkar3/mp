#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <math.h>
#include "3dbpp.h"

using namespace std;

void help() {
	cout << "./pose Convert the database into IP problems" << endl;
	cout << "Usage: ./pose <foldername>" << endl;
}

int main(int argc, char *argv[]) {
	int verbose = 0;

	if (argc < 2) {
		help();
		return 0;
	}

	string dir(argv[argc-1]);
	input i;
	output o;
	database d;

	i.load_xml(dir.c_str());
	d.get_input(i);
	d.importdb();
	o.set_database(&d);

	d.pose_lpc("prob");

	return 0;
}
