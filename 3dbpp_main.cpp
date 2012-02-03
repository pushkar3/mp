#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include "3dbpp.h"
#include "palletizing.h"

using namespace std;

struct configcomp {
	static bool compare(config_t c1, config_t c2) {
		float diff = fabs(c1.density() - c2.density())/max(c1.density(), c2.density());
		return (diff < 0.05 && c1.is_bound() && c2.is_bound());
	}
};

void binpack2(database *d) {
	vector<int> combo(d->package.size(), 0);
	for (uint i = 0; i < d->package.size(); i++) {
		combo[i] = 1;
		std::vector<int> key(combo);
		std::vector<int> pattern(3, 0);
		d->insert(key, pattern);
		combo[i] = 0;
	}

	std::map<key_, config_t>::iterator it1;
	std::map<key_, config_t>::iterator it2;

	long int n = 0;
	for (it1 = d->config_map.begin(); it1 != d->config_map.end(); it1++) {
		for (it2 = d->config_map.begin(); it2 != d->config_map.end(); it2++) {
			for (uint i = 0; i < 3; i++) {
				config_t c1 = (*it1).second;
				config_t c2 = (*it2).second;
				c2.set_origin(c1.get_corner(i));
				c1.add(c2);

				if (configcomp::compare(c1, c2)) {
					d->insert(c1.get_key(), c1.get_pattern());
				}
			}
			if (++n % 10 == 0) {
				printf("Database size: %d\r", d->config_map.size());
				d->exportdb();
			}
		}
	}
}

int main(int argc, char *argv[]) {

	input i;
	output o;
	database d;
	i.load("data");
	i.print();

	d.get_input(i);
	std::cout << "Starting binpacking" << std::endl;

	if (!d.importdb()) {
		binpack2(&d);
		d.exportdb();
	}

	//d.printdb();

	palletizing p(&d);
	p.solve();

	//o.exportl(d, "data");

	return 0;
}
