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

double diff(vector<int> d1, vector<int> d2) {

	if(d1.size() != d2.size()) {
		cerr << RED"Difference between two vectors of unequal sizes"NORMAL << endl;
	}

	double s1 = 0.0f, s2 = 0.0f;
	for (int i = 0; i < d1.size(); i++) {
		s1 += d1[i]*d1[i];
		s2 += d2[i]*d2[i];
	}

	return fabs(s1-s2);
}

vector<int> subtract(vector<int> d1, vector<int> d2) {
	for (int i = 0; i < d1.size(); i++) {
		d1[i] -= d2[i];
	}
	return d1;
}


int diff_checks(vector<int> key, vector<int> order) {
	order = subtract(order, key);
	int sign = 1;
	for (int i = 0; i < order.size(); i++) {
		if(order[i] < 0) sign = 0;
	}
	return sign;
}

double diff_c(config_t c1, config_t c2) {
	return (diff(c1.get_key(), c2.get_key()));
}

int distance(vector<int> d) {
	int s = 0;
	for (int i = 0; i < d.size(); i++) {
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
	o.set_database(&d);

	vector<int> order = d.order;

	print("Order is ", order);

	vector<config_t> packlist;

	// Layers
	while (distance(order) > 0) {

		int selected = 0;
		multimap<key_, config_t>::iterator it;
		config_t c;
		int c_val = 1000000.0f;

		for (it = d.layer_map.begin(); it != d.layer_map.end(); it++) {
			double cv = diff((*it).first, order);
			int cv_checks = diff_checks((*it).first, order);
			if ((cv < c_val) && cv_checks) {
				c_val = cv;
				c = (*it).second;
				selected = 1;
			}
		}

		if(selected == 0) break;
		cout << "Selected  " << c.key_s() << endl;
		packlist.push_back(c);
		order = subtract(order, c.get_key());
	}

	multimap<int, config_t> packlist_temp;
	multimap<int, config_t>::reverse_iterator pt;
	for (uint i = 0; i < packlist.size(); i++) {
		packlist_temp.insert(pair<int, config_t>(packlist[i].get_weight(), packlist[i]));
	}
	for (pt = packlist_temp.rbegin(); pt != packlist_temp.rend() ; pt++) {
		o.insert((*pt).second);
	}
	packlist_temp.clear();

	print("Order left ", order);

	// Configs
	while (distance(order) > 0) {

		int selected = 0;
		multimap<key_, config_t>::iterator it;
		config_t c;
		int c_val = 1000000.0f;

		for (it = d.config_map.begin(); it != d.config_map.end(); it++) {
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
		o.insert(c);
		order = subtract(order, c.get_key());
	}

	print("Order left ", order);

	o.save_packlist();
	o.exportpl();
	o.savepl_xml();


	// Done our hack!
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

//	d.pose_lp("prob");

	return 0;
}
