#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <tinyxml2.h>
#include "3dbpp.h"

using namespace std;
using namespace tinyxml2;

struct configcomp {
	static bool compare(config_t c1, config_t c2) {
		float diff = fabs(c1.density() - c2.density())/max(c1.density(), c2.density());
		return (diff < 0.05 && c1.is_bound() && c2.is_bound());
	}
};

class binpack_job {
	int is_singlepackage;
	int package_key;
	float h_max;
	int completed;
public:
	database d;
	database d1, d2;
	binpack_job() {
		is_singlepackage = 0;
		h_max = 0.0f;
		completed = 0;
	}

	~binpack_job() { }

	void set_singlepackage(database* db, int _key, int _hmax) {
		is_singlepackage = 1;
		package_key = _key;
		h_max = _hmax;
		d1 = db->clone();

		vector<int> key(d1.package.size(), 0);
		vector<int> pattern(3, 0);
		key[package_key] = 1;
		d1.insert(key, pattern);
	}

	void set_multiplepackage(database db1, database db2, float _hmax) {
//		h_max = _hmax;
//		d1 = db1.clone();
//		d2 = db2.clone();
//		d1.config_map = db1.config_map;
//		cout << "Set multiplepackage" << endl;
		cout << "d1 " << d1.config_map.size() << " db1 " << db1.config_map.size()  << endl;
	}

	void singlepackageconfig() {
		multimap<key_, config_t>::iterator it1;
		multimap<key_, config_t>::iterator it2;

		long int n = 0;
		for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
			for (it2 = d1.config_map.begin(); it2 != d1.config_map.end(); it2++) {
				for (uint i = 0; i < 3; i++) {
					config_t c1 = (*it1).second;
					config_t c2 = (*it2).second;
					c2.set_origin(c1.get_corner(i));
					c1.add(c2);

					if (configcomp::compare(c1, c2) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern());
					}
				}
			}
		}
		d = d1.clone();
		d.config_map = d1.config_map;
		d.layer_map = d1.layer_map;
	}

	void multiplepackageconfig() {
		cout << "Running Multiple package version" << endl;
		cout << "Size " << d1.config_map.size() << " " << d2.config_map.size() << endl;
		multimap<key_, config_t>::iterator it1;
		multimap<key_, config_t>::iterator it2;

		long int n = 0;
		for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
			for (it2 = d2.config_map.begin(); it2 != d2.config_map.end(); it2++) {
				for (uint i = 0; i < 3; i++) {
					config_t c1 = (*it1).second;
					config_t c2 = (*it2).second;
					c2.set_origin(c1.get_corner(i));
					c1.add(c2);

					if (configcomp::compare(c1, c2) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern());
					}
				}
			}
		}

		singlepackageconfig();

	}

	void run() {
		if (completed) return;
		if(is_singlepackage)
			return singlepackageconfig();
		else
			return multiplepackageconfig();
		completed = 1;
	}

};

void binpack2(database *d) {
	vector<binpack_job*> job;
	for (uint i = 0; i < d->package.size(); i++) {
		binpack_job* bj = new binpack_job();
		bj->set_singlepackage(d, i, d->package[i].d*1.05);
		job.push_back(bj);
		job[i]->run();
	}

	for (uint i = 0; i < d->package.size(); i++) {
		for (uint j = 0; j < d->package.size(); j++) {
			if (i == j) continue;
			int h1 = d->package[i].d;
			int h2 = d->package[j].d;
			int l = lcm(h1, h2);
			if (l == h1 || l == h2) {
				binpack_job* bj = new binpack_job();
				bj->set_multiplepackage(job[i]->d, job[j]->d, l);
				cout << endl;
				cout << i << " " << j << " " << l << " [" <<  job[j]->d.config_map.size() << "]"  <<endl;
				job.push_back(bj);
			}
		}
	}

	for (uint i = 0; i < job.size(); i++) {
//		cout << "Running Job " << i << endl;
//		job[i]->run();
//		job[i]->d.printdb_stat();
	}

	for (uint i = 0; i < job.size(); i++) {
		d->insert(job[i]->d.config_map, job[i]->d.layer_map);
	}
}

int main(int argc, char *argv[]) {
	input i;
	output o;
	database d;
	//i.load("data");
	i.load_xml("ex");
	i.print();

	d.get_input(i);
	cout << "Starting binpacking" << endl;

	if (!d.importdb()) {
		d.initdb();
		binpack2(&d);
		d.exportdb();
	}

	cout << "Done." << endl;
	d.printdb_stat();
	return 0;
}
