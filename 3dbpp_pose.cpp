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

	i.load(dir.c_str());
	d.get_input(i);
	d.importdb();

//	multimap<key_, config_t>::iterator it;
//
//	for (it = d.config_map.begin(); it != d.config_map.end(); it++) {
//		if((*it).second.is_layer()) {
//			(*it).second.spread_out(d.bin);
//		}
//	}
//
//	// This is important
//	d.exportdb();

	// This all makes the db_layer database invalid, since I am not updating it here
	// todo: layer spacing

	d.pose_lp("prob");

	return 0;
}
