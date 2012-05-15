#ifndef BPP_H_
#define BPP_H_

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string.h>
#include <sys/stat.h>
#include <cmath>

#include <tinyxml2.h>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

using namespace std;
using namespace tinyxml2;
using namespace boost::interprocess;

class bpp_param;
class database;
class config_t;

extern bpp_param param;

typedef double cost;
typedef vector<int> key_;
typedef vector<int> pattern_;
typedef vector<int> dimensions_;
typedef vector<int> order_t;
typedef vector<config_t> packlist_;

static int lcm(int a, int b) {
	int n;
	for (n = 1;; n++) {
		if (n % a == 0 && n % b == 0)
			return n;
	}
}

static int fact(int n) {
	int ret = 1;
	while (n > 0) {
		ret *= n;
		n--;
	}
	return ret;
}

static int coin_flip(double p) {
	int n = 10;
	int x = n / 2;

	double y = (fact(n) / (fact(x) * fact(n - x))) * pow(p, x) * pow(1.0 - p,
			n - x);
	if (y > 0.5)
		return 1;
	return 0;
}

class package_t {
public:
	int id, w, h, d, n;
	int x, y, z;
	int weight;
	package_t(int _id, int _w, int _h, int _d, int _n) :
		id(_id), w(_w), h(_h), d(_d), n(_n) {
		x = y = z = 0;
	}

	package_t(int _id, int _w, int _h, int _d, int _x, int _y, int _z) :
		id(_id), w(_w), h(_h), d(_d), x(_x), y(_y), z(_z) {
	}

	void set_weight(int _w) {
		weight = _w;
	}

	int get_weight() {
		return weight;
	}

	int volume() {
		return (w * d * h);
	}

	void print() {
		cout << id << " " << w << " " << h << " "  << d << " " << endl;
	}
};

class bin_t {
public:
	int id, w, h, d, n;
	vector<package_t> package_list;
	bin_t() {
		id = w = h = d = n = 0;
	}

	bin_t(int _id, int _w, int _h, int _d, int _n) :
		id(_id), w(_w), h(_h), d(_d), n(_n) {
	}

	int volume() {
		return (w * d * h);
	}

	void add_package(package_t p) {
		package_list.push_back(p);
	}

	double density() {
		double d = 0;
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

	bool operator()(const package_t& lhs, const package_t& rhs) const {
		int l = lhs.w*lhs.d*lhs.h;
		int r = rhs.w*rhs.d*rhs.h;
		return (l < r);
	}
};

class bpp_param {
	XMLDocument doc;
	XMLElement* params;
public:
	bpp_param();
	~bpp_param();
	int load(const char* dir);
	double get(const char* name);
};

class config_t {
	database* d;
	vector<int> key;
	vector<int> pattern;
	vector<int> orientation;
	double n_density;
	int n_packs;
	int n_area;
	int n_maxw;
	int n_maxd;
	int n_maxh;

	int n_tvolume;
	int n_packvolume;
	int n_packweight;

	int gap_w;
	int gap_h;

	vector<int> origin;
	vector<int> corner1, corner2, corner3;
public:
	config_t();
	~config_t();
	void reset();
	void set(database* d, key_ key, pattern_ pattern, vector<int> orientation);
	void recalc();
	vector<int> get_origin();
	void set_origin(vector<int> o);
	void eval();
	bool operator ==(const config_t &c);
	bool is_bound();
	bool is_layer();
	friend ostream & operator <<(ostream &o, const config_t &c);
	void add(const config_t c);
	void add(database* db, const config_t c); // hack to remove a segfault
	int get_height();
	int get_area();
	vector<int> get_corner(int i);
	key_ get_key();
	pattern_ get_pattern();
	vector<int> get_orientation();
	dimensions_ get_dimensions();
	int get_weight();
	int get_packagevolume();
	int get_totalvolume();
	int get_totalpacks();
	void change_orientation();
	void change_orientation(database* db); // hack to remove a segfault
	string key_s();
	string pattern_s();
	string orientation_s();
	string dimensions_s();
	string cost_s();
	double density();
	void set_gap(int x, int y);
	void spread_out(bin_t bin);
};

class input {
public:

	vector<package_t> package_orig;
	vector<package_t> package;
	bin_t bin;
	order_t order;
	string dir;

	input();
	void load_package_list(const char* filename);
	void print_package_list();
	void load_bin_list(const char* filename);
	void print_bin_list();
	void load_problem(const char* filename);
	void print_problem();
	void load(const char* dirname);
	void load_xml(const char* pre_c);
	void print();
	void exporti();
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

	database();
	~database();
	database clone();
	void set_dir(const char* dirname);
	const char* get_dir();
	void get_input(input i);
	int insert(config_t c);
	int insert(key_ key, pattern_ pattern, vector<int> orientation);
	int insert(multimap<key_, config_t, classcomp> _config_map);
	config_t get_last_inserted_config();
	config_t get_layer_from_name(string str);
	void find_layers();
	void exportdb();
	static vector<int> deserialize_vector(string str);
	static double deserialize_cost(string str);
	void initdb();
	int importdb();
	void printdb();
	void cleandb();
	void printdb_stat();
	void pose_mps(const char* filename);
	void pose_lp(const char* filename);
};

class output {
	string dir;
	database *db;
	vector<config_t> packlist;
	tinyxml2::XMLDocument doc;
	vector<int> order_remaining;

	tinyxml2::XMLElement* newTiXMLElement(const char* name, const char* value);
	tinyxml2::XMLElement* newTiXMLElement(const char* name, int value);
	tinyxml2::XMLElement* newTiXMLElement(const char* name);

public:
	vector<packlist_> packlist_vector;

	output();
	~output();
	void insert(config_t c);
	void clear();
	void set_database(database* _db);
	void exportpl();
	void savepl_xml();
	void importpl();
	void find_order_remaining();
	double find_com_height();
	void run_mcmc(int iterations);
};

#endif /* BPP_H_ */
