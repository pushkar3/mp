/*
 * Class structure to parse output pallet configuration for a mixed pallet.
 * KUKA output file definition for orders is used here.
 *
 * Author: Pushkar Kolhe
*/

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "packlist.h"
#include <vector>

class Point {
public:
	int x;
	int y;
	int z;

	Point() {
		x = 0; y = 0; z = 0;
	}
	~Point() {}
	int parse(std::string data);
	std::string xml(const char* name);

	void set(int _x, int _y, int _z) {
		x = _x; y = _y; z = _z;
	}

	Point add(Point d) {
		Point a;
		a.x = x+d.x; a.y = y+d.y; a.z = z+d.z;
		return a;
	}


	Point subtract(Point d) {
		Point a;
		a.x = x-d.x; a.y = y-d.y; a.z = z-d.z;
		return a;
	}

	Point mult(float n) {
		Point a;
		a.x = x*n; a.y = y*n; a.z = z*n;
		return a;
	}
};

class Package {
public:
	unsigned int pack_sequence;
	unsigned int incoming_sequence;
	unsigned int orderlineno;
	unsigned int parent_layer;
	Article article;
	Barcode barcode;
	Point place_position;
	unsigned int orientation;
	Point approach_point_1;
	Point approach_point_2;
	Point approach_point_3;
	unsigned int stack_height_before;

	double normal_force;
	double min_x, max_x, min_y, max_y, z;
	std::vector<Package> connect;

	Package() {
		pack_sequence = 0;
		incoming_sequence = 0;
		orderlineno = 0;
		parent_layer = 0;
		orientation = 1;
		stack_height_before = 0;
		normal_force = 0;
		min_x = 0;
		max_x = 0;
		min_y = 0;
		max_y = 0;
		z = 0;
	};
	~Package() {};
	int parse(std::string data);
	std::string xml();
	void find_boundaries() {
		double l, w;

		if(orientation == 1) {
			l = (double) article.length;
			w = (double) article.width;
		}
		else {
			l = (double) article.width;
			w = (double) article.length;
		}
		min_x = place_position.x - l/2.0f;
		max_x = place_position.x + l/2.0f;
		min_y = place_position.y - w/2.0f;
		max_y = place_position.y + w/2.0f;
		z = place_position.z - article.height/2.0f;
	}
};

class Dimensions {
public:
	unsigned int length;
	unsigned int width;
	unsigned int max_load_weight;
	unsigned int max_load_height;

	Dimensions() {
		length = 0;
		width = 0;
		max_load_weight = 0;
		max_load_height = 0;
	}
	~Dimensions() {}
	int parse(std::string data);
	std::string xml();

	void set(int _length, int _width, int _height, int _weight) {
		length = _length;
		width = _width;
		max_load_height = _height;
		max_load_weight = _weight;
	}
};

class PackPallet {
public:
	unsigned int pallet_number;
	unsigned int brutto_weight;
	unsigned int number_of_packages;
	std::string description;
	Dimensions dimension;
	std::vector<Package> package;

	PackPallet() {
		pallet_number = 0;
		brutto_weight = 0;
		number_of_packages = 0;
	};
	~PackPallet() {};
	unsigned int n_package() { return package.size(); }
	int parse(std::string data);
	std::string xml();

	void insertPackage(Package pack, int w, int h, int d, int e) {
		package.push_back(pack);
		package[n_package()-1].pack_sequence = n_package();
		package[n_package()-1].article.set(w, h, d, e);
	}

	// Call once
	// Might be deprecated! Haven't used this for a while (Remove from viewer.cpp)
	void find_normals() {
		for(int i = n_package()-1; i >= 0; i--) {
			package[i].normal_force = 0.0f;
			package[i].normal_force += package[i].article.weight;
			for(unsigned int j = i+1; j < n_package(); j++) {
				// All packages	 above this one
				if(package[j].z > package[i].z) {
					double s = 0.1f, len = 0.0f, wid = 0.0f;
					for(double l = 0; l < dimension.length; l+=s)
						if( (package[i].min_x <= l && l <= package[i].max_x) && (package[j].min_x <= l && l <= package[j].max_x)) len += s;
					for(double w = 0; w < dimension.width ; w+=s)
						if( (package[i].min_y <= w && w <= package[i].max_y) && (package[j].min_y <= w && w <= package[j].max_y) ) wid += s;

					double area = len*wid;
					if(area > 0) package[j].connect.push_back(package[i]);
					package[i].normal_force += package[j].normal_force*area;
				}
			}
		}
	}
};

class PackList {
public:
	unsigned int order_id;
	std::vector<PackPallet> packpallet;

	PackList() {
		order_id = 1;
	};
	~PackList() {};
	unsigned int n_packpallet() { return packpallet.size(); }
	void insertPallet(PackPallet pallet, int W, int H, int D, int E) {
		packpallet.push_back(pallet);
		packpallet[n_packpallet()-1].pallet_number = n_packpallet();
		packpallet[n_packpallet()-1].dimension.set(W, H, D, E);
	}
	int parse(std::string data);
	std::string xml();
};

PackList read_response(const char* filename, int debug);
void write_response(PackList list, const char* filename, int debug);

#endif /* RESPONSE_H_ */
