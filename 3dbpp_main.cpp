#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include "3dbpp.h"
#include "3dbpp_settings.h"
#include "3dbpp_test.h"

using namespace std;

void generate_combo(const input &in, vector<int> &combo, int max, int id,
		bpp_settings* settings, database* db) {
	if (id == in.package_info.size()) { // create layer

		int w[MAXBOXES], h[MAXBOXES], d[MAXBOXES];
		int x[MAXBOXES], y[MAXBOXES], z[MAXBOXES], bno[MAXBOXES];
		int wt[MAXBOXES], id[MAXBOXES];
		int c = 0;
		for (int n = 0; n < in.package_info.size(); n++) {
			for (int j = 0; j < combo[n]; j++) {
				w[c] = in.package_info[n].w;
				h[c] = in.package_info[n].h;
				d[c] = in.package_info[n].d;
				x[c] = 0;
				y[c] = 0;
				z[c] = 0;
				bno[c] = 0;
				// no box should be bigger than bin size
				if (w[c] > 0 && h[c] > 0 && d[c] > 0 &&
						w[c] < in.bin_info[0].w	&&
						d[c] < in.bin_info[0].d &&
						h[c] < in.bin_info[0].h)
					c++;
			}
		}

		binpack3d(c, in.bin_info[0].w, in.bin_info[0].h, in.bin_info[0].d, w, h,
				d, x, y, z, bno, &settings->lb, &settings->ub,
				settings->nodelimit, settings->iterlimit, settings->timelimit,
				&settings->nodeused, &settings->iterused, &settings->timeused,
				settings->packingtype);

		key _key = combo;
		pattern _pattern;
		pattern::iterator _pattern_it;

		// todo
		// check if combo matches pattern with z = 0

		_pattern.insert(_pattern.end(), x, x+c);
		_pattern.insert(_pattern.end(), y, y+c);
		_pattern.insert(_pattern.end(), z, z+c);

		db->insert(_key, _pattern);

		return;
	} // create layer

	for (int i = 0; i <= max && i <= in.package_info[id].n; i++) {
		combo[id] = i;
		generate_combo(in, combo, max, id + 1, settings, db);
	}

}

database binpack(const input &in, bpp_settings* settings) {
	database d;

	int max = -1;
	vector<int> combo(in.package_info.size(), 0);
	for (int i = 0; i < in.package_info.size(); i++)
		if (max < in.package_info[i].n)
			max = in.package_info[i].n;
	generate_combo(in, combo, max, 0, settings, &d);

	return d;
}




int main(int argc, char *argv[]) {

	bpp_settings settings;
	settings.set_default();

	input i;
	i.load_package_list("package_list.txt");
	i.load_bin_list("bin_list.txt");
	i.print_package_list();
	i.print_bin_list();

	cout << "Done loading files" << endl;

	database d = binpack(i, &settings);
	d.exportdb("db.txt");

//  // Try problem
//  p.push_back(package(2, 2, 2, 8));
//  p.push_back(package(1, 1, 1, 8));
//  p_n.push_back(0);
//  p_n.push_back(0);
//
//  for (int i = 0; i < p[0].n; i++) {
//	  for (int j = 0; j < p[1].n; j++) {
//		  p_n[0] = i;
//		  p_n[1] = j;
//		  //binpack();
//	  }
//  }

	return 0;
}
