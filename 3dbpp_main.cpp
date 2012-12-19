#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tinyxml2.h>
#include "3dbpp.h"
#include <dirent.h>

#include <regex>

using namespace std;
using namespace tinyxml2;

void binpack2(database *d) {
	vector<binpack_job*> job;

	cout << BOLD"Running for single packages"NORMAL << endl;
	for (uint i = 0; i < d->package.size(); i++) {
		for (uint j = 0; j < d->bin_d.size(); j++) {
			if (d->package[i].d <= d->bin_d[j].d) {
				binpack_job* bj = new binpack_job();
				bj->set_singlepackage(d, i, d->bin_d[j].d);
				job.push_back(bj);
			}
		}
	}

	for (uint i = 0; i < job.size(); i++) {
		cout << "Job " << i << " Height=" << job[i]->get_height() << " Package Key="<< job[i]->get_packagekey() <<endl;
		job[i]->run();
		job[i]->d.printdb_stat();
		d->insert(job[i]->d.config_map);
		d->exportdb();
	}

	cout << BOLD"Running for multiple packages"NORMAL << endl;

	uint n = job.size();
	for (uint i = 0; i < n; i++) {
		for (uint j = 0; j < n; j++) {

			if(job[i]->is_singlepack() && job[j]->is_singlepack() &&
					job[i]->get_packagekey() != job[j]->get_packagekey() &&
					job[i]->get_height() == job[j]->get_height() ) {

				binpack_job* bj = new binpack_job();
				bj->set_multiplepackage(job[i]->d, job[j]->d, job[i]->get_height());
				job.push_back(bj);
			}
		}
	}

	for (uint i = n; i < job.size(); i++) {
		cout << "Job " << i << " Height=" << job[i]->get_height() << " Package Key="<< job[i]->get_packagekey() <<endl;
		job[i]->run();
		job[i]->d.printdb_stat();
		d->insert(job[i]->d.config_map);
		d->exportdb();
	}
}

void help() {
	cout << "./plan Plan for the order file in folder" << endl;
	cout << "Usage: ./plan <foldername>" << endl;
}

int main(int argc, char *argv[]) {

//	regex e("(c)(.*)");
//
//	DIR* dirs = NULL;
//	struct dirent *drnt = NULL;
//
//	dirs = opendir("./data");
//
//	if (dirs) {
//		while (drnt = readdir(dirs)) {
//			cout << drnt->d_name;
//			if(regex_match(drnt->d_name, e)) {
//				cout << "\tfile matched";
//			}
//			cout << endl;
//		}
//		closedir(dirs);
//	}
//
//	return 1;

	if (argc < 2) {
		help();
		return 0;
	}

	string dir(argv[argc-1]);

	input i;
	output o;
	database d;
	//i.load(dir.c_str());
	i.load_xml(dir.c_str());
	i.print();

	d.get_input(i);
	cout << BOLD"Starting binpacking"NORMAL << endl;

	if (!d.importdb()) {
		d.initdb();
		binpack2(&d);
		d.exportdb();
	}

	cout << endl << BOLD"Done."NORMAL << endl;
	d.printdb_stat();
	return 0;
}
