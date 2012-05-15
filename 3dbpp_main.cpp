#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <tinyxml2.h>
#include "3dbpp.h"

#define BOLD "\033[1m"
#define NORMAL "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"

using namespace std;
using namespace tinyxml2;

struct configcomp {
	static bool compare(config_t c1, config_t c2, double tolerance) {
		double diff = fabs(c1.density() - c2.density())/max(c1.density(), c2.density());
		return (diff < tolerance && c1.is_bound() && c2.is_bound());
	}
};

class binpack_job {
	double tolerance;
	int is_singlepackage;
	int package_key;
	double h_max;
	int completed;
public:
	database d;
	database d1, d2;
	binpack_job() {
		package_key = -1;
		is_singlepackage = 0;
		h_max = 0.0f;
		completed = 0;
	}

	~binpack_job() { }

	int is_singlepack() {
		return is_singlepackage;
	}

	double get_height() {
		return h_max;
	}

	int get_packagekey() {
		return package_key;
	}

	void set_singlepackage(database* db, int _key, int _hmax) {
		tolerance = 0.005f;
		is_singlepackage = 1;
		package_key = _key;
		h_max = _hmax;
		d1 = db->clone();
		d1.config_map.clear();
		d1.layer_map.clear();

		vector<int> key(d1.package.size(), 0);
		vector<int> pattern(3, 0);
		key[package_key] = 1;
		d1.insert(key, pattern);
	}

	void set_multiplepackage(database db1, database db2, double _hmax) {
		tolerance = 0.05f;
		h_max = _hmax;
		d1 = db1.clone();
		d2 = db2.clone();
	}

	void singlepackageconfig() {
		multimap<key_, config_t>::iterator it1;
		multimap<key_, config_t>::iterator it2;

		for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
			for (it2 = d1.config_map.begin(); it2 != d1.config_map.end(); it2++) {
				for (uint i = 0; i < 3; i++) {
					config_t c1 = (*it1).second;
					config_t c2 = (*it2).second;
					c2.set_origin(c1.get_corner(i));
					c1.add(&d1, c2);

					if (configcomp::compare(c1, c2, tolerance) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern());
					}
				}
			}
		}
		d = d1.clone();
	}

	void multiplepackageconfig() {
		multimap<key_, config_t>::iterator it1;
		multimap<key_, config_t>::iterator it2;

		long int n = 0;
		for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
			for (it2 = d2.config_map.begin(); it2 != d2.config_map.end(); it2++) {
				n++;
				for (uint i = 0; i < 3; i++) {
					config_t c1 = (*it1).second;
					config_t c2 = (*it2).second;
					c2.set_origin(c1.get_corner(i));
					c1.add(&d1, c2);

					if (configcomp::compare(c1, c2, tolerance) && c1.get_height() <= h_max && c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern());
					}
				}
			}

			if(n > 10000) {
				cout << RED"Reached computation limit!"NORMAL << endl;
				break;
			}
		}

		for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
			for (it2 = d1.config_map.begin(); it2 != d1.config_map.end(); it2++) {
				n++;
				for (uint i = 0; i < 3; i++) {
					config_t c1 = (*it1).second;
					config_t c2 = (*it2).second;
					c2.set_origin(c1.get_corner(i));
					c1.add(&d1, c2);

					if (configcomp::compare(c1, c2, tolerance) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern());
					}
				}
			}

			if(n > 10000) {
				cout << RED"Reached computation limit!"NORMAL << endl;
				break;
			}
		}

		d = d1.clone();
	}

	void run() {
		if (completed) return;
		if(is_singlepackage)
			singlepackageconfig();
		else {
			multiplepackageconfig();
		}
		completed = 1;
	}
};

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
	}

	cout << BOLD"Running for multiple packages"NORMAL << endl;

	int n = job.size();
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
	}

	for (uint i = 0; i < job.size(); i++) {
		if(job[i]->is_singlepack() == 0)
			d->insert(job[i]->d.config_map);
	}
}

void help() {
	cout << "./plan Plan for the order file in folder" << endl;
	cout << "Usage: ./plan <foldername>" << endl;
}

int main(int argc, char *argv[]) {
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
