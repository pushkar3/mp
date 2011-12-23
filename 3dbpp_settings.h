#ifndef BPP_SOLVER_SETTINGS_H_
#define BPP_SOLVER_SETTINGS_H_

#include <vector>
#include <map>
#include <iostream>
#include <fstream>

class package {
public:
	int id, w, h, d, n;
	package(int _id, int _w, int _h, int _d, int _n) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		n = _n;
	}
};

class bin {
public:
	int id, w, h, d, n;
	bin(int _id, int _w, int _h, int _d, int _n) {
		id = _id;
		w = _w;
		h = _h;
		d = _d;
		n = _n;
	}
};

class input {
public:
	std::vector<package> package_info;
	std::vector<bin> bin_info;

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
			std::cout << package_info[i].id << " " << package_info[i].w
					<< " " << package_info[i].h << " " << package_info[i].d << " " << package_info[i].n << std::endl;
		}
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
					<< bin_info[i].h << " " << bin_info[i].d << " " << bin_info[i].n
					<< std::endl;
		}
	}


};

class database {
public:
	typedef std::vector<int> key;
	typedef double cost;
	std::map<key, cost> arrangement;

	database() {
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
