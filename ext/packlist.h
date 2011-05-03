/*
 * Class structure to parse input orders for a mixed pallet.
 * KUKA input file definition for orders is used here.
 *
 * Author: Pushkar Kolhe
*/

#ifndef PACKLIST_H_
#define PACKLIST_H_

#include <vector>
#include <string.h>
#include "xml_parser.h"

class Article {
public:
	unsigned int id;
	std::string description;
	unsigned int type;
	unsigned int length;
	unsigned int width;
	unsigned int height;
	unsigned int weight;
	unsigned int family;

	Article() {
		id = 0;
		type = 0;
		length = 0;
		width = 0;
		height = 0;
		weight= 0;
		family = 0;
	}
	~Article() {}
	int parse(std::string data);
	std::string xml();
	void set(int _length, int _width, int _height, int _weight) {
		length = _length;
		width = _width;
		height = _height;
		weight = _weight;
	}
};

class Barcode {
public:
	std::string code;

	Barcode() {
		code = " ";
	}
	~Barcode() {}
	int parse(std::string data);
	std::string xml();
};

class OrderLine {
public:
	unsigned int orderlineno;
	Article article;
	std::vector<Barcode> barcode;

	OrderLine() {
		orderlineno = 0;
	}
	~OrderLine() {}
	unsigned n_barcode() { return barcode.size(); }
	int parse(std::string data);
};

class Restrictions {
public:
	bool familygrouping;
	bool ranking;

	Restrictions() {
		familygrouping = false;
		ranking = false;
	}
	~Restrictions() {}
	int parse(std::string data);
};

class Order {
public:
	unsigned int id;
	std::string description;
	Restrictions restriction;
	std::vector <OrderLine> orderline;

	Order() {
		id = 1;
	}
	~Order() {}
	unsigned int n_orderline() { return orderline.size(); }
	int parse(std::string data);
};

class Pallet {
public:
	int palletnumber;
	std::string description;
	int length;
	int width;
	int maxloadheight;
	int maxloadweight;

	Pallet() {
		palletnumber = 0;
		length = 0;
		width = 0;
		maxloadheight = 0;
		maxloadweight = 0;
	}
	~Pallet() {}
	int parse(std::string data);
};

class OrderXML {
public:
	std::vector<Pallet> pallet;
	unsigned int n_pallet() { return pallet.size(); }
	Order order;

	OrderXML() {}
	~OrderXML() {}
	int parse(const char* filename, int debug_p = 0, int debug_o = 0);
};


#endif
