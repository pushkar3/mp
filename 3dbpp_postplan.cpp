#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <math.h>
#include "3dbpp.h"

using namespace std;

double diff(vector<int> d1, vector<int> d2) {

	if(d1.size() != d2.size()) {
		cerr << RED"Difference between two vectors of unequal sizes"NORMAL << endl;
	}

	double s1 = 0.0f, s2 = 0.0f;
	for (uint i = 0; i < d1.size(); i++) {
		s1 += d1[i]*d1[i];
		s2 += d2[i]*d2[i];
	}

	return fabs(s1-s2);
}

vector<int> subtract(vector<int> d1, vector<int> d2) {
	vector<int> d(d1.size(), 0);
	for (uint i = 0; i < d1.size(); i++) {
		d[i] = d1[i] - d2[i];
	}
	return d;
}

vector<int> add(vector<int> d1, vector<int> d2) {
	vector<int> d(d1.size(), 0);
	for (uint i = 0; i < d1.size(); i++) {
		d[i] = d1[i] + d2[i];
	}
	return d;
}


int diff_checks(vector<int> key, vector<int> order) {
	order = subtract(order, key);
	int sign = 1;
	for (uint i = 0; i < order.size(); i++) {
		if(order[i] < 0) sign = 0;
	}
	return sign;
}

double diff_c(config_t c1, config_t c2) {
	return (diff(c1.get_key(), c2.get_key()));
}

int distance(vector<int> d) {
	int s = 0;
	for (uint i = 0; i < d.size(); i++) {
		s += d[i]*d[i];
	}
	return s;
}

void print(const char* str, vector<int> d) {
	cout << str << " ";
	for (uint i = 0; i < d.size(); i++) {
		cout << d[i] << " ";
	}
	cout << endl;
}

packlist_ greedy_select(database* d) {
	vector<int> order = d->order;

	packlist_ packlist;
	packlist_ pl;

	// Layers
	config_t prev_c;
	int prev_rot = 0;

	while (distance(order) > 0) {

		int selected = 0;
		multimap<key_, config_t>::iterator it;
		config_t c;
		int c_val = 1000000.0f;

		for (it = d->layer_map.begin(); it != d->layer_map.end(); it++) {
			double cv = diff((*it).first, order);
			int cv_checks = diff_checks((*it).first, order);
			if ((cv < c_val) && cv_checks) {
				c_val = cv;
				c = (*it).second;
				selected = 1;
			}
		}

		if(selected == 0) break;
		cout << "Selected  " << c.key_s();
		packlist.push_back(c);
		prev_c = c;
		order = subtract(order, c.get_key());
		cout << endl;
	}

	multimap<int, config_t> packlist_temp;
	multimap<int, config_t>::reverse_iterator pt;
	for (uint i = 0; i < packlist.size(); i++) {
		packlist_temp.insert(pair<int, config_t>(packlist[i].get_weight(), packlist[i]));
	}
	for (pt = packlist_temp.rbegin(); pt != packlist_temp.rend() ; pt++) {
		pl.push_back((*pt).second);
	}
	packlist_temp.clear();

	print("Order left ", order);

	// Configs
	vector<config_t> c_vec;
	while (distance(order) > 0) {

		int selected = 0;
		multimap<key_, config_t>::iterator it;
		config_t c;
		int c_val = 1000000.0f;

		for (it = d->config_map.begin(); it != d->config_map.end(); it++) {
			double cv = diff((*it).first, order);
			int cv_checks = diff_checks((*it).first, order);
			if ((cv < c_val) && cv_checks) {
				c_val = cv;
				c = (*it).second;
				selected = 1;
			}
		}

		if(selected == 0) break;
		cout << "Selected  " << c.key_s() << " Dims: " << c.dimensions_s() << endl;
		c_vec.push_back(c);

		order = subtract(order, c.get_key());
	}

	for (uint i = 0; i < c_vec.size(); i++) {
		packlist_temp.insert(pair<int, config_t>(c_vec[i].get_weight(), c_vec[i]));
	}
	for (pt = packlist_temp.rbegin(); pt != packlist_temp.rend() ; pt++) {
		pl.push_back((*pt).second);
	}
	packlist_temp.clear();

	print("Order left ", order);
	return pl;
}

void postplan(packlist_ pl, database *d, output* o) {
	config_t prev_c;
	int prev_rot = 0;

	// Put it in bins
	int h = 0;
	for (uint i = 0; i < pl.size(); i++) {
			h += pl[i].get_height();
	}

	int n_bins_req = round((double)h/(double)d->bin.d);
	o->gen_bins(n_bins_req);

	h = 0;
	int n_bin = 0;
	for (uint i = 0; i < pl.size(); i++) {
		int bn = n_bin % n_bins_req;
		int bn_new = bn;
		if ((o->get_height(bn)+pl[i].get_height()) > d->bin.d) {
			bn_new = n_bins_req;
		}

		if(((double) pl[i].get_area()/(double) d->bin.get_area()) > 0.40) {
			pl[i].spread_out(d->bin);
			pl[i].center_in(d->bin);
		}

		config_t prev;
		if (o->get_n_configs(bn_new) > 0)
			prev = o->get_config_on_top(bn_new);

		if (pl[i].compare(prev) == 0) {
			if (prev_rot == 0) {
				pl[i].rotate();
				prev_rot = 1;
			}
			else
				prev_rot = 0;
		}

		cout << "Inserting in bin " << bn_new << endl;
		o->insert(bn_new, pl[i]);
		n_bin++;
	}

	o->save_packlist();
	o->exportpl();
	cout << "Saving XML" << endl;
	o->savepl_xml();
}
