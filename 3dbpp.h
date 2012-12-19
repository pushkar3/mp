/** @3dbpp.h
 *  @brief Class prototypes and declarations for 3D bin packing
 *
 *  The algorithm uses dynamic programming to construct layers and column-based optimization for bin packing
 *
 *  @todo Logging framework
 *  @author Pushkar Kolhe
 */

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

#define BOLD "\033[1m"
#define NORMAL "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"

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

/**
 * @brief A package is what needs to be binpacked
 */
class package_t {
public:
	int id, w, h, d, n;
	int x, y, z;
	int weight;
	string description;
	vector<string> barcode;
	vector<int> vid;
	uint barcode_c;
	int type, family;
	uint id_c;
	package_t(int _id, int _w, int _h, int _d, int _n) :
		id(_id), w(_w), h(_h), d(_d), n(_n) {
		x = y = z = 0;
		barcode_c = 0;
		id_c = 0;
	}

	package_t(int _id, int _w, int _h, int _d, int _x, int _y, int _z) :
		id(_id), w(_w), h(_h), d(_d), x(_x), y(_y), z(_z) {
		barcode_c = 0;
		id_c = 0;
	}

	void set_description(const char* value) {
		description.assign(value);
	}

	void set_id(vector<int> _ids) {
		vid.assign(_ids.begin(), _ids.end());
	}

	int get_id() {
		int ret = vid[id_c++];
		if(id_c == vid.size()) id_c = 0;
		return ret;
	}

	void insert_barcode(const char* value) {
		barcode.push_back(value);
	}

	string get_new_barcode() {
		string str = barcode[barcode_c];
		barcode_c++;
		if(barcode_c == barcode.size()) barcode_c = 0;
		return str;
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

/**
 * @brief A packaging is made for a bin
 */
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

	int get_area() {
		return (w * d);
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

/**
 * @brief Compares basic types key and packages
 */
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

/**
 * @brief Loads external parameters
 */
class bpp_param {
	XMLDocument doc;
	XMLElement* params;
public:
	bpp_param();
	~bpp_param();
	int load(const char* dir);
	double get(const char* name);
};

/**
 * @brief Smallest unit of a binpack
 *
 * Multiple configurations can be put together to create other configurations or layers
 */
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
	/** reset() is called. Initializes all variables */
	config_t();

	~config_t();

	/** Initializes all variables */
	void reset();

	/** Sets internal variables for this particular configuration */
	void set(database* d, key_ key, pattern_ pattern, vector<int> orientation);

	/** Gets the origin of this configuration. Usually the point closer to 0,0 0 */
	vector<int> get_origin();

	/** Sets the origin */
	void set_origin(vector<int> o);

	/** Compares if the two configurations are equal in size. Checks the number of packages and their dimensions. */
	bool operator ==(const config_t &c);

	/** Checks if the configuration is bigger than the bin or not */
	bool is_bound();

	/** Checks if the configuration is a layer or not by comparing density difference */
	bool is_layer();

	/** Writes the configuration to a stringstream */
	friend ostream & operator <<(ostream &o, const config_t &c);

	/** Adds a configuration to the current one. Set origin of the configuration which will be added on using set_origin() before calling this function */
	void add(const config_t c);

	/** Adds a configuration to the current one. Set origin of the configuration which will be added on using set_origin() before calling this function
	 * @bug Same as add(), but had to use this to remove a segfault */
	void add(database* db, const config_t c); // Hack to remove a segfault

	/** Returns height of this configuration */
	int get_height();

	/** Returns are of this configuration */
	int get_area();

	/** Returns corner this configuration. Input is 0-2. 0, 1, 2 return corners along w, h, d respectively */
	vector<int> get_corner(int i);

	/** Returns the key of this configuration */
	key_ get_key();

	/** Returns the pattern of this configuration */
	pattern_ get_pattern();

	/** Returns the orientation vector of this configuration */
	vector<int> get_orientation();

	/** Returns dimensions of this configuration */
	dimensions_ get_dimensions();

	/** Returns weight of this configuration */
	int get_weight();

	/** Returns the total volume of all packages in this configuration */
	int get_packagevolume();

	/** Returns the total volume taken by this configuration, as derived from it w, h and d */
	int get_totalvolume();

	/** Returns the total number of packages in this configuration */
	int get_totalpacks();

	/** Rotates the configuration by 90 degrees */
	void change_orientation();

	/** Rotates the configuration by 90 degrees
	* @bug Same as change_orientation(), but had to use this to remove a segfault */
	void change_orientation(database* db); // hack to remove a segfault

	string key_s();
	string pattern_s();
	string orientation_s();
	string dimensions_s();
	string cost_s();
	double density();

	/** Sets maximum gap between packages */
	void set_gap(int x, int y);

	/** Spreads out the packages in the configuration using the gap provided */
	void spread_out(bin_t bin);

	/** Centers the configuration in the bin center  */
	void center_in(bin_t bin);

	/** Rotates the configuration by 180 degrees */
	void rotate();

	/** Commpares the key of the package, that is the number of packages of each type */
	int compare(config_t c);
};

/**
 * @brief Gets input from XML or Text format
 */
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
	void load_old(const char* dirname);
	void load_xml(const char* pre_c);
	void print();
	void exporti();
};

/**
 * @brief Database for storing configurations and layers
 *
 * Configurations are smaller parts of layers that are not as wide as the bin size.
 */
class database {
public:
	vector<package_t> package; 							/*< Layers are made using these packets */
	bin_t bin;											/*< Layers have to be created for this bin */
	vector<int> order;									/*< Layers are made for these orders */
	multimap<key_, config_t, classcomp> config_map;     /*< Configurations are stored here - Input<number of each package type, geometric configuration> */
	multimap<key_, config_t, classcomp> layer_map;		/*< Layers are stored here - Input<number of each package type, geometric configuration> */

	vector<bin_t> bin_d;								/*< Layers are made for these bins. Temp variable. Juggad in get_input() */
	string dir;
	string db_c, db_l;
	config_t config_last;

	/** Empty */
	database();

	/** Empty */
	~database();

	/** Clones the database.
	 * Copies over all class attributes except layer_map */
	database clone();

	/** Set directory where database will reside */
	void set_dir(const char* dirname);

	/** Returns directory name where database is stored */
	const char* get_dir();

	/** Initializes and populates the database object. */
	void get_input(input i);

	/** Inserts a configuration in the database */
	int insert(config_t c);

	/** Inserts a configuration in the database using parameters
	 * \param key Number of packages of each type
	 * \param pattern The geometric position of each package
	 * \param orientation The orientation for each package. If 1, package is rotated and placed. */
	int insert(key_ key, pattern_ pattern, vector<int> orientation);

	/** Function to help database cloning. Copies config_map */
	int insert(multimap<key_, config_t, classcomp> _config_map);

	/** Returns the last inserted configuration */
	config_t get_last_inserted_config();

	/** Function for accessing layers using string names for Gurobi calls. Example: c1 returns the 2nd layer in layer_map */
	config_t get_layer_from_name(string str);

	/** Populates layer_map from config_map */
	void find_layers();

	/** Exports database in a text file. If the file is present, it is rewritten */
	void exportdb();

	/** Converts a string of numbers into a vector of integers */
	static vector<int> deserialize_vector(string str);

	/** Converts a string into a double */
	static double deserialize_cost(string str);

	/** Initializes the database. Creates the empty files required. */
	void initdb();

	/** Imports the database from a text file */
	int importdb();

	/** Prints the database on the terminal */
	void printdb();

	/** Cleans the database object and reinitializes all the variables */
	void cleandb();

	/** Print database stats. It can print the total configurations and layers it has found */
	void printdb_stat();

	/** Poses the binpacking problem in MPS format for Gurobi */
	void pose_mps(const char* filename);

	/** Poses the binpacking problem in LP format for Gurobi */
	void pose_lp(const char* filename);

	/** Poses the binpacking problem in LPC format for Gurobi */
	void pose_lpc(const char* filename);
};

/**
 * @brief Exports database or packlists in XML or Text format
 */
class output {
	string dir;
	database *db;
	vector<config_t> _packlist;
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
	void insert(int bin_n, config_t c);
	int get_height(int bin_n);
	int get_n_configs(int bin_n);
	config_t get_config_on_top(int bin_n);
	void save_packlist();
	void gen_bins(int n);
	void set_database(database* _db);
	void exportpl();
	void savepl_xml();
	void importpl();
	double find_com_height();
	void run_mcmc(int iterations);
};

// Post Plan
packlist_ greedy_select(database* d);
void postplan(packlist_ pl, database *d, output* o);

/**
 * @brief Used for running high level layer-finding jobs
 */
class binpack_job {
	double tolerance;
	int is_singlepackage;
	int package_key;
	double h_max;
	int completed;
public:
	database d;
	database d1, d2;
	binpack_job();
	~binpack_job();
	int is_singlepack();
	double get_height();
	int get_packagekey();
	void set_singlepackage(database* db, int _key, int _hmax);
	void set_multiplepackage(database db1, database db2, double _hmax);
	void singlepackageconfig();
	void multiplepackageconfig();
	void run();
};

#endif /* BPP_H_ */
