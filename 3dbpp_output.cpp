#include <iostream>
#include <sstream>
#include <tinyxml2.h>
#include "3dbpp.h"

using namespace tinyxml2;

output::output() {
}

output::~output() {
}

void output::insert(config_t c) {
	packlist.push_back(c);
}

void output::clear() {
	packlist_vector.push_back(packlist);
	packlist.clear();
}

void output::set_database(database* _db) {
	db = _db;
	dir.assign(db->get_dir());
}

void output::exportpl() {
	int n = packlist_vector.size() - 1;
	packlist = packlist_vector[n];

	char f_s[100];
	sprintf(f_s, "%s/pack_list_%d.txt", dir.c_str(), n);

	ofstream ofs;
	ofs.open(f_s);

	vector<int> origin(3, 0);

	for (uint i = 0; i < packlist.size(); i++) {
		config_t config = packlist[i];
		if (i > 0)
			origin[2] += packlist[i - 1].get_height();
		config.set_origin(origin);
		ofs << "k " << config.key_s() << "\n";
		ofs << "c " << config.cost_s() << "\n";
		ofs << "p " << config.pattern_s() << "\n";
		ofs << "d " << config.dimensions_s() << "\n";
	}

	ofs.close();
}

XMLElement* output::newTiXMLElement(const char* name, const char* value) {
	XMLElement* element = doc.NewElement(name);
	XMLText* text = doc.NewText(value);
	element->LinkEndChild(text);
	return element;
}

XMLElement* output::newTiXMLElement(const char* name, int value) {
	string str;
	str += value;
	return newTiXMLElement(name, str.c_str());
}

XMLElement* output::newTiXMLElement(const char* name) {
	return doc.NewElement(name);
}

void output::savepl_xml() {
	int order_id_val = 1;

	string str;

	XMLElement* packlist_root = newTiXMLElement("Packlist");
	XMLElement* order_id = newTiXMLElement("OrderID", order_id_val);
	XMLElement* packpallets = newTiXMLElement("PackPallets");
	packlist_root->LinkEndChild(order_id);
	packlist_root->LinkEndChild(packpallets);

	// Mulitplies with the number of pallets
	XMLElement* packpallet = newTiXMLElement("PackPallet");
	packpallets->LinkEndChild(packpallet);

	XMLElement* palletnumber = newTiXMLElement("PalletNumber", 1);
	XMLElement* bruttoweight = newTiXMLElement("BruttoWeight", 0);
	XMLElement* numofpackages = newTiXMLElement("NumberofPackages", packlist.size());
	XMLElement* description = newTiXMLElement("Description");
	XMLElement* dimensions = newTiXMLElement("Dimensions");
	XMLElement* length = newTiXMLElement("Length", db->bin.w);
	XMLElement* width = newTiXMLElement("Width", db->bin.h);
	XMLElement* height = newTiXMLElement("MaxLoadHeight", db->bin.d);
	XMLElement* weight = newTiXMLElement("MaxLoadWeight", 500);
	dimensions->LinkEndChild(length);
	dimensions->LinkEndChild(width);
	dimensions->LinkEndChild(height);
	dimensions->LinkEndChild(weight);

	XMLElement* overhang = newTiXMLElement("Overhang");
	XMLElement* overhang_length = newTiXMLElement("Length", 0);
	XMLElement* overhang_width = newTiXMLElement("Width", 0);
	overhang->LinkEndChild(overhang_length);
	overhang->LinkEndChild(overhang_width);

	XMLElement* packages = newTiXMLElement("Packages");
	for (int i = 0; i < packlist.size(); i++) {
		XMLElement* package = newTiXMLElement("package");
		//Everything package
		packages->LinkEndChild(package);
	}

	packpallet->LinkEndChild(palletnumber);
	packpallet->LinkEndChild(bruttoweight);
	packpallet->LinkEndChild(numofpackages);
	packpallet->LinkEndChild(description);
	packpallet->LinkEndChild(dimensions);
	packpallet->LinkEndChild(overhang);
	packpallet->LinkEndChild(packages);

	doc.SaveFile("op.packlist.xml");
}

void output::importpl() {
	struct stat buf;
	char f_s[100];

	int count = 0;
	while (1) {
		sprintf(f_s, "%s/pack_list_%d.txt", dir.c_str(), count);
		if (stat(f_s, &buf) != 0)
			break;
		cerr << "Found packlist at " << f_s << endl;

		ifstream ifs(f_s);
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
					key = database::deserialize_vector(str);
					is_key = 1;
					break;
				case 'c':
					cost = database::deserialize_cost(str);
					is_cost = 1;
					break;
				case 'p':
					pattern = database::deserialize_vector(str);
					is_pattern = 1;
					break;
				case 'd':
					dimensions = database::deserialize_vector(str);
					is_dims = 1;
					break;
				default:
					printf("error");
				}
				str.clear();

				if (is_key && is_cost && is_pattern && is_dims) {
					config_t c;
					c.set(db, key, pattern);
					packlist.push_back(c);
					if (c.get_dimensions() != dimensions) {
						cerr << "Error while importing packlist at ";
						for (uint i = 0; i < c.get_key().size(); i++) {
							cerr << c.get_key()[i] << " ";
						}
						cerr << endl;
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

		packlist_vector.push_back(packlist);
		packlist.clear();
		count++;
	}
}

double output::find_com_height() {
	int h = 0;
	int h_prev = 0;
	int mass = 0;

	for (uint i = 0; i < packlist.size(); i++) {
		mass += packlist[i].get_weight();
		h += packlist[i].get_weight() * ((packlist[i].get_height() / 2.0)
				+ h_prev);
		h_prev += packlist[i].get_height();
	}

	return (double) h / mass;
}

void output::run_mcmc(int iterations) {

	int n = packlist.size();

	vector<int> pos(n, 0);
	double t[n][n];

	for (int i = 0; i < n; i++) {
		pos.push_back(i);
		for (int j = i; j < n; j++) {
			if (packlist[i].get_weight() > packlist[j].get_weight()) {
				t[i][j] = 8;
				t[j][i] = 2;
			} else if (packlist[i].get_weight() < packlist[j].get_weight()) {
				t[i][j] = 2;
				t[j][i] = 8;
			}
		}
	}

	double pl = 1.0f, pl_new = pl;

	while (iterations > 0) {
		pl = 1.0f;
		pl_new = 1.0f;

		for (int i = 0; i < n - 1; i++) {
			pl *= t[i][i + 1];
		}

		int e1 = rand() % n;
		int e2 = rand() % n;

		swap(packlist[e1], packlist[e2]);
		for (int i = 0; i < n; i++) {
			double temp = t[e1][i];
			t[e1][i] = t[e2][i];
			t[e2][i] = temp;
		}

		for (int i = 0; i < n - 1; i++) {
			pl_new *= t[i][i + 1];
		}

		if ((pl_new < pl) && !(coin_flip(pl_new / pl))) {
			swap(packlist[e2], packlist[e1]);
			for (int i = 0; i < n; i++) {
				double temp = t[e1][i];
				t[e1][i] = t[e2][i];
				t[e2][i] = temp;
			}
		} else {
			//cout << "Accepted swap " << e1 << " and " << e2 << endl;
		}

		iterations--;
		//cout << "Iteration " << iterations << " com: " << find_com_height() << endl;
		packlist_vector.push_back(packlist);
		exportpl();
	}

}

