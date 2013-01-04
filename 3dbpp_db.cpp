#include <iostream>
#include <sstream>
#include <tinyxml2.h>
#include "3dbpp.h"

database::database() {
}

database::~database() {
}

database database::clone() {
	database db;
	db.package.assign(package.begin(), package.end());
	db.bin_d.assign(bin_d.begin(), bin_d.end());
	db.order.assign(order.begin(), order.end());
	db.bin = bin;
	db.dir = dir;
	db.db_c = db_c;
	db.db_l = db_l;
	db.insert(config_map);
	return db;
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

	param.load(dir.c_str());

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
				FILE_LOG(logWARNING) << "Adding " << b.d << " for " << package[i].d << " and " << package[j].d;
			}
		}
	}

	FILE_LOG(logWARNING) << "New bins";
	for (uint i = 0; i < bin_d.size(); i++) {
		FILE_LOG(logWARNING) << bin_d[i].id << " " << bin_d[i].w << " " << bin_d[i].h
				<< " " << bin_d[i].d << " " << bin_d[i].n;
	}
	FILE_LOG(logWARNING);
}

int database::insert(config_t c) {
	return insert(c.get_key(), c.get_pattern(), c.get_orientation());
}

int database::insert(key_ key, pattern_ pattern, vector<int> orientation) {
	config_t config;
	config.set(this, key, pattern, orientation);

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
		layer_map.insert(pair<key_, config_t> (config_last.get_key(), config_last));
	}

	return 1;
}

int database::insert(multimap<key_, config_t, classcomp> _config_map) {
	multimap<key_, config_t>::iterator it;
	for (it = _config_map.begin(); it != _config_map.end(); it++) {
		insert((*it).second);
	}
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
		if (((double) (area - c.get_area()) / (double) area) < param.get("layer_density_threshold"))
			layer_map.insert(pair<key_, config_t> (c.get_key(), c));
	}
}

void database::exportdb() {

	ofstream ofs;

	file_lock config_lock(db_c.c_str());
	config_lock.lock();
	ofs.open(db_c.c_str());

	multimap<key_, config_t>::iterator it;

	for (it = config_map.begin(); it != config_map.end(); it++) {
		config_t config = (*it).second;
		ofs << "k " << config.key_s() << "\n";
		ofs << "c " << config.cost_s() << "\n";
		ofs << "p " << config.pattern_s() << "\n";
		ofs << "o " << config.orientation_s() << "\n";
		ofs << "d " << config.dimensions_s() << "\n";
	}
	ofs.flush();
	ofs.close();
	config_lock.unlock();


	file_lock layer_lock(db_l.c_str());
	layer_lock.lock();
	ofs.open(db_l.c_str());

	for (it = layer_map.begin(); it != layer_map.end(); it++) {
		config_t config = (*it).second;
		ofs << "k " << config.key_s() << "\n";
		ofs << "c " << config.cost_s() << "\n";
		ofs << "p " << config.pattern_s() << "\n";
		ofs << "o " << config.orientation_s() << "\n";
		ofs << "d " << config.dimensions_s() << "\n";
	}
	ofs.flush();
	ofs.close();
	layer_lock.unlock();
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

double database::deserialize_cost(string str) {
	str = str.substr(2);
	int i = atoi(str.c_str());
	return (double) i;
}

void database::initdb() {
	struct stat buf;
	if (stat(db_c.c_str(), &buf) != 0) {
		ofstream ofs;
		ofs.open(db_c.c_str());
		usleep(1000);
		ofs.close();
	}
	if (stat(db_l.c_str(), &buf) != 0) {
		ofstream ofs;
		ofs.open(db_l.c_str());
		usleep(1000);
		ofs.close();
	}
}

int database::importdb() {
	struct stat buf;
	if (stat(db_c.c_str(), &buf) != 0) return 0;
	FILE_LOG(logINFO) << "Found config database at " << db_c;

	file_lock config_lock(db_c.c_str());
	config_lock.lock();
	ifstream ifs(db_c.c_str());
	string str;
	key_ key;
	pattern_ pattern;
	vector<int> orientation;
	dimensions_ dimensions;
	double cost;
	int is_key = 0;
	int is_cost = 0;
	int is_pattern = 0;
	int is_orientation = 0;
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
			case 'o':
				orientation = deserialize_vector(str);
				is_orientation = 1;
				break;
			case 'd':
				dimensions = deserialize_vector(str);
				is_dims = 1;
				break;
			default:
				printf("error");
			}
			str.clear();

			if (is_key && is_cost && is_pattern && is_dims && is_orientation) {
				if (insert(key, pattern, orientation)) {
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
				is_orientation = 0;
				is_dims = 0;
			}
		}
	}
	ifs.close();
	config_lock.unlock();

	if (stat(db_l.c_str(), &buf) == 0) {
		FILE_LOG(logINFO) << "Found layer database, but will skip import" << endl;
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

void database::cleandb() {
	package.clear();
	bin_d.clear();
	config_map.clear();
	layer_map.clear();
}

void database::printdb_stat() {
	FILE_LOG(logDEBUG1) << "Total Configurations = " << config_map.size();
	FILE_LOG(logDEBUG1) << "Total Layers = " << layer_map.size();
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
		ofs << " <= " << order[i] * param.get("langrangian_slack_on_order");
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

void database::pose_lpc(const char* filename) {
	uint j = 0;
	string str = dir + "/" + filename + ".lp";
	multimap<key_, config_t>::iterator it;

	cout << "Will write to " << str << endl;
	ofstream ofs(str.c_str());

	ofs << "Maximize\n";
	ofs << "    ";
	it = config_map.begin();
	it++;
	ofs << "c0";
	for (j = 1; it != config_map.end(); it++, j++) {
		ofs << " + " << "c" << j;
	}
	ofs << "\n";

	ofs << "Subject To\n";
	for (uint i = 0; i < package.size(); i++) {
		ofs << "    ";
		ofs << "c" << i << ": ";
		it = config_map.begin();
		ofs << ((*it).first)[i] << " c0";
		it++;
		for (j = 1; it != config_map.end(); it++, j++) {
			if (((*it).first)[i] != 0)
				ofs << " + " << ((*it).first)[i] << " c" << j;
		}
		ofs << " <= " << order[i] * param.get("langrangian_slack_on_order");
		ofs << "\n";
	}

	ofs << "Generals\n";
	ofs << "    ";
	it = config_map.begin();
	ofs << "c0";
	it++;
	for (j = 1; it != config_map.end(); it++, j++) {
		ofs << " c" << j;
	}
	ofs << "\n";
	ofs << "End\n";

	ofs.close();
}
