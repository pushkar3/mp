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
	friend ostream & operator <<(ostream &o, const config_t &c);
	void add(const config_t c);
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

class order {
public:
	int id, quantity;
	order(int _id, int _quantity) {
		id = _id;
		quantity = _quantity;
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

	vector<package_t> package_info;
	vector<bin_t> bin_info;
	vector<order> order_info;
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
			package_info.push_back(p);
		}
		ifs.close();
	}

	void print_package_list() {
		for (uint i = 0; i < package_info.size(); i++) {
			cout << package_info[i].id << " " << package_info[i].w << " "
					<< package_info[i].h << " " << package_info[i].d << " "
					<< package_info[i].n << endl;
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
			bin_info.push_back(b);
		}
		ifs.close();
	}

	void print_bin_list() {
		for (uint i = 0; i < bin_info.size(); i++) {
			cout << bin_info[i].id << " " << bin_info[i].w << " "
					<< bin_info[i].h << " " << bin_info[i].d << " "
					<< bin_info[i].n << endl;
		}
		cout << endl;
	}

	void load_problem(const char* filename) {
		ifstream ifs;
		ifs.open(filename);
		int id, quantity;
		while (!ifs.eof()) {
			ifs >> id >> quantity;
			if (!ifs.good())
				break;
			order o(id, quantity);
			order_info.push_back(o);
		}
		ifs.close();
	}

	void print_problem() {
		for (uint i = 0; i < order_info.size(); i++) {
			cout << order_info[i].id << " " << order_info[i].quantity
					<< endl;
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
	vector<bin_t> bin_info; // need not be a vector
	vector<order> order_info; // need not be a vector
	vector<package_t> packlist;
	multimap<key_, config_t, classcomp> config_map;
	string dir;
	string db_list;
	config_t config_last;

	database() { }

	void set_dir(const char* dirname) {
		dir.assign(dirname);
	}

	void get_input(input i) {
		package.clear();
		bin_info.clear();
		order_info.clear();
		packlist.clear();
		package = i.package_info;
		order_info = i.order_info;
		set_dir(i.dir.c_str());
		db_list = dir + "/db.txt";

		// bin juggad
		int bin_add = 0;
		bin = i.bin_info[0];
		for (uint i = 0; i < package.size(); i++) {
			for (uint j = i; j < package.size(); j++) {
				bin_t b = bin;
				b.d = lcm(package[i].d, package[j].d);
				bin_add = 1;
				for (uint k = 0; k < bin_info.size(); k++) {
					if(bin_info[k].d == b.d) {
						bin_add = 0;
						break;
					}
				}

				if(b.d > bin.d) bin_add = 0;

				if(bin_add) {
					bin_info.push_back(b);
					bin_add = 0;
				}
			}
		}

		cout << "New bins" << endl;
		for (uint i = 0; i < bin_info.size(); i++) {
			cout << bin_info[i].id << " " << bin_info[i].w << " "
					<< bin_info[i].h << " " << bin_info[i].d << " "
					<< bin_info[i].n << endl;
		}
		cout << endl;
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
		return 1;
	}

	config_t get_last_inserted_config() {
		return config_last;
	}

	void exportdb() {
		ofstream ofs(db_list.c_str());
		multimap<key_, config_t>::iterator it;

		for (it = config_map.begin(); it != config_map.end(); it++) {
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
		int i = atoi(str.c_str());
		return (float) i;
	}

	int importdb() {
		struct stat buf;
		if (stat(db_list.c_str(), &buf) != 0) {
			return 0;
		}

		cerr << "Found database at " << db_list << ". Importing..."
				<< endl;

		ifstream ifs(db_list.c_str());
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
							cerr << "Error while importing database" << endl;
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

	~database() {
	}

};

class output {
	vector<package_t> packlist;
public:
	output() {
	}
	~output() {
	}

	void exportl(database db, const char* dirname) {
		string dir;
		dir.assign(dirname);
		string output_list = dir + "/output_list.txt";

		packlist.clear();
		packlist = db.packlist;

		ofstream ofs;
		ofs.open(output_list.c_str());
		for (uint i = 0; i < packlist.size(); i++) {
			ofs << packlist[i].id << "\t" << packlist[i].w << " "
					<< packlist[i].h << " " << packlist[i].d << "\t"
					<< packlist[i].x << " " << packlist[i].y << " "
					<< packlist[i].z << " " << endl;
		}
		ofs.close();
	}
};

#endif /* BPP_H_ */
