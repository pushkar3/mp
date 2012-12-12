#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <math.h>
#include "3dbpp.h"

using namespace std;

void help() {
	cout << "./viewer Output stats for the database folder" << endl;
	cout << "Usage: ./viewer <foldername>" << endl;
}

string colors[100] =
		{ "0x5CB73F", "0x341355", "0x1A6393", "0xB80114", "0xCFC5C3",
				"0x764154", "0x2345D8", "0xFFCEC3", "0xFF5DC7", "0x31E3B4",
				"0x3379E5", "0x6FBF18", "0x343B08", "0x9E7DF0", "0x543C48",
				"0x8130B2", "0x1D2FC1", "0xC49773", "0x273C70", "0x22DC5B",
				"0xDDA937", "0x5506C4", "0xB781CA", "0xAC42F9", "0x13F9A4",
				"0x279412", "0xD3FAF9", "0x5FEF91", "0x8A064D", "0x3E64BC",
				"0x452855", "0xC24D70", "0x0C78FE", "0x9E91CF", "0xC1E869",
				"0x83A085", "0xA5583C", "0x4A687C", "0xED0A69", "0xB5729F",
				"0xA2ADC4", "0xE38308", "0x3935F4", "0xD6E1A8", "0x581BE4",
				"0xC77A47", "0xD57643", "0xB85F6C", "0xF80CCA", "0xDF31EC",
				"0x737044", "0x28E8CB", "0x538770", "0xD51432", "0x6D57A1",
				"0x3FF123", "0xEFF7B0", "0x613946", "0x67BD9D", "0x916D70",
				"0x9C6A96", "0xD08F4E", "0x8E32E5", "0xBCB90C", "0xA72F2F",
				"0x346924", "0xDADC9F", "0x81B2A0", "0xECE7C1", "0x1182C5",
				"0x8DC93D", "0x7A5E23", "0x81EEB6", "0x3759CE", "0x4D2049",
				"0x11F9C4", "0x56B8EA", "0xAEFAC6", "0x4F8F10", "0xC26D96",
				"0xD001E9", "0x8827E0", "0x04DB9E", "0xAF81A4", "0x7F50E4",
				"0x999C35", "0x5BAAF8", "0x9405C3", "0xE126FF", "0xCCFCD0",
				"0xD9ECFE", "0xF15A85", "0x467500", "0x608DE7", "0x6F642E",
				"0x419552", "0x83744E", "0x27B82F", "0x5E1CC1", "0x35E7B7" };

map<int, string> id_color;
map<int, string>::iterator it;
int last_used_color = 0;
ofstream ofs;
ifstream ifs;

string get_color(int id) {
	it = id_color.find(id);
	if (it == id_color.end()) {
		id_color.insert(pair<int, string> (id, colors[last_used_color]));
		last_used_color = (last_used_color+1)%100;
	}
	return id_color[id];
}

void print(int w, int h, int d, int x, int y, int z, string color) {
	ofs << "\t\tdrawCube(" << w << ", " << h << ", " << d << ", " << x << ", " << y << ", " << z << ", " << color << ");" << endl;
}

int main(int argc, char *argv[]) {
	int verbose = 0;

	if (argc < 2) {
		help();
		return 0;
	}

	string dir(argv[argc-1]);
	input i;
	output o;
	database d;

	i.load_xml(dir.c_str());
	d.get_input(i);
	d.importdb();

	o.set_database(&d);
	o.importpl();

	string op_file(dir + "/index.html");
	ofs.open(op_file.c_str());

	ifs.open("ext/three_header.html");
	while (ifs.good()) {
		char c = ifs.get();
		if (ifs.good())
			ofs << c;
	}
	ifs.close();


	int w_before = -(d.bin.w)*(o.packlist_vector.size()/2.0);
	for (uint i = 0; i < o.packlist_vector.size() - 1; i++) {
		packlist_ p = o.packlist_vector[i];
		print(d.bin.w, d.bin.h, 15, d.bin.w/2.0+w_before, d.bin.h/2.0, -15, "0x000000");

		for (uint j = 0; j < p.size(); j++) {
			config_t c = p[j];
			vector<int> ori = c.get_orientation();
			int c1 = 0;
			for (uint a = 0; a < c.get_key().size(); a++) {
				for (int b = 0; b < c.get_key()[a]; b++) {
					int id = d.package[a].get_id();
					int w = d.package[a].w;
					int h = d.package[a].h;
					int _d = d.package[a].d;
					int x = c.get_pattern()[c1*3+0];
					int y = c.get_pattern()[c1*3+1];
					int z = c.get_pattern()[c1*3+2];
					if(ori[c1] == 1) {
						int temp = w;
						w = h;
						h = temp;
					}
					x += w/2.0;
					y += h/2.0;
					z += _d/2.0;
					print(w, h, _d, x+w_before, y, z, get_color(id));
					c1++;
				}
			}
		}
		w_before += d.bin.w;
		w_before += 100;
	}

	ifs.open("ext/three_footer.html");
	while (ifs.good()) {
		char c = ifs.get();
		if (ifs.good())
			ofs << c;
	}
	ifs.close();

	ofs.close();
	return 0;
}
