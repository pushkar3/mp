#ifndef BPP_SOLVER_SETTINGS_H_
#define BPP_SOLVER_SETTINGS_H_

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string.h>

typedef double cost;
typedef std::vector<int> key;
typedef std::vector<int> pattern;

class package {
public:
	int id, w, h, d, n;
	int x, y, z;
	package(int _id, int _w, int _h, int _d, int _n) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		n = _n;
		x = y = z = 0;
	}

	package(int _id, int _w, int _h, int _d, int _x, int _y, int _z) {
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

class bin {
public:
	int id, w, h, d, n;
	std::vector<package> package_list;
	bin(int _id, int _w, int _h, int _d, int _n) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		n = _n;
	}

	int volume() {
		return (w * d * h);
	}

	void add_package(package p) {
		package_list.push_back(p);
	}

	double density() {
		double d = 0;
		for (int i = 0; i < package_list.size(); i++) {
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
	bool operator()(const key& lhs, const key& rhs) const {
		int d1 = 0, d2 = 0;
		for (int i = 0; i < lhs.size(); i++) {
			d1 += lhs[i] * lhs[i];
			d2 += rhs[i] * rhs[i];
		}

		return (d1 < d2);
	}
};

class input {
public:
	std::vector<package> package_info;
	std::vector<bin> bin_info;
	std::vector<order> order_info;

	void load_package_list(const char* filename) {
		std::ifstream ifs;
		ifs.open(filename);
		int id, w, h, d, n;
		while (!ifs.eof()) {
			ifs >> id >> w >> h >> d >> n;
			if (!ifs.good())
				break;
			package p(id, w, h, d, n);
			package_info.push_back(p);
		}
		ifs.close();
	}

	void print_package_list() {
		for (int i = 0; i < package_info.size(); i++) {
			std::cout << package_info[i].id << " " << package_info[i].w << " "
					<< package_info[i].h << " " << package_info[i].d << " "
					<< package_info[i].n << std::endl;
		}
		std::cout << std::endl;
	}

	void load_bin_list(const char* filename) {
		std::ifstream ifs;
		ifs.open(filename);
		int id, w, h, d, n;
		while (!ifs.eof()) {
			ifs >> id >> w >> h >> d >> n;
			if (!ifs.good())
				break;
			bin b(id, w, h, d, n);
			bin_info.push_back(b);
		}
		ifs.close();
	}

	void print_bin_list() {
		for (int i = 0; i < bin_info.size(); i++) {
			std::cout << bin_info[i].id << " " << bin_info[i].w << " "
					<< bin_info[i].h << " " << bin_info[i].d << " "
					<< bin_info[i].n << std::endl;
		}
		std::cout << std::endl;
	}

	void load_problem(const char* filename) {
		std::ifstream ifs;
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
		for (int i = 0; i < order_info.size(); i++) {
			std::cout << order_info[i].id << " " << order_info[i].quantity
					<< std::endl;
		}
		std::cout << std::endl;
	}

};

class output {
	std::vector<package> packlist;
public:
	output() {}
	~output() {}
	void insert(package p) {
		packlist.push_back(p);
	}

	void exportl(const char* filename) {
		std::ofstream ofs;
		ofs.open(filename);
		for (int i = 0; i < packlist.size(); i++) {
			ofs << packlist[i].id << "\t"
					<< packlist[i].w << " "
					<< packlist[i].h << " "
					<< packlist[i].d << "\t"
					<< packlist[i].x << " "
					<< packlist[i].y << " "
					<< packlist[i].z << " " << std::endl;
		}
		ofs.close();
	}
};

class database {
public:
	input* i;
	output* o;
	std::map<key, cost, classcomp> layer_cost;
	std::map<key, pattern, classcomp> layer_pattern;

	database(input* _i, output* _o) {
		i = _i;
		o = _o;
	}

	void insert(key _key, pattern _pattern, int _cost) {
		layer_pattern.insert(std::pair<key, pattern>(_key, _pattern));
		layer_cost.insert(std::pair<key, cost>(_key, _cost));
	}

	void exportdb(const char* filename) {
		std::ofstream ofs(filename);
		std::map<key, pattern>::iterator itp;
		std::map<key, cost>::iterator itc;

		for (itp = layer_pattern.begin(), itc = layer_cost.begin(); itp
				!= layer_pattern.end(); itp++, itc++) {

			ofs << "k ";
			for (int i = 0; i < (*itp).first.size(); i++)
				ofs << (*itp).first[i] << " ";
			ofs << "\n";

			ofs << "c " << (*itc).second << "\n";

			ofs << "p";
			for (int i = 0; i < (*itp).second.size(); i++)
				ofs << (*itp).second[i] << " ";
			ofs << '\n';

		}
		ofs.close();
	}

	std::vector<int> deserialize_vector(std::string str) {
		std::vector<int> vec;
		char* p;
		str = str.substr(2); // remove first char and space
		char* c = (char*) str.c_str();
		p = strtok(c, " ");
		while(p != NULL) {
			vec.push_back(atoi(p));
			p = strtok(NULL, " ");
		}

		return vec;
	}

	double deserialize_cost(std::string str) {
		str = str.substr(2);
		int i = atoi(str.c_str());
		return (double) i;
	}

	void importdb(const char* filename) {
		std::ifstream ifs(filename);
		std::string str;
		std::vector<int> key, pattern;
		double cost;
		int is_key = 0;
		int is_cost= 0;
		int is_pattern = 0;

		while(ifs.good()) {
			char c = ifs.get();
			if(c != '\n') {
				str.push_back(c);
			}
			else {
				switch (str.at(0)) {
				case 'k': key = deserialize_vector(str); is_key = 1; break;
				case 'c': cost = deserialize_cost(str); is_cost = 1; break;
				case 'p': pattern = deserialize_vector(str); is_pattern = 1; break;
				default: printf("error");
				}
				str.clear();

				if(is_key && is_cost && is_pattern) {
					insert(key, pattern, cost);
					key.clear();
					pattern.clear();
					is_key = 0;
					is_cost = 0;
					is_pattern = 0;
				}
			}
		}
		ifs.close();
	}

	void printdb() {
		std::map<key, pattern>::iterator itp;
		std::map<key, cost>::iterator itc;

		std::cout << "Database in record: " << std::endl;
		for (itp = layer_pattern.begin(), itc = layer_cost.begin(); itp
				!= layer_pattern.end(); itp++, itc++) {

			for (int i = 0; i < (*itp).first.size(); i++)
				std::cout << (*itp).first[i] << " ";

			std::cout << "\t" << (*itc).second << "\t";

			for (int i = 0; i < (*itp).second.size(); i++)
				std::cout << (*itp).second[i] << " ";

			std::cout << '\n';
		}
		std::cout << "Database End." << std::endl;

	}

	~database() {
	}

};

class bpp_settings {
public:
	int tests;
	int bdim, type, packingtype;
	int nodelimit, iterlimit, timelimit, nodeused, iterused, timeused;
	int ub, lb, solved, gap, sumnode, sumiter;
	double time, sumtime, deviation, sumub, sumlb, sumdev;

	bpp_settings();
	~bpp_settings();

	bpp_settings(const bpp_settings& s);
	void set_default();
	void show_start();
	void show_end();
	void reset();
};

#endif /* BPP_SOLVER_SETTINGS_H_ */