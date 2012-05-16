#include <iostream>
#include <sstream>
#include <tinyxml2.h>
#include "3dbpp.h"

using namespace tinyxml2;

input::input() {
}

void input::load_package_list(const char* filename) {
	ifstream ifs;
	ifs.open(filename);

	int id, w, h, d, n, weight;
	while (!ifs.eof()) {
		ifs >> id >> w >> h >> d >> n >> weight;
		if (!ifs.good())
			break;
		package_t p(id, w, h, d, n);
		p.set_weight(weight);
		package.push_back(p);
		package_orig.push_back(p);
	}
	ifs.close();
}

void input::print_package_list() {
	cout << "Id \t W \t H \t D \t N" << endl;
	for (uint i = 0; i < package.size(); i++) {
		cout << package[i].id << "\t" << package[i].w << "\t" << package[i].h
				<< "\t" << package[i].d << "\t" << package[i].n << endl;
//		for (int j = 0; j < package[i].vid.size(); j++) {
//			cout << package[i].vid[j] << " " << endl;
//		}
	}
	cout << endl;
}

void input::load_bin_list(const char* filename) {
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

void input::print_bin_list() {
	cout << "Id \t W \t H \t D \t N" << endl;
	cout << bin.id << "\t" << bin.w << "\t" << bin.h << "\t" << bin.d << "\t"
			<< bin.n << endl;
	cout << endl;
}

void input::load_problem(const char* filename) {
	ifstream ifs;
	ifs.open(filename);
	int id, quantity;
	order_t o(package.size(), 0);

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

void input::print_problem() {
	cout << "Id \t N" << endl;
	for (uint i = 0; i < order.size(); i++) {
		cout << package[i].id << "\t" << order[i] << endl;
	}
	cout << endl;
}

void input::load_old(const char* dirname) {
	dir.assign(dirname);
	string package_list = dir + "/package_list.txt";
	string bin_list = dir + "/bin_list.txt";
	string problem_list = dir + "/problem_list.txt";

	struct stat buf;
	if (stat(package_list.c_str(), &buf) == 0) {
		cerr << "Loading " << package_list << endl;
		load_package_list(package_list.c_str());
	} else
		cerr << "Package List not found at " << package_list << endl;

	if (stat(bin_list.c_str(), &buf) == 0) {
		cerr << "Loading " << bin_list << endl;
		load_bin_list(bin_list.c_str());
	} else
		cerr << "Bin List not found at " << bin_list << endl;

	if (stat(problem_list.c_str(), &buf) == 0) {
		cerr << "Loading " << bin_list << endl;
		load_problem(problem_list.c_str());
	} else
		cerr << "Problem List not found at " << problem_list << endl;
}

void input::load_xml(const char* pre_c) {
	string pre(pre_c);
	string order_file = pre + "/order.xml";
	dir = pre;

	XMLDocument order_xml;
	order_xml.LoadFile(order_file.c_str());

	XMLElement* pallet_dimensions =	order_xml.FirstChildElement("Message")
			->FirstChildElement("PalletInit")
			->FirstChildElement("Pallets")
			->FirstChildElement("Pallet")
			->FirstChildElement("Dimensions");

	// Bin
	bin.w = atoi(pallet_dimensions->FirstChildElement("Length")->GetText());
	bin.h = atoi(pallet_dimensions->FirstChildElement("Width")->GetText());
	bin.d = atoi(pallet_dimensions->FirstChildElement("MaxLoadHeight")->GetText());
	bin.n = 10;

	// Articles (Boxes)
	XMLNode	* orders = order_xml.FirstChildElement("Message")
			->FirstChildElement("Order")
			->FirstChildElement("OrderLines");
	XMLNode* ordern = orders->FirstChild();
	while (ordern != NULL) {
		XMLElement* article = ordern->FirstChildElement("Article");
		int id = atoi(article->FirstChildElement("ID")->GetText());
		int w = atoi(article->FirstChildElement("Length")->GetText());
		int h = atoi(article->FirstChildElement("Width")->GetText());
		int d = atoi(article->FirstChildElement("Height")->GetText());
		int weight = atoi(article->FirstChildElement("Weight")->GetText());
		int type = atoi(article->FirstChildElement("Type")->GetText());
		int family = atoi(article->FirstChildElement("Family")->GetText());

		int n = 0;
		vector<string> barcode_str;
		XMLNode* barcode = ordern->FirstChildElement("Barcodes")->FirstChildElement("Barcode");
		while (barcode != NULL) {
			n++;
			string str(barcode->ToElement()->GetText());
			barcode_str.push_back(str);
			barcode = barcode->NextSibling();
		}
		package_t p(id, w, h, d, n);
		p.set_description(article->FirstChildElement("Description")->GetText());
		p.barcode.assign(barcode_str.begin(), barcode_str.end());
		p.type = type;
		p.family = family;
		p.set_weight(weight);
		package_orig.push_back(p);
		ordern = ordern->NextSibling();
	}

	map<package_t, int, classcomp> package_map;
	map<package_t, int, classcomp>::iterator pmap_it;
	map<package_t, vector<int>, classcomp> package_map_vecid;
	map<package_t, vector<int>, classcomp>::iterator pmap_vecid_it;

	// Order
	for (int i = 0; i < package_orig.size(); i++) {
		package_t p = package_orig[i];
		pmap_it = package_map.find(p);
		pmap_vecid_it = package_map_vecid.find(p);
		if (pmap_it == package_map.end()) {
			package_map.insert(pair<package_t, int> (p, p.n));
			vector<int> vector_ids;
			for (int i = 0; i < p.n; i++) vector_ids.push_back(p.id);
			package_map_vecid.insert(pair<package_t, vector<int> > (p, vector_ids));
		}
		else {
			pmap_it->second += p.n;
			for (int i = 0; i < p.n; i++)
				(pmap_vecid_it->second).push_back(p.id);
		}
	}

	order_t o(package_map.size());

	int c = 0;
	for (pmap_it = package_map.begin(), pmap_vecid_it = package_map_vecid.begin();
			pmap_it != package_map.end();
			pmap_it++, pmap_vecid_it++) {
		package_t p = pmap_it->first;
		p.n = pmap_it->second;
		p.set_id((*pmap_vecid_it).second);
		package.push_back(p);
		o[c++] = p.n;
	}
	order = o;

	exporti();
}

void input::print() {
	cout << "Package List" << endl;
	print_package_list();
	cout << "Bin List" << endl;
	print_bin_list();
	cout << "Order" << endl;
	print_problem();
}

void input::exporti() {
	string problem_list, bin_list, package_list;
	package_list = dir + "/package_list.txt";
	bin_list = dir + "/bin_list.txt";
	problem_list = dir + "/problem_list.txt";

	ofstream ofs(package_list.c_str());
	for (int i = 0; i < package.size(); i++)
		ofs << package[i].id << " " << package[i].w << " " << package[i].h << " " << package[i].d << " " << package[i].n << " " << package[i].weight << endl;
	ofs.close();

	ofs.open(bin_list.c_str());
	ofs << bin.id << " " << bin.w << " " << bin.h << " " << bin.d << " " << bin.n << endl;
	ofs.close();

	ofs.open(problem_list.c_str());
	for (int i = 0; i < order.size(); i++)
		ofs << package[i].id << " " << order[i] << endl;
	ofs.close();
}
