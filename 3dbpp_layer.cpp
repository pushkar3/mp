#include <stdio.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include "3dbpp.h"

int debug = 0;
using namespace std;

// ---------------------------------------------------------------------------------
// Global data types
typedef struct {
	int x, y, z, w, h, d, bno;
	int wt, id;
} box_t;

typedef struct {
	vector<box_t> pattern;
	int box_type;
} layer_t;

typedef struct {
	vector<box_t> box;
	vector<int> n_box;
} bpp_t;

typedef struct {
	int x, y, z;
	int off_x, off_y, off_z;
	double bpp_part[MAXBOXES];
} surface_t;

typedef struct{
	int d;
	vector<int> box_types;
} hash_t;

vector<layer_t> layer;
vector<surface_t> surface;
vector<hash_t> hash;

// ---------------------------------------------------------------------------------
// Global variables
bpp_t bpp;

// ---------------------------------------------------------------------------------
// Helper function
int is_near(int a, int b, int near) {
	if(abs(b - a) < near)
		return 1;
	return 0;
}

void box_debug(box_t b) {
	printf("\n%d %d %d \t %d %d %d", b.w, b.h, b.d, b.x, b.y, b.z);
}

void bpp_debug(int n, int W, int H, int D) {
	printf("\nProblem is %d: %d, %d, %d", n, W, H, D);
	for(int i = 0; i < bpp.box.size(); i++) {
		box_debug(bpp.box[i]);
		printf("\t%d", bpp.n_box[i]);
	}
	printf("\n\n");
}

void layer_debug() {
	printf("\nLayers are\n");
	for(int i = 0; i < layer.size(); i++) {
		printf("\n\nLayer %d is made of %d (%d)", i, layer[i].box_type, layer[i].pattern.size());

		printf("\nPattern:\n");

		for(int j = 0; j < layer[i].pattern.size(); j++) {
			box_debug(layer[i].pattern[j]);
		}
	}
}

void surface_debug() {
	for(int i = 0; i < surface.size(); i++) {
		printf("%d. %d %d %d %d %d %d\n", i, surface[i].x, surface[i].y, surface[i].z, surface[i].off_x, surface[i].off_y, surface[i].off_z);
	}
}

// Algorithm helpers
int bpp_relaxed(bpp_t sol, surface_t* s) {
	int ret = 1;
	for (int i = 0; i < bpp.box.size(); i++) {
		int w = bpp.box[i].w;
		int h = bpp.box[i].h;

		int nw = s->x / w;
		int nh = s->y / h;
		int n_boxes = nw * nh;

		double n_sol_boxes = sol.n_box[i];
		double part = 0.0f;
		if(n_boxes > 0) part = (double) (n_sol_boxes / n_boxes);
		if (part < 0.0f) part = 0.0f;
		s->bpp_part[i] = part;
		if (part > 1.0) ret *= 0;
		//printf("\n %d [%d] \t %2.2lf (%d left)", i, bpp.box[i].d, s->bpp_part[i], sol.n_box[i]);
	}
	return ret;
}

int bpp_relaxed_max(surface_t s, double* val) {
	int ret = -1;
	double v = 0.0f;
	for (int i = 0; i < bpp.box.size(); i++) {
		if(s.bpp_part[i] > v) {
			v = s.bpp_part[i];
			ret = i;
		}
	}
	*val = v;
	return ret;
}

// ---------------------------------------------------------------------------------
// Main Algorithm

layer_t bpp_create_layer(int W, int H, int D, int box_type, int off_x, int off_y, int off_z) {
	layer_t l;
	int w = bpp.box[box_type].w;
	int h = bpp.box[box_type].h;
	int d = bpp.box[box_type].d;

	int n_w = W/w; // Boxes that fit along w, h
	int n_h = H/h;
	double g_w = (double)((W-n_w*w)/n_w); // Gap that needs to be filled by each box
	double g_h = (double)((H-n_h*h)/n_h);

	int x1[MAXBOXES];
	int y1[MAXBOXES];
	int z1[MAXBOXES];
	int w1[MAXBOXES];
	int h1[MAXBOXES];
	int d1[MAXBOXES];
	int bno1[MAXBOXES];
	int lb1 = 0; int ub1 = 0;
	int nodeused1 = 0;
	int iterused1 = 0;
	int timeused1 = 0;
	int nodelimit1 = 5;
	for(int i = 0; i < 50; i++) {
		x1[i] = y1[i] = z1[i] = 0;
		w1[i] = w; h1[i] = h; d1[i] = d;
		bno1[i] = 1;
	}
	binpack3d(50, W, H, D, w1, h1, d1, x1, y1, z1, bno1, &lb1, &ub1, 0, nodelimit1, 0, &nodeused1, &iterused1, &timeused1, 1);

	l.box_type = box_type;
	for(int i = 0; i < 50; i++) {
		if(z1[i] == 0 && bno1[i] == 1) {
			box_t b;
			b.w = bpp.box[box_type].w;
			b.h = bpp.box[box_type].h;
			b.d = bpp.box[box_type].d;
			b.x = x1[i] + /*g_w/2 +*/ off_x;
			b.y = y1[i] + /*g_h/2 +*/ off_y;
			b.z = off_z;
			b.bno = 1;
			l.pattern.push_back(b);
		}
	}
	return l;
}

void bpp_add_layer(bpp_t* sol, vector<layer_t>* sol_layer, surface_t* surface, int box_type) {
	surface_t s = *surface;
	layer_t l = bpp_create_layer(s.x, s.y, s.z, box_type, s.off_x, s.off_y, s.off_z);
	sol->n_box[box_type] -= l.pattern.size();
	sol_layer->push_back(l);
}

void sort_sol_range(int i_start, int i_end, int *a, int *b, double *c, int use_h_to_sort) {
	if (a == NULL) {
		printf("Error in %d %s\n", __LINE__, __func__);
		return;
	}

	for (int i = i_start; i < i_end; i++) {
		for (int j = i; j < i_end; j++) {
			if (a[i] > a[j] && use_h_to_sort) {
				exchange(a, i, j);
				exchange(b, i, j);
				exchange_double(c, i, j);
			}
			if (c[i] < c[j] && !use_h_to_sort) {
				exchange(a, i, j);
				exchange(b, i, j);
				exchange_double(c, i, j);
			}
		}
	}
}


int binpack3d_layer(int n, int W, int H, int D, int *w, int *h, int *d, int *x,
		int *y, int *z, int* wt, int* id, int *bno, int *lb, int *ub, int nodelimit,
		int iterlimit, int timelimit, int *nodeused, int *iterused,
		int *timeused, int packingtype) {


	box_t b;
	b.w = w[0]; b.h = h[0]; b.d = d[0];
	b.x = b.y = b.z = b.bno = 0;
	b.wt = wt[0];
	b.id = id[0];
	bpp.box.push_back(b);
	bpp.n_box.push_back(1);

	hash_t ht;
	ht.d = d[0];
	ht.box_types.push_back(0);
	hash.push_back(ht);

	for(int i = 1; i < n; i++) {
		if((w[i] != w[i-1]) || (h[i] != h[i-1]) || (d[i] != d[i-1])) {
			b.w = w[i]; b.h = h[i]; b.d = d[i];
			b.x = b.y = b.z = b.bno = 0;
			b.wt = wt[i];
			b.id = id[i];
			bpp.box.push_back(b);
			bpp.n_box.push_back(1);
		}
		else {
			bpp.n_box[bpp.n_box.size()-1]++;
		}
	}

	for(int i = 1; i < bpp.box.size(); i++) {
		if(bpp.box[i].d == hash.back().d) hash.back().box_types.push_back(i);
		else {
			hash_t ht;
			ht.d = bpp.box[i].d;
			ht.box_types.push_back(i);
			hash.push_back(ht);
		}
	}

	bpp_debug(n, W, H, D);
	//-------------------
    // Make all changes below this only --------------------------------------------------------

	// Try to solve the problem
	bpp_t sol = bpp;
	vector<layer_t> sol_layer;
	int sol_bt[MAXBOXES];
	double sol_bt_bpp[MAXBOXES];
	int sol_bt_h[MAXBOXES];
	int solved = 0; // reverse

	// The whole surface is open to palletizing
	surface_t s;
	s.x = W; s.y = H; s.z = D;
	s.off_x = 0; s.off_y = 0; s.off_z = 0;
	surface.push_back(s);

	// Solves for all full layers
	while (!solved) {
		// Calculate parts
		solved = bpp_relaxed(sol, &surface.back());

		// Add surfaces
		for (int j = 0; j < bpp.box.size(); j++) {
			if (surface.back().bpp_part[j] >= 0.85) { // Will always have 1 layer
				bpp_add_layer(&sol, &sol_layer, &surface.back(), j);
				surface.back().off_z += bpp.box[j].d;
			}
		}

		printf("\n");
	}

	bpp_relaxed(sol, &surface.back());

	// Now solve for partial layers, give precedence to same height

	printf("\n");
	for(int i = 0; i < hash.size(); i++) {

		double bpp_sum = 0.0f;

		for(int j = 0; j < hash[i].box_types.size(); j++) {
			if(surface.back().bpp_part[hash[i].box_types[j]] <= 0.0f)
				hash[i].box_types.erase(hash[i].box_types.begin()+j);
			bpp_sum += surface.back().bpp_part[hash[i].box_types[j]];
		}

		if(bpp_sum >= 0.85) {
			for(int j = 0; j < hash[i].box_types.size(); j++) {
				double h_part = surface.back().bpp_part[hash[i].box_types[j]];
				surface.back().y = h_part*H;
				bpp_add_layer(&sol, &sol_layer, &surface.back(), hash[i].box_types[j]);
				surface.back().off_y += h_part*H;
			}
			surface.back().y = H;
			surface.back().off_y = 0.0f;
			surface.back().off_z += hash[i].d;
		}

		printf("\n");
		bpp_relaxed(sol, &surface.back());
	}

	printf("\n\nMultiple Layers\n");
	for(int k = 0; k < surface.size(); k++) {
		printf("\nLayer %d/%d\n", k, surface.size()-1);
		bpp_relaxed(sol, &surface[k]);
	}

	solved = 0;
	printf("\n");
	while(!solved) {
		double max_bpp = 0.0f;
		int box_type = -1;
		int best_surface = 0;
		for(int k = 0; k < surface.size(); k++) {
			//printf("\n");
			for (int i = 0; i < hash.size(); i++) {

				//printf("\nHash: h = %d ", hash[i].d);
				for (int j = 0; j < hash[i].box_types.size(); j++) {
					/*if (surface[k].bpp_part[hash[i].box_types[j]] <= 0.0f)
						hash[i].box_types.erase(hash[i].box_types.begin() + j);
						*/
					// TODO: Not required? Why?
					//printf(" %d ", hash[i].box_types[j]);

					if (max_bpp < surface[k].bpp_part[hash[i].box_types[j]]) {
						max_bpp = surface[k].bpp_part[hash[i].box_types[j]];
						box_type = hash[i].box_types[j];
						best_surface = k;
					}
				}
			}
		}

		if(box_type == -1) {
			printf("\nEnd\n");
			solved = 1;
			break;
		}

		surface_t s = surface[best_surface];
		if(max_bpp > 1.0) max_bpp = 1.0;
		if(max_bpp*s.x >= bpp.box[box_type].w && s.y >= bpp.box[box_type].h) {
			s.x = max_bpp*s.x;
			surface[best_surface].x += s.x;
			surface[best_surface].off_x += s.x;
			surface.push_back(s);
		}
		else if(max_bpp*s.y >= bpp.box[box_type].h && s.x >= bpp.box[box_type].w) {
			s.y = max_bpp*s.y;
			surface[best_surface].y += s.y;
			surface[best_surface].off_y += s.y;
			surface.push_back(s);
		}

		if(surface.back().off_z + bpp.box[box_type].d > D) {
			printf("\n\nBin full\n");
			box_type = -1;
			break;
		}

		bpp_add_layer(&sol, &sol_layer, &surface.back(), box_type);
		printf("\nTrying to add box %d on layer %d\n", box_type, best_surface);
		surface_debug();
		surface.back().off_z += bpp.box[box_type].d;

		for(int k = 0; k < surface.size(); k++) {
			printf("\n\nLayer %d", k);
			bpp_relaxed(sol, &surface[k]);
		}
	}

	// After this I have to maintain many surfaces


	// Done. Convert this to array format
	n = 0;
	for(int i = 0; i < sol_layer.size(); i++) {
		for(int j = 0; j < sol_layer[i].pattern.size(); j++) {
			x[n] = sol_layer[i].pattern[j].x;
			y[n] = sol_layer[i].pattern[j].y;
			z[n] = sol_layer[i].pattern[j].z;
			w[n] = sol_layer[i].pattern[j].w;
			h[n] = sol_layer[i].pattern[j].h;
			d[n] = sol_layer[i].pattern[j].d;
			bno[n] = 1;
			wt[n] = bpp.box[sol_layer[i].box_type].wt;
			id[n] = bpp.box[sol_layer[i].box_type].id;
			if(z[n] + w[n] > D) break;
 			n++;
		}
	}

	printf("\n\nCreated a pallet of %d boxes. Done", n);
	printf("\n");
	return n;
}

