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
} box_t;

typedef struct {
	vector<box_t> pattern;
	vector<int> box;
	vector<int> n_box;
} layer_t;

typedef struct {
	vector<box_t> box;
	vector<int> n_box;
} bpp_t;

vector<layer_t> layer;

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
		printf("\n\nLayer %d is made of ", i);
		for(int j = 0; j < layer[i].box.size(); j++) {
			printf("(%d, %d) ", layer[i].box[j], layer[i].n_box[j]);
		}

		printf("\nPattern:\n");

		for(int j = 0; j < layer[i].pattern.size(); j++) {
			box_debug(layer[i].pattern[j]);
		}
	}
}

// Algorithm helpers
int sol_value(bpp_t sol) {
	int ret = 0;
	for(int i = 0; i < sol.box.size(); i++) {
		ret += sol.n_box[i];
	}
	return ret;
}

int sol_use_layer(bpp_t* sol, layer_t layer) {
	bpp_t _sol = *sol;
	int init_value = sol_value(_sol);
	for(int i = 0; i < layer.box.size(); i++) {
		_sol.n_box[layer.box[i]] -= layer.n_box[i];
		if(_sol.n_box[layer.box[i]] < 0)
			_sol.n_box[layer.box[i]] = 0;
	}
	int final_value = sol_value(_sol);
	if(final_value < init_value) {
		for(int i = 0; i < layer.box.size(); i++) {
			sol->n_box[layer.box[i]] -= layer.n_box[i];
			if(sol->n_box[layer.box[i]] < 0)
				sol->n_box[layer.box[i]] = 0;
		}
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------------
// Main Algorithm

layer_t create_layer(int W, int H, int D, int box_type, int off_x, int off_y, int off_z) {
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
		w1[i] = w+g_w; h1[i] = h+g_h; d1[i] = d;
		bno1[i] = 1;
	}
	binpack3d(50, W, H, D, w1, h1, d1, x1, y1, z1, bno1, &lb1, &ub1, 0, nodelimit1, 0, &nodeused1, &iterused1, &timeused1, 1);

	l.box.push_back(box_type);
	for(int i = 0; i < 50; i++) {
		if(z1[i] == 0 && bno1[i] == 1) {
			box_t b;
			b.w = bpp.box[box_type].w;
			b.h = bpp.box[box_type].h;
			b.d = bpp.box[box_type].d;
			b.x = x1[i] + g_w/2 + off_x;
			b.y = y1[i] + g_h/2 + off_y;
			b.z = off_z;
			b.bno = 1;
			l.pattern.push_back(b);
		}
	}
	l.n_box.push_back(l.pattern.size());
	return l;
}

int binpack3d_layer(int n, int W, int H, int D, int *w, int *h, int *d, int *x,
		int *y, int *z, int *bno, int *lb, int *ub, int nodelimit,
		int iterlimit, int timelimit, int *nodeused, int *iterused,
		int *timeused, int packingtype) {


	box_t b;
	b.w = w[0]; b.h = h[0]; b.d = d[0];
	b.x = b.y = b.z = b.bno = 0;
	bpp.box.push_back(b);
	bpp.n_box.push_back(1);

	for(int i = 1; i < n; i++) {
		if((w[i] != w[i-1]) || (h[i] != h[i-1]) || (d[i] != d[i-1])) {
			b.w = w[i]; b.h = h[i]; b.d = d[i];
			b.x = b.y = b.z = b.bno = 0;
			bpp.box.push_back(b);
			bpp.n_box.push_back(1);
		}
		else {
			bpp.n_box[bpp.n_box.size()-1]++;
		}
	}

	bpp_debug(n, W, H, D);

	//-------------------
	// Layers made of box of one type alone
	for (int i = 0; i < bpp.box.size(); i++) {
		printf("Creating layer for box %d\n", i);
		layer_t l = create_layer(W, H, D, i, 0, 0, 0);
		layer.push_back(l);
	}


	// layer_debug();
	//-------------------
    // Make all changes below this only --------------------------------------------------------

	// Try to solve the problem
	bpp_t sol = bpp;
	vector<layer_t> sol_layer;
	int solved = 1; // reverse

	if(debug > 0)
		printf("Trying to create pallet\n");

	while(solved != 0) {
		solved = 0;
		for(int i = 0; i < layer.size(); i++) {
			int ret = sol_use_layer(&sol, layer[i]);
			if(ret == 1) {
				printf("\nBox %d -> %d instead of %d ", layer[i].box[0], layer[i].pattern.size(), bpp.n_box[layer[i].box[0]]);
				sol_layer.push_back(layer[i]);
				solved++;
			}
		}
	}

	if(debug > 0) {
		printf("Solved");
		printf("\n\n");
	}

	for(int i = 0; i < MAXBOXES; i++) {
		x[i] = y[i] = z[i] = w[i] = h[i] = d[i] = 0.0f;
	}

	n = 0;
	int _h = 0;
	int _h_layer = 0;
	for(int i = 0; i < sol_layer.size(); i++) {
		_h = 0;
		for(int j = 0; j < sol_layer[i].pattern.size(); j++) {
			x[n] = sol_layer[i].pattern[j].x;
			y[n] = sol_layer[i].pattern[j].y;
			z[n] = sol_layer[i].pattern[j].z + _h_layer;
			w[n] = sol_layer[i].pattern[j].w;
			h[n] = sol_layer[i].pattern[j].h;
			d[n] = sol_layer[i].pattern[j].d;
			bno[n] = sol_layer[i].pattern[j].bno;
			if(z[n] + d[n] > _h) _h = z[n] + d[n];
 			n++;
		}
		_h_layer = _h;
	}

	printf("\n\nCreated a pallet of %d boxes. Done", n);
	printf("\n");
	return n;
}

