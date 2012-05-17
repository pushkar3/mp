#include <iostream>
#include <sstream>
#include <tinyxml2.h>
#include "3dbpp.h"

using namespace std;
using namespace tinyxml2;

bpp_param param;

bpp_param::bpp_param() {
}

bpp_param::~bpp_param() {
}

int bpp_param::load(const char* dir) {
	string filename(dir);
	filename += "/params.xml";
	doc.LoadFile(filename.c_str());
	params = doc.FirstChildElement("params");
}

double bpp_param::get(const char* name) {
	return atof(params->FirstChildElement(name)->GetText());
}


config_t::config_t() {
	reset();
}

config_t::~config_t() {

}

void config_t::reset() {
	key.clear();
	pattern.clear();
	orientation.clear();
	n_packs = 0;
	n_area = 0;
	n_maxh = 0;
	n_maxw = 0;
	n_maxd = 0;
	n_tvolume = 0;
	n_density = 0.0f;
	n_packvolume = 0;
	n_packweight = 0;
	origin.assign(3, 0);
	corner1.assign(3, 0);
	corner2.assign(3, 0);
	corner3.assign(3, 0);
	gap_w = 0;
	gap_h = 0;
}

void config_t::set(database* db, key_ k, pattern_ p, vector<int> o) {
	d = db;
	key = k;
	pattern = p;
	orientation = o;

	int c = 0;
	for (uint i = 0; i < key.size(); i++) {
		for (int j = 0; j < key[i]; j++) {
			int pack_w = 0, pack_h = 0;
			if(o[c] == 1) {
				pack_w = d->package[i].h;
				pack_h = d->package[i].w;
			}
			else {
				pack_w = d->package[i].w;
				pack_h = d->package[i].h;
			}
			int xn = pattern[c*3 + 0] + pack_w;
			int yn = pattern[c*3 + 1] + pack_h;
			int zn = pattern[c*3 + 2] + d->package[i].d;
			if (n_maxw < xn)	n_maxw = xn;
			if (n_maxh < yn)	n_maxh = yn;
			if (n_maxd < zn)	n_maxd = zn;
			n_packvolume += db->package[i].volume();
			n_packweight += db->package[i].get_weight();
			c++;
		}
	}

	n_packs = c;
	n_tvolume = n_maxw * n_maxh * n_maxd;
	if (n_tvolume != 0)
		n_density = (double) ((double)(n_packvolume) / (double)(n_tvolume));
	n_area = n_maxw * n_maxh;

	corner1[0] = n_maxw;
	corner2[1] = n_maxh;
	corner3[2] = n_maxd;
}

vector<int> config_t::get_origin() {
	return origin;
}

void config_t::set_origin(vector<int> o) {
	origin = o;

	for (uint i = 0; i < pattern.size(); i++) {
		pattern[i] += origin[i%3];
	}
}

bool config_t::operator == (const config_t &c) {
	if (c.key == key && n_maxw == c.n_maxw && n_maxh == c.n_maxh && n_maxd == c.n_maxd)
		return true;
	return false;
}

bool config_t::is_bound() {
	return (n_maxw <= d->bin.w && n_maxh <= d->bin.h && n_maxd <= d->bin.d);
}

bool config_t::is_layer() {
	int area = d->bin.w * d->bin.h;
	return(((double)(area - get_area())/(double)area) < param.get("layer_density_threshold"));
}

ostream & operator << (ostream &o, const config_t &c) {
	vector<int> key = c.key;
	vector<int> pattern = c.pattern;
	o << "Layer " << endl;
	for (uint i = 0; i < key.size(); i++) o << key[i] << " ";
	o << endl;
	for (uint i = 0; i < pattern.size(); i++) o << pattern[i] << " ";
	o << endl;
	o << "Total packages: " << c.n_packs << endl;
	o << "Total packvolume: " << c.n_packvolume << endl;
	o << "Total volume: " << c.n_tvolume << endl;
	o << "Total area: " << c.n_area << endl;
	o << "Max height: " << c.n_maxd << endl;
	o << "Density: " << c.n_density << endl;
	o << endl;
	return o;
}

void config_t::add(const config_t c) {
	vector<int> p;
	vector<int> o;
	vector<int> k(key.size(), 0);
	int c1 = 0, c2 = 0;

	for (uint i = 0; i < key.size(); i++) {
		for (int j = 0; j < key[i]; j++) {
			p.push_back(pattern[c1*3 + 0]);
			p.push_back(pattern[c1*3 + 1]);
			p.push_back(pattern[c1*3 + 2]);
			o.push_back(orientation[c1]);
			c1++;
		}

		for (int j = 0; j < c.key[i]; j++) {
			p.push_back(c.pattern[c2*3 + 0]);
			p.push_back(c.pattern[c2*3 + 1]);
			p.push_back(c.pattern[c2*3 + 2]);
			o.push_back(c.orientation[c2]);
			c2++;
		}
		k[i] = key[i] + c.key[i];
	}

	reset();
	set(this->d, k, p, o);
}

void config_t::add(database* db, const config_t c) {
	d = db;
	add(c);
}

int config_t::get_height() {
	return n_maxd;
}

int config_t::get_area() {
	return n_area;
}

vector<int> config_t::get_corner(int i) {
	switch(i) {
	case 0: return corner1;
	case 1: return corner2;
	case 2: return corner3;
	default: return corner3;
	}
}

key_ config_t::get_key() {
	return key;
}

pattern_ config_t::get_pattern() {
	return pattern;
}

vector<int> config_t::get_orientation() {
	return orientation;
}

dimensions_ config_t::get_dimensions() {
	dimensions_ dims;
	dims.push_back(n_maxw);
	dims.push_back(n_maxh);
	dims.push_back(n_maxd);
	return dims;
}

int config_t::get_weight() {
	return n_packweight;
}

int config_t::get_packagevolume() {
	return n_packvolume;
}

int config_t::get_totalvolume() {
	return n_tvolume;
}

int config_t::get_totalpacks() {
	return n_packs;
}

void config_t::change_orientation() {
	int c1 = 0;
	vector<int> p(pattern.size(), 0);
	vector<int> o(orientation.size(), 0);
	vector<int> k(key.size(), 0);

	for (uint i = 0; i < key.size(); i++) {
		k[i] = key[i];
		for (int j = 0; j < key[i]; j++) {
			p[c1*3 + 1] = pattern[c1*3 + 0];
			p[c1*3 + 0] = pattern[c1*3 + 1];
			p[c1*3 + 2] = pattern[c1*3 + 2];
			if(orientation[c1] == 0) o[c1] = 1;
			else if(orientation[c1] == 1) o[c1] = 0;
			c1++;
		}
	}

	reset();
	set(this->d, k, p, o);
}

void config_t::change_orientation(database* db) {
	d = db;
	change_orientation();
}

double config_t::density() {
	return n_density;
}

string config_t::key_s() {
	stringstream s(stringstream::out);
	for (uint i = 0; i < key.size(); i++) {

		s << key[i] << " ";
	}
	return s.str();
}

string config_t::pattern_s() {
	stringstream s(stringstream::out);
	for (uint i = 0; i < pattern.size(); i++) {
		s << pattern[i] << " ";
	}
	return s.str();
}

string config_t::orientation_s() {
	stringstream s(stringstream::out);
	for (uint i = 0; i < orientation.size(); i++) {
		s << orientation[i] << " ";
	}
	return s.str();
}


string config_t::dimensions_s() {
	stringstream s(stringstream::out);
	s << n_maxw << " " << n_maxh << " " << n_maxd << " ";
	return s.str();
}

string config_t::cost_s() {
	stringstream s(stringstream::out);
	// s << n_packweight + n_area + n_density;
	s << n_packweight;
	return s.str();
}

void config_t::set_gap(int w, int h) {
	gap_w = w;
	gap_h = h;
}

void config_t::spread_out(bin_t bin) {
	double sw = 0.0f, sh = 0.0f;

	sw = (float)bin.w/n_maxw;
	sh = (float)bin.h/n_maxh;

	int c = 0;
	for (uint i = 0; i < key.size(); i++) {
		for (int j = 0; j < key[i]; j++) {

			int pack_w = 0, pack_h = 0;
			if(orientation[c] == 1) {
				pack_w = d->package[i].h;
				pack_h = d->package[i].w;
			}
			else {
				pack_w = d->package[i].w;
				pack_h = d->package[i].h;
			}

			pattern[c*3+0] = (pattern[c*3+0]-pattern[0])*sw;
			pattern[c*3+1] = (pattern[c*3+1]-pattern[1])*sh;

			// new max calc's
			int xn = pattern[c*3 + 0] + pack_w;
			int yn = pattern[c*3 + 1] + pack_h;
			int zn = pattern[c*3 + 2] + d->package[i].d;
			if (n_maxw < xn)	n_maxw = xn;
			if (n_maxh < yn)	n_maxh = yn;
			if (n_maxd < zn)	n_maxd = zn;
			c++;
		}
	}
}


void config_t::center_in(bin_t bin) {
	double sw = 0.0f, sh = 0.0f;

	int dw = n_maxw - bin.w;
	int dh = n_maxh - bin.h;

	sw = (float) dw/2.0f;
	sh = (float) dh/2.0f;

	int c = 0;
	for (uint i = 0; i < key.size(); i++) {
		for (int j = 0; j < key[i]; j++) {

			int pack_w = 0, pack_h = 0;
			if(orientation[c] == 1) {
				pack_w = d->package[i].h;
				pack_h = d->package[i].w;
			}
			else {
				pack_w = d->package[i].w;
				pack_h = d->package[i].h;
			}

			pattern[c*3+0] -= sw;
			pattern[c*3+1] -= sh;

			// new max calc's
			int xn = pattern[c*3 + 0] + pack_w;
			int yn = pattern[c*3 + 1] + pack_h;
			int zn = pattern[c*3 + 2] + d->package[i].d;
			if (n_maxw < xn)	n_maxw = xn;
			if (n_maxh < yn)	n_maxh = yn;
			if (n_maxd < zn)	n_maxd = zn;
			c++;
		}
	}
}

void config_t::rotate() {
	int c = 0;
	for (uint i = 0; i < key.size(); i++) {
		for (int j = 0; j < key[i]; j++) {
			int pack_w = 0, pack_h = 0;
			if(orientation[c] == 1) {

				pack_w = d->package[i].h;
				pack_h = d->package[i].w;
			}
			else {
				pack_w = d->package[i].w;
				pack_h = d->package[i].h;
			}

			pattern[c*3+0] = n_maxw - pattern[c*3+0] - pack_w;
			pattern[c*3+1] = n_maxh - pattern[c*3+1] - pack_h;
			c++;
		}
	}
}

int config_t::compare(config_t c) {
	int ret = 0;
	if(key.size() != c.key.size()) return -1;
	for (int i = 0; i < c.key.size(); i++) {
		if(key[i] != c.key[i]) ret = -1;
	}
	return ret;
}
