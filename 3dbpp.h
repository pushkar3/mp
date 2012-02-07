#ifndef BPP_H_
#define BPP_H_

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string.h>
#include <sys/stat.h>

using namespace std;

class database;

typedef float cost;
typedef vector<int> key_;
typedef vector<int> pattern_;
typedef vector<int> dimensions_;
typedef vector<int> order_t;

static int hcf(int x, int y) {
    int tmp;
	if (x < y) {
		tmp = x;
		x = y;
		y = tmp;
	}
	while (x % y != 0) {
		tmp = x % y;
		y = x;
		x = tmp;
	}
	return (y);
}

static int lcm(int x, int y) {
	return (x * y / hcf(x, y));
}

class package_t {
public:
	int id, w, h, d, n;
	int x, y, z;
	package_t(int _id, int _w, int _h, int _d, int _n) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		n = _n;
		x = y = z = 0;
	}

	package_t(int _id, int _w, int _h, int _d, int _x, int _y, int _z) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		x = _x;
		y = _y;
		z = _z;
	}

	int volume() {
		return (w * d * h);
	}
};

class config_t {
	database* d;
	vector<int> key;
	vector<int> pattern;
	float n_density;
	int n_packs;
	int n_area;
	int n_maxw;
	int n_maxd;
	int n_maxh;

	int n_tvolume;
	int n_packvolume;

	vector<int> origin;
	vector<int> corner1, corner2, corner3;
public:
	config_t();
	~config_t() { }
	void reset();
	void set(database* d, key_ key, pattern_ pattern);
	void set_origin(vector<int> o);
	void eval();
	bool operator == (const config_t &c);
	bool is_bound();
	bool is_layer();
	friend ostream & operator <<(ostream &o, const config_t &c);
	void add(const config_t c);
	int get_area();
	vector<int> get_corner(int i);
	key_ get_key();
	pattern_ get_pattern();
	dimensions_ get_dimensions();
	string key_s();
	string pattern_s();
	string dimensions_s();
	string cost_s();
	float density();
};

class bin_t {
public:
	int id, w, h, d, n;
	vector<package_t> package_list;
	bin_t() {
		id = w = h = d = n = 0;
	}

	bin_t(int _id, int _w, int _h, int _d, int _n) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		n = _n;
	}

	int volume() {
		return (w * d * h);
	}

	void add_package(package_t p) {
		package_list.push_back(p);
	}

	float density() {
		float d = 0;
		for (uint i = 0; i < package_list.size(); i++) {
			d += package_list[i].volume();
		}
		return (d / volume());
	}
};

struct classcomp {
	bool operator()(const key_& lhs, const key_& rhs) const {
		int d1 = 0, d2 = 0;
		for (uint i = 0; i < lhs.size(); i++) {
			d1 += lhs[i] * lhs[i];
			d2 += rhs[i] * rhs[i];
		}

		return (d1 < d2);
	}
};

class input {
public:

	vector<package_t> package;
	bin_t bin;
	order_t order;
	string dir;

	input() {
	}

	void load_package_list(const char* filename) {
		ifstream ifs;
		ifs.open(filename);

		int id, w, h, d, n;
		while (!ifs.eof()) {
			ifs >> id >> w >> h >> d >> n;
			if (!ifs.good())
				break;
			package_t p(id, w, h, d, n);
			package.push_back(p);
		}
		ifs.close();
	}

	void print_package_list() {
		for (uint i = 0; i < package.size(); i++) {
			cout << package[i].id << " " << package[i].w << " "
					<< package[i].h << " " << package[i].d << " "
					<< package[i].n << endl;
		}
		cout << endl;
	}

	void load_bin_list(const char* filename) {
		ifstream ifs;
		ifs.open(filename);
		int id, w, h, d, n;
		while (!ifs.eof()) {
			ifs >> id >> w >> h >> d >> n;
			if (!ifs.good())
				break;
			bin_t b(id, w, h, d, n);
			bin = b;
		}
		ifs.close();
	}

	void print_bin_list() {
			cout << bin.id << " " << bin.w << " "
					<< bin.h << " " << bin.d << " "	<< bin.n << endl;
	}

	void load_problem(const char* filename) {
		ifstream ifs;
		ifs.open(filename);
		int id, quantity;
		order_t o (package.size(), 0);

		while (!ifs.eof()) {
			ifs >> id >> quantity;
			if (!ifs.good()) break;
			for (uint i = 0; i < package.size(); i++) {
				if (package[i].id == id) o[i] = quantity;
			}
		}
		order = o;
		ifs.close();
	}

	void print_problem() {
		for (uint i = 0; i < order.size(); i++) {
			cout << order[i] << " ";
		}
		cout << endl;
	}

	void load(const char* dirname) {
		dir.assign(dirname);
		string package_list = dir + "/package_list.txt";
		string bin_list = dir + "/bin_list.txt";
		string problem_list = dir + "/problem_list.txt";

		struct stat buf;
		if (stat(package_list.c_str(), &buf) == 0) {
			cerr << "Loading " << package_list << endl;
			load_package_list(package_list.c_str());
		} else
			cerr << "Package List not found at " << package_list
					<< endl;

		if (stat(bin_list.c_str(), &buf) == 0) {
			cerr << "Loading " << bin_list << endl;
			load_bin_list(bin_list.c_str());
		} else
			cerr << "Bin List not found at " << bin_list << endl;

		if (stat(problem_list.c_str(), &buf) == 0) {
			cerr << "Loading " << bin_list << endl;
			load_problem(problem_list.c_str());
		} else
			cerr << "Problem List not found at " << problem_list
					<< endl;
	}

	void print() {
		cout << "Package List" << endl;
		print_package_list();
		cout << "Bin List" << endl;
		print_bin_list();
		cout << "Order" << endl;
		print_problem();
	}
};

class database {
public:
	vector<package_t> package;
	bin_t bin;
	vector<bin_t> bin_d;
	vector<int> order;
	multimap<key_, config_t, classcomp> config_map;
	multimap<key_, config_t, classcomp> layer_map;
	string dir;
	string db_c, db_l;
	config_t config_last;

	database() { }

	void set_dir(const char* dirname) {
		dir.assign(dirname);
	}

	void get_input(input i) {
		package.clear();
		bin_d.clear();
		order.clear();
		package = i.package;
		order = i.order;
		set_dir(i.dir.c_str());
		db_c = dir + "/db_config.txt";
		db_l= dir + "/db_layer.txt";

		// bin juggad
		int bin_add = 0;
		bin = i.bin;
		for (uint i = 0; i < package.size(); i++) {
			for (uint j = i; j < package.size(); j++) {
				bin_t b = bin;
				b.d = lcm(package[i].d, package[j].d);
				bin_add = 1;
				for (uint k = 0; k < bin_d.size(); k++) {
					if(bin_d[k].d == b.d) {
						bin_add = 0;
						break;
					}
				}

				if(b.d > bin.d) bin_add = 0;

				if(bin_add) {
					bin_d.push_back(b);
					bin_add = 0;
				}
			}
		}

		cout << "New bins" << endl;
		for (uint i = 0; i < bin_d.size(); i++) {
			cout << bin_d[i].id << " " << bin_d[i].w << " "
					<< bin_d[i].h << " " << bin_d[i].d << " "
					<< bin_d[i].n << endl;
		}
		cout << endl;
	}

	int insert(config_t c) {
		return insert(c.get_key(), c.get_pattern());
	}

	int insert(key_ key, pattern_ pattern) {
		config_t config;
		config.set(this, key, pattern);

		multimap<key_, config_t>::iterator it;
		pair <multimap<key_, config_t>::iterator, multimap<key_, config_t>::iterator> ret;
		ret = config_map.equal_range(key);
		for (it = ret.first; it != ret.second; it++) {
			if(config == (*it).second) {
				return 0;
			}
		}

		config_map.insert(pair<key_, config_t> (key, config));
		config_last = config;

		int area = bin.w * bin.h;
		if(config_last.is_layer()) {
			layer_map.insert(pair<key_, config_t> (config_last.get_key(), config_last));
		}

		return 1;
	}

	config_t get_last_inserted_config() {
		return config_last;
	}

	config_t get_layer_from_name(string str) {
		string alphas("abcdefghijklmnopqrstuvwxyz_.");
		size_t found;
		found = str.find_first_of(alphas);
		while (found != string::npos) {
			str[found] = '*';
			found = str.find_first_of(alphas, found + 1);
		}

		int n = atoi(str.c_str());
		multimap<key_, config_t>::iterator it;
		it = layer_map.begin();
		while (n > 0) ++it;
		return (*it).second;
	}

	void find_layers() {
		int area = bin.w * bin.h;
		multimap<key_, config_t>::iterator it;
		for (it = config_map.begin(); it != config_map.end(); it++) {
			config_t c = (*it).second;
			if(((float)(area - c.get_area())/(float)area) < 0.05)
				layer_map.insert(pair<key_, config_t> (c.get_key(), c));
		}
	}

	void exportdb() {
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

	vector<int> deserialize_vector(string str) {
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

	float deserialize_cost(string str) {
		str = str.substr(2);
		int i = atoi	(str.c_str());
		return (float) i;
	}

	int importdb() {
		struct stat buf;
		if (stat(db_c.c_str(), &buf) != 0) return 0;
		cerr << "Found config database at " << db_c << endl;

		ifstream ifs(db_c.c_str());
		string str;
		vector<int> key, pattern, dimensions;
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
					if (insert(key, pattern) )
					{
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

	void printdb() {
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

	void pose_mps(const char* filename) {
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
					ofs << "    C" << i << "\tR" << j << "\t" << c.get_key()[j] << ".\n";
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

	void pose_lp(const char* filename) {
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
			for (j = 1	; it != layer_map.end(); it++, j++) {
				if (((*it).first)[i] != 0) ofs <<  " + " << ((*it).first)[i] << " c" << j;
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

	~database() {
	}

};

class output {
	vector<config_t> packlist;
public:
	output() { }

	~output() {	}

	void insert(config_t c) {
		packlist.push_back(c);
	}

	void exportpl(database* db) {
		string output_list = db->dir + "/pack_list.txt";
		packlist.clear();

		ofstream ofs;
		ofs.open(output_list.c_str());

		for (uint i = 0; i < packlist.size(); i++) {
			config_t config = packlist[i];
			ofs << "k " << config.key_s() << "\n";
			ofs << "c " << config.cost_s() << "\n";
			ofs << "p " << config.pattern_s() << "\n";
			ofs << "d " << config.dimensions_s() << "\n";
		}

		ofs.close();
	}
};

#endif /* BPP_H_ */
