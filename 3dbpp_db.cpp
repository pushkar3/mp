#include <iostream>
#include <sstream>
#include <tinyxml2.h>
#include "3dbpp.h"

database::database() {
}

database::~database() {
}

void database::set_dir(const char* dirname) {
	dir.assign(dirname);
}

const char* database::get_dir() {
	return dir.c_str();
}

void database::get_input(input i) {
	package.clear();
	bin_d.clear();
	order.clear();
	package = i.package;
	order = i.order;
	set_dir(i.dir.c_str());
	db_c = dir + "/db_config.txt";
	db_l = dir + "/db_layer.txt";

	// bin juggad
	int bin_add = 0;
	bin = i.bin;
	for (uint i = 0; i < package.size(); i++) {
		for (uint j = i; j < package.size(); j++) {
			bin_t b = bin;
			b.d = lcm(package[i].d, package[j].d);
			bin_add = 1;
			for (uint k = 0; k < bin_d.size(); k++) {
				if (bin_d[k].d == b.d) {
					bin_add = 0;
					break;
				}
			}

			if (b.d > bin.d)
				bin_add = 0;

			if (bin_add) {
				bin_d.push_back(b);
				bin_add = 0;
			}
		}
	}

	cout << "New bins" << endl;
	for (uint i = 0; i < bin_d.size(); i++) {
		cout << bin_d[i].id << " " << bin_d[i].w << " " << bin_d[i].h
				<< " " << bin_d[i].d << " " << bin_d[i].n << endl;
	}
	cout << endl;
}

int database::insert(config_t c) {
	return insert(c.get_key(), c.get_pattern());
}

int database::insert(key_ key, pattern_ pattern) {
	config_t config;
	config.set(this, key, pattern);

	multimap<key_, config_t>::iterator it;
	pair < multimap<key_, config_t>::iterator, multimap<key_, config_t>::iterator> ret;
	ret = config_map.equal_range(key);
	for (it = ret.first; it != ret.second; it++) {
		if (config == (*it).second) {
			return 0;
		}
	}

	config_map.insert(pair<key_, config_t> (key, config));
	config_last = config;

	if (config_last.is_layer()) {
		layer_map.insert(
				pair<key_, config_t> (config_last.get_key(), config_last));
	}

	return 1;
}

config_t database::get_last_inserted_config() {
	return config_last;
}

config_t database::get_layer_from_name(string str) {
	str = str.substr(1);
	int n = atoi(str.c_str());
	multimap<key_, config_t>::iterator it;
	it = layer_map.begin();
	while (n > 0) {
		it++;
		n--;
	}
	return (*it).second;
}

void database::find_layers() {
	int area = bin.w * bin.h;
	multimap<key_, config_t>::iterator it;
	for (it = config_map.begin(); it != config_map.end(); it++) {
		config_t c = (*it).second;
		if (((float) (area - c.get_area()) / (float) area) < 0.05)
			layer_map.insert(pair<key_, config_t> (c.get_key(), c));
	}
}

void database::exportdb() {
	ofstream ofs(db_c.c_str());
	multimap<key_, config_t>::iterator it;

	for (it = config_map.begin(); it != config_map.end(); it++) {
		config_t config = (*it).second;
		ofs << "k " << config.key_s() << "\n";
		ofs << "c " << config.cost_s() << "\n";
		ofs << "p " << config.pattern_s() << "\n";
		ofs << "d " << config.dimensions_s() << "\n";
	}
	ofs.close();

	ofs.open(db_l.c_str());
	for (it = layer_map.begin(); it != layer_map.end(); it++) {
		config_t config = (*it).second;
		ofs << "k " << config.key_s() << "\n";
		ofs << "c " << config.cost_s() << "\n";
		ofs << "p " << config.pattern_s() << "\n";
		ofs << "d " << config.dimensions_s() << "\n";
	}
	ofs.close();
}

vector<int> database::deserialize_vector(string str) {
	vector<int> vec;
	char* p;
	str = str.substr(2); // remove first char and space
	char* c = (char*) str.c_str();
	p = strtok(c, " ");
	while (p != NULL) {
		vec.push_back(atoi(p));
		p = strtok(NULL, " ");
	}

	return vec;
}

float database::deserialize_cost(string str) {
	str = str.substr(2);
	int i = atoi(str.c_str());
	return (float) i;
}

int database::importdb() {
	struct stat buf;
	if (stat(db_c.c_str(), &buf) != 0)
		return 0;
	cerr << "Found config database at " << db_c << endl;

	ifstream ifs(db_c.c_str());
	string str;
	key_ key;
	pattern_ pattern;
	dimensions_ dimensions;
	float cost;
	int is_key = 0;
	int is_cost = 0;
	int is_pattern = 0;
	int is_dims = 0;

	while (ifs.good()) {
		char c = ifs.get();
		if (c != '\n') {
			str.push_back(c);
		} else {
			switch (str.at(0)) {
			case 'k':
				key = deserialize_vector(str);
				is_key = 1;
				break;
			case 'c':
				cost = deserialize_cost(str);
				is_cost = 1;
				break;
			case 'p':
				pattern = deserialize_vector(str);
				is_pattern = 1;
				break;
			case 'd':
				dimensions = deserialize_vector(str);
				is_dims = 1;
				break;
			default:
				printf("error");
			}
			str.clear();

			if (is_key && is_cost && is_pattern && is_dims) {
				if (insert(key, pattern)) {
					config_t c = get_last_inserted_config();
					if (c.get_dimensions() != dimensions) {
						cerr << "Error while importing database at ";
						for (uint i = 0; i < c.get_key().size(); i++) {
							cerr << c.get_key()[i] << " ";
						}
						cerr << endl;
					}
				}
				key.clear();
				pattern.clear();
				dimensions.clear();
				is_key = 0;
				is_cost = 0;
				is_pattern = 0;
				is_dims = 0;
			}
		}
	}
	ifs.close();

	if (stat(db_l.c_str(), &buf) == 0) {
		cout << "Found layer database, but will skip import" << endl;
	}

	return 1;
}

void database::printdb() {
	cout << "Database in record: " << endl;
	multimap<key_, config_t>::iterator it;

	for (it = config_map.begin(); it != config_map.end(); it++) {
		config_t config = (*it).second;
		cout << "k " << config.key_s() << "\n";
		cout << "c " << config.cost_s() << "\n";
		cout << "p " << config.pattern_s() << "\n";
		cout << "d " << config.dimensions_s() << "\n";
	}
	cout << "Database End." << endl;
}

void database::printdb_stat() {
	cout << "Total Configurations = " << config_map.size() << endl;
	cout << "Total Layers = " << layer_map.size() << endl;
	cout << endl;
}

void database::pose_mps(const char* filename) {
	uint i = 0;
	string str = dir + "/" + filename + ".mps";
	multimap<key_, config_t>::iterator it;

	cout << "Will write to " << str << endl;
	ofstream ofs(str.c_str());

	ofs << "NAME\tPROB\n";

	ofs << "ROWS\n";
	ofs << " N  COST" << "\n";
	for (i = 0; i < package.size(); i++) {
		ofs << " E  R" << i << "\n";
	}

	ofs << "COLUMNS\n";
	ofs << "    MARK0000  'MARKER'                 'INTORG'\n";

	i = 0;
	for (it = layer_map.begin(); it != layer_map.end(); it++, i++) {
		config_t c = (*it).second;
		ofs << "    C" << i << "\tCOST" << "\t-1\n";
		for (uint j = 0; j < c.get_key().size(); j++) {
			if (c.get_key()[j] > 0)
				ofs << "    C" << i << "\tR" << j << "\t" << c.get_key()[j]
						<< ".\n";
		}
	}
	ofs << "    MARK0001  'MARKER'                 'INTEND'\n";

	ofs << "RHS\n";
	for (i = 0; i < order.size(); i++) {
		ofs << "    RHS\tR" << i << "\t" << order[i] << ".\n";
	}

	ofs << "ENDATA\n";
	ofs.close();
}

void database::pose_lp(const char* filename) {
	uint j = 0;
	string str = dir + "/" + filename + ".lp";
	multimap<key_, config_t>::iterator it;

	cout << "Will write to " << str << endl;
	ofstream ofs(str.c_str());

	ofs << "Maximize\n";
	ofs << "    ";
	it = layer_map.begin();
	it++;
	ofs << "c0";
	for (j = 1; it != layer_map.end(); it++, j++) {
		ofs << " + " << "c" << j;
	}
	ofs << "\n";

	ofs << "Subject To\n";
	for (uint i = 0; i < package.size(); i++) {
		ofs << "    ";
		ofs << "c" << i << ": ";
		it = layer_map.begin();
		ofs << ((*it).first)[i] << " c0";
		it++;
		for (j = 1; it != layer_map.end(); it++, j++) {
			if (((*it).first)[i] != 0)
				ofs << " + " << ((*it).first)[i] << " c" << j;
		}
		ofs << " <= " << order[i];
		ofs << "\n";
	}

	ofs << "Generals\n";
	ofs << "    ";
	it = layer_map.begin();
	ofs << "c0";
	it++;
	for (j = 1; it != layer_map.end(); it++, j++) {
		ofs << " c" << j;
	}
	ofs << "\n";
	ofs << "End\n";

	ofs.close();
}

