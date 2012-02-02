#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include "3dbpp.h"
#include "palletizing.h"

using namespace std;

class layer_t {
	std::vector<int> layer_key;
	std::vector<int> layer;
	std::vector<package> pack;
	database* db;

	int n_packs;
	int n_area;
	int n_maxw;
	int n_maxd;
	int n_maxh;
	float n_density;
	int n_tvolume;
	int n_packvolume;

public:
	layer_t() {
	}
	layer_t(database* d) {
		set_database(d);
	}
	~layer_t() {
	}

	void set_database(database* d) {
		db = d;
		pack = d->package_info;
	}

	void set_layer(std::vector<int> _key, std::vector<int> _pattern) {
		layer_key.clear();
		layer.clear();
		layer_key = _key;
		layer = _pattern;
		n_packs = 0;
		n_area = 0;
		n_maxh = 0;
		n_maxw = 0;
		n_maxd = 0;
		n_tvolume = 0;
		n_density = 0.0f;
		n_packvolume = 0;
	}

	std::vector<int> get_key() {
		return layer_key;
	}

	std::vector<int> get_pattern() {
		return layer;
	}

	std::vector<int> get_dimensions() {
		std::vector<int> ret(3, 0);
		ret[0] = n_maxw;
		ret[1] = n_maxh;
		ret[2] = n_maxd;
		return ret;
	}

	float density() {
		return n_density;
	}

	void evaluate() {
		int c = 0;
		for (uint i = 0; i < layer_key.size(); i++) {
			n_packs += layer_key[i];
			n_packvolume += (layer_key[i] * pack[i].volume());

			for (uint j = 0; j < layer_key[i]; j++) {
				int w = pack[i].w;
				int h = pack[i].h;
				int d = pack[i].d;
				int x = layer[c * 3 + 0];
				int y = layer[c * 3 + 1];
				int z = layer[c * 3 + 2];
				if (n_maxw < x + w)
					n_maxw = x + w;
				if (n_maxh < y + h)
					n_maxh = y + h;
				if (n_maxd < z + d)
					n_maxd = z + d;
				c++;
			}
		}

		n_tvolume = n_maxw * n_maxh * n_maxd;
		if (n_tvolume != 0)
			n_density = (float) ((n_packvolume) / (n_tvolume));
		n_area = n_maxw * n_maxh;

	}

	int corner(int i) {
		switch (i) {
		case 0:
			return n_maxw;
			break;
		case 1:
			return n_maxh;
			break;
		case 2:
			return n_maxd;
			break;
		}

	}

	void print() {
		std::cout << "Layer " << std::endl;
		for (uint i = 0; i < layer_key.size(); i++)
			std::cout << layer_key[i] << " ";
		std::cout << std::endl;
		for (uint i = 0; i < layer.size(); i++)
			std::cout << layer[i] << " ";
		std::cout << std::endl;
		std::cout << "Total packages: " << n_packs << std::endl;
		std::cout << "Total packvolume: " << n_packvolume << std::endl;
		std::cout << "Total volume: " << n_tvolume << std::endl;
		std::cout << "Total area: " << n_area << std::endl;
		std::cout << "Max height: " << n_maxd << std::endl;
		printf("Density: %.2f\n", n_density);
		std::cout << std::endl;
	}
};

class layer_comp_t {
	database *d;
	layer_t l1, l2;
public:
	layer_comp_t(database* _d) {
		d = _d;
		l1.set_database(d);
		l2.set_database(d);
	}

	~layer_comp_t() {
	}

	void set1(std::vector<int> _key, std::vector<int> _pattern) {
		l1.set_layer(_key, _pattern);
		l1.evaluate();
	}

	void set2(std::vector<int> key2, std::vector<int> pattern2, int corner_pos) {
		if (corner_pos >= 3) {
			std::cerr << "Error at corner_pos" << std::endl;
			exit(1);
		}

		int w = 0 , h = 0, d = 0;
		if(corner_pos == 0) w = l1.corner(0);
		if(corner_pos == 1) h = l1.corner(1);
		if(corner_pos == 2) d = l1.corner(2);

		std::vector<int> key1 = l1.get_key();
		std::vector<int> pattern1 = l1.get_pattern();
		std::vector<int> key(key1.size(), 0);
		std::vector<int> pattern;

		int c1 = 0, c2 = 0;
		for (uint i = 0; i < key1.size(); i++) {
			for (uint j = 0; j < key1[i]; j++) {
				pattern.push_back(pattern1[c1 * 3 + 0]);
				pattern.push_back(pattern1[c1 * 3 + 1]);
				pattern.push_back(pattern1[c1 * 3 + 2]);
				c1++;
			}
			for (uint j = 0; j < key2[i]; j++) {
				pattern.push_back(pattern2[c2 * 3 + 0] + w);
				pattern.push_back(pattern2[c2 * 3 + 1] + h);
				pattern.push_back(pattern2[c2 * 3 + 2] + d);
				c2++;
			}
			key[i] = key1[i] + key2[i];
		}

		l2.set_layer(key, pattern);
		l2.evaluate();
	}

	std::vector<int> get_key2() {
		return l2.get_key();
	}

	std::vector<int> get_pattern2() {
		return l2.get_pattern();
	}

	std::vector<int> get_dimensions2() {
		return l2.get_dimensions();
	}

	double get_cost2() {
		return (double) l2.density();
	}

	int compare() {

		float diff = fabs(l1.density() - l2.density()) / max(l1.density(), l2.density());
		for (uint i = 0; i < d->bin_info.size(); i++) {
			if (l2.corner(0) <= d->bin_info[i].w
					&& l2.corner(1)	<= d->bin_info[i].h
					&& l2.corner(2) <= d->bin_info[i].d
					&& diff < 0.05) {
				return 1;
			}
		}

		return 0;
	}
};

void binpack2(database *d) {
	// Build the smallest blocks
	vector<int> combo(d->package_info.size(), 0);
	vector<int> combo_max(d->package_info.size(), 5);
	for (uint i = 0; i < d->package_info.size(); i++) {
		combo[i] = 1;
		std::vector<int> pattern(3, 0);
		std::vector<int> key(combo);
		std::vector<int> dimensions(3, 0);
		dimensions[0] = d->package_info[i].w;
		dimensions[1] = d->package_info[i].h;
		dimensions[2] = d->package_info[i].d;
		double cost = 1.0f;
		d->insert(key, pattern, cost, dimensions);
		combo[i] = 0;
	}

	std::map<key, pattern>::iterator itp;
	std::map<key, pattern>::iterator itd;
	std::map<key, cost>::iterator itc;
	std::map<key, pattern>::iterator itp2;
	std::map<key, pattern>::iterator itd2;
	std::map<key, cost>::iterator itc2;

	layer_comp_t layer_comp(d);

	long int c = 0;
	for (itp = d->layer_pattern.begin(), itc = d->layer_cost.begin(), itd
			= d->layer_dimensions.begin(); itp != d->layer_pattern.end(); itp++, itc++, itd++) {

		itp2 = d->layer_pattern.begin();
		itc2 = d->layer_cost.begin();
		itd2 = d->layer_dimensions.begin();
		layer_comp.set1((*itp).first, (*itp).second);

		for (; itp2 != d->layer_pattern.end(); itp2++, itc2++, itd2++) {
			for (uint i = 0; i < 3; i++) {
				layer_comp.set2((*itp2).first, (*itp2).second, i);
				if (layer_comp.compare() == 1) {
					d->insert(layer_comp.get_key2(), layer_comp.get_pattern2(),
							layer_comp.get_cost2(),
							layer_comp.get_dimensions2());
				}
			}
			if (++c % 10 == 0) {
				printf("Database size: %d\r", d->layer_pattern.size());
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

	palletizing p(&d);
	p.solve();

	o.exportl(d, "data");

	return 0;
}
