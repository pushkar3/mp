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

int binpack3d_layer(int n, int W, int H, int D, int w, int h, int d, int *x,
		int *y, int *z, int *bno, int *lb, int *ub, int nodelimit,
		int iterlimit, int timelimit, int *nodeused, int *iterused,
		int *timeused, int packingtype, int off_x, int off_y, int off_z) {

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
		box_t b = bpp.box[i];
		int x1[50];
		int y1[50];
		int z1[50];
		int w1[50];
		int h1[50];
		int d1[50];
		int bno1[50];
		int lb1, ub1;
		int nodeused1 = 2;
		int iterused1 = 0;
		int timeused1 = 0;
		for(int j = 0; j < 50; j++) {
			w1[j] = b.w; h1[j] = b.h; d1[j] = b.d;
		}
		// Bin packing with w, h
		printf("Solving bpp for box (%d, %d, %d) \n", w1[0], h1[0], d1[0]);
		binpack3d(50, W, H, D, w1, h1, d1, x1, y1, z1, bno1, &lb1, &ub1, 0, 5 /*TODO*/, 0, &nodeused1, &iterused1, &timeused1, 1);
		//printboxes(50, W, H, D, w1, h1, d1, x1, y1, z1, bno1);
		layer_t l;
		l.box.push_back(i);
		for(int j = 0; j < 50; j++) {
			if(z1[j] == 0) {
				box_t b1;
				b1.x = x1[j];
				b1.y = y1[j];
				b1.z = z1[j];
				b1.w = w1[j];
				b1.h = h1[j];
				b1.d = d1[j];
				b1.bno = 1; // has to be > 0
				l.pattern.push_back(b1);
			}
		}
		l.n_box.push_back(l.pattern.size());
		layer.push_back(l);
		printf(".");
	}

	if(debug > 1) {
		printf("Layers created.");
		printf("\n\n");
		layer_debug();
		printf("\n\n");
	}

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
				printf("\nBox %d -> %d ", layer[i].box[0], layer[i].pattern.size());
				sol_layer.push_back(layer[i]);
				solved++;
			}
		}
	}

	if(debug > 0) {
		printf("Solved");
		printf("\n\n");
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

