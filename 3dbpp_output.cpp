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

void output::set_database(database* _db) {
	db = _db;
	dir.assign(db->get_dir());
}

void output::save_packlist() {
	packlist_vector.push_back(packlist);
	packlist.clear();
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
		ofs << "o " << config.orientation_s() << "\n";
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
	char str[10];
	sprintf(str, "%d", value);
	return newTiXMLElement(name, str);
}

XMLElement* output::newTiXMLElement(const char* name) {
	return doc.NewElement(name);
}

void output::savepl_xml() {
	int order_id_val = 1;

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
	int n = 0;
	int h_before = 0;
	vector<int> origin(3, 0);
	for (int i = 0; i < packlist.size(); i++) {
		config_t c = packlist[i];
		if (i > 0) {
			h_before = packlist[i-1].get_height();
			origin[2] += h_before;
		}
		c.set_origin(origin);
		int c1 = 0;
		for (int j = 0; j < c.get_key().size(); j++) {
			for (int k = 0; k < c.get_key()[j]; k++) {
				XMLElement* package = newTiXMLElement("Package");
				XMLElement* packageseq = newTiXMLElement("PackSequence", n++);
				XMLElement* article = newTiXMLElement("Article");
				XMLElement* article_id = newTiXMLElement("ID", db->package[j].id);
				XMLElement* article_desc = newTiXMLElement("Description");
				XMLElement* article_type = newTiXMLElement("Type", 0);
				XMLElement* article_l = newTiXMLElement("Length", db->package[j].w);
				XMLElement* article_w = newTiXMLElement("Width", db->package[j].h);
				XMLElement* article_h = newTiXMLElement("Height", db->package[j].d);
				XMLElement* article_W = newTiXMLElement("Weight", db->package[j].weight);
				XMLElement* article_family = newTiXMLElement("Family", 0);
				article->LinkEndChild(article_id);
				article->LinkEndChild(article_desc);
				article->LinkEndChild(article_type);
				article->LinkEndChild(article_l);
				article->LinkEndChild(article_w);
				article->LinkEndChild(article_h);
				article->LinkEndChild(article_W);
				article->LinkEndChild(article_family);
				XMLElement* barcode = newTiXMLElement("Barcode");
				XMLElement* placeposition = newTiXMLElement("PlacePosition");
				XMLElement* placeposition_x = newTiXMLElement("X", c.get_pattern()[c1*3+0]);
				XMLElement* placeposition_y = newTiXMLElement("X", c.get_pattern()[c1*3+1]);
				XMLElement* placeposition_z = newTiXMLElement("X", c.get_pattern()[c1*3+2]);
				placeposition->LinkEndChild(placeposition_x);
				placeposition->LinkEndChild(placeposition_y);
				placeposition->LinkEndChild(placeposition_z);
				XMLElement* orientation = newTiXMLElement("Orientation", c.get_orientation()[c1]);
				XMLElement* approachpoint1 = newTiXMLElement("ApproachPoint1");
				XMLElement* approachpoint2 = newTiXMLElement("ApproachPoint2");
				XMLElement* approachpoint3 = newTiXMLElement("ApproachPoint3");
				XMLElement* approach1_x = newTiXMLElement("X", 0);
				XMLElement* approach1_y = newTiXMLElement("Y", 0);
				XMLElement* approach1_z = newTiXMLElement("Z", 0);
				approachpoint1->LinkEndChild(approach1_x);
				approachpoint1->LinkEndChild(approach1_y);
				approachpoint1->LinkEndChild(approach1_z);
				XMLElement* approach2_x = newTiXMLElement("X", 0);
				XMLElement* approach2_y = newTiXMLElement("Y", 0);
				XMLElement* approach2_z = newTiXMLElement("Z", 0);
				approachpoint2->LinkEndChild(approach2_x);
				approachpoint2->LinkEndChild(approach2_y);
				approachpoint2->LinkEndChild(approach2_z);
				XMLElement* approach3_x = newTiXMLElement("X", 0);
				XMLElement* approach3_y = newTiXMLElement("Y", 0);
				XMLElement* approach3_z = newTiXMLElement("Z", 0);
				approachpoint3->LinkEndChild(approach3_x);
				approachpoint3->LinkEndChild(approach3_y);
				approachpoint3->LinkEndChild(approach3_z);
				XMLElement* stackheightbefore = newTiXMLElement("StackHeightBefore", h_before);

				package->LinkEndChild(packageseq);
				package->LinkEndChild(article);
				package->LinkEndChild(barcode);
				package->LinkEndChild(placeposition);
				package->LinkEndChild(orientation);
				package->LinkEndChild(approachpoint1);
				package->LinkEndChild(approachpoint2);
				package->LinkEndChild(approachpoint3);
				package->LinkEndChild(stackheightbefore);

				packages->LinkEndChild(package);
				c1++;
			}
		}
	}

	packpallet->LinkEndChild(palletnumber);
	packpallet->LinkEndChild(bruttoweight);
	packpallet->LinkEndChild(numofpackages);
	packpallet->LinkEndChild(description);
	packpallet->LinkEndChild(dimensions);
	packpallet->LinkEndChild(overhang);
	packpallet->LinkEndChild(packages);

	doc.LinkEndChild(packlist_root);

	string packlist_file = dir + "/packlist.xml";
	doc.SaveFile(packlist_file.c_str());
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
				case 'o':
					orientation = database::deserialize_vector(str);
					is_orientation = 1;
					break;
				case 'd':
					dimensions = database::deserialize_vector(str);
					is_dims = 1;
					break;
				default:
					printf("error");
				}
				str.clear();

				if (is_key && is_cost && is_pattern && is_dims && is_orientation) {
					config_t c;
					c.set(db, key, pattern, orientation);
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
					is_orientation = 0;
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

void output::find_order_remaining() {
	vector<int> order_sat(db->order.size());
	vector<int> order_remaining(db->order.size());

	for (uint i = 0; i < packlist.size(); i++) {
		config_t config = packlist[i];
		for (uint j = 0; j < config.get_key().size(); j++) {
			order_sat[j] += config.get_key()[j];
		}
	}

	cout << "Order Remaining: " << endl;
	for (uint i = 0; i < db->order.size(); i++) {
		order_remaining[i] = db->order[i] - order_sat[i];
		cout << i << "->" << order_remaining[i] << endl;
	}
	cout << endl;
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

