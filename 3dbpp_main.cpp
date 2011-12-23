#include <iostream>
#include <stdio.h>
#include <vector>
#include "3dbpp.h"
#include "3dbpp_settings.h"
#include "3dbpp_test.h"

using namespace std;


vector<package> p;
vector<bin> b;
vector<int> p_n;

//void binpack() {
//	int c = 0;
//	for (int n = 0; n < p_n.size(); n++) {
//		for (int i = 0; i < p_n[n]; i++) {
//			w[c] = p[n].w;
//			h[c] = p[n].h;
//			d[c] = p[n].d;
//			c++;
//		}
//	}
//
//	N = c;
//    settings.show_start();
//	binpack3d(N, W, H, D, w, h, d, x, y, z, bno, &settings.lb, &settings.ub,
//			settings.nodelimit, settings.iterlimit, settings.timelimit,
//			&settings.nodeused, &settings.iterused, &settings.timeused,
//			settings.packingtype);
//	settings.show_end();
//
//	printboxes(N, W, H, D, w, h, d, x, y, z, wt, id, bno);
//}

void generate_combo(const input &i, vector<int> &combo, int max, int id) {
	if(id == i.package_info.size()) {
		//shit!
		return;
	}

	for(int i = 0; i <= max && i <= i.package_info[id].n; i++) {
		combo[id] = i;
		generate_combo(i, combo, max, id+1);
	}

}

database binpack(const input &i, const bpp_settings &settings) {
	database d;

	int max = -1;
	vector<int> combo;
	generate_combo(i, combo, max, id)

	// gen combo


	// run this for every combo
//	int w[MAXBOXES], h[MAXBOXES], d[MAXBOXES];
//	int x[MAXBOXES], y[MAXBOXES], z[MAXBOXES], bno[MAXBOXES];
//	int wt[MAXBOXES], id[MAXBOXES];
//	int c = 0;
//	for (int n = 0; n < i.package_info.size(); n++) {
//		for (int j = 0; j < i.package_info().n; n++) {
//			//w[c] =
//		}
//	}

//	binpack3d(N, W, H, D, w, h, d, x, y, z, bno, &settings.lb, &settings.ub,
//			settings.nodelimit, settings.iterlimit, settings.timelimit,
//			&settings.nodeused, &settings.iterused, &settings.timeused,
//			settings.packingtype);

	return d;
}

int main(int argc, char *argv[])
{
  bpp_settings settings;
  settings.set_default();

  input i;
  i.load_package_list("package_list.txt");
  i.load_bin_list("bin_list.txt");
  i.print_package_list();
  i.print_bin_list();

  database d = binpack(i, settings);

//
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
