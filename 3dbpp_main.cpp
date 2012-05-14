#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <math.h>
#include "3dbpp.h"
#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;

struct configcomp {
	static bool compare(config_t c1, config_t c2) {
		float diff = fabs(c1.density() - c2.density())/max(c1.density(), c2.density());
		return (diff < 0.05 && c1.is_bound() && c2.is_bound());
	}
};

void binpack_singlepackageconfig(database* d, int package_num) {
	int h_max = d->package[package_num].d * 1.05;
	int _key = package_num;

	database db(*d);

	vector<int> key(db.package.size(), 0);
    vector<int> pattern(3, 0);
    key[_key] = 1;
    db.insert(key, pattern);

    multimap<key_, config_t, classcomp> cmap;
    multimap<key_, config_t>::iterator it1;
    multimap<key_, config_t>::iterator it2;
    multimap<key_, config_t>::iterator it_temp;

    long int n = 0;

	for (it1 = db.config_map.begin(); it1 != db.config_map.end(); it1++) {
		for (it2 = db.config_map.begin(); it2 != db.config_map.end(); it2++) {
			for (uint i = 0; i < 3; i++) {
				config_t c1 = (*it1).second;
				config_t c2 = (*it2).second;
				c2.set_origin(c1.get_corner(i));
				c1.add(c2);

				if (configcomp::compare(c1, c2) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
					db.insert(c1.get_key(), c1.get_pattern());
				}
			}
			if (++n % 100 == 0) {
				//d->exportdb();
				printf("Database size: c %d, l %d\r", db.config_map.size(), db.layer_map.size());
			}
		}
	}

	d->insert(db.config_map, db.layer_map);
	printf("Database size: c %d, l %d\r", d->config_map.size(), d->layer_map.size());
}

void binpack2(database *d) {
	for (uint i = 0; i < d->package.size(); i++) {
		binpack_singlepackageconfig(d, i);
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

	return 0;
}
