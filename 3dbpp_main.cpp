#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include "3dbpp.h"
#include "3dbpp_settings.h"
#include "3dbpp_test.h"
#include "palletizing.h"

using namespace std;

void generate_combo(vector<int> &combo, int max, int id,
		bpp_settings* settings, database* db) {
	if (id == db->package_info.size()) { // create layer

		int w[MAXBOXES], h[MAXBOXES], d[MAXBOXES];
		int x[MAXBOXES], y[MAXBOXES], z[MAXBOXES], bno[MAXBOXES];
		int wt[MAXBOXES], id[MAXBOXES];
		int c = 0;
		for (int n = 0; n < db->package_info.size(); n++) {
			for (int j = 0; j < combo[n]; j++) {
				w[c] = db->package_info[n].w;
				h[c] = db->package_info[n].h;
				d[c] = db->package_info[n].d;
				x[c] = 0;
				y[c] = 0;
				z[c] = 0;
				bno[c] = 0;
				// no box should be bigger than bin size
				if (w[c] > 0 && h[c] > 0 && d[c] > 0 &&
						w[c] < db->bin_info[0].w	&&
						d[c] < db->bin_info[0].d &&
						h[c] < db->bin_info[0].h)
					c++;
			}
		}

		binpack3d(c, db->bin_info[0].w, db->bin_info[0].h, db->bin_info[0].d, w, h,
				d, x, y, z, bno, &settings->lb, &settings->ub,
				settings->nodelimit, settings->iterlimit, settings->timelimit,
				&settings->nodeused, &settings->iterused, &settings->timeused,
				settings->packingtype);

		if (c == 0) return;

		key _key = combo;
		pattern _pattern;
		int _cost = 0;
		pattern::iterator _pattern_it;

		// todo
		// check if combo matches pattern with z = 0
		for (int i = 0; i < db->package_info.size(); i++)
			_key[i] = 0;

		for (int i = 0; i < c; i++) {
			if(z[i] > 0) continue;
			for (int j = 0; j < db->package_info.size(); j++) {
				if(w[i] == db->package_info[j].w
						&& h[i] == db->package_info[j].h
						&& d[i] == db->package_info[j].d) {
					_key[j]++;
				}
			}
			_pattern.push_back(x[i]);
			_pattern.push_back(y[i]);
			_pattern.push_back(z[i]);

			_cost += (w[i]*h[i]*d[i]);
		}

		db->insert(_key, _pattern, _cost);


		return;
	} // create layer

	for (int i = 0; i <= max && i <= db->package_info[id].n; i++) {
		combo[id] = i;
		generate_combo(combo, max, id + 1, settings, db);
	}

}

void binpack(bpp_settings* settings, database *d) {
	int max = -1;
	vector<int> combo(d->package_info.size(), 0);
	for (int i = 0; i < d->package_info.size(); i++)
		if (max < d->package_info[i].n)
			max = d->package_info[i].n;
	generate_combo(combo, max, 0, settings, d);
}




int main(int argc, char *argv[]) {

	bpp_settings settings;
	settings.set_default();

	input i;
	output o;
	database d;
	i.load("data");
	i.print();

	d.get_input(i);
	if(!d.importdb("data")) {
		binpack(&settings, &d);
		d.exportdb();
	}

	d.printdb();

	palletizing p(&d);
	p.solve();

	o.exportl(d, "data");

	return 0;
}
