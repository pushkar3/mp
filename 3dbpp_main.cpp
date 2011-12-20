#include <iostream>
#include <stdio.h>
#include "3dbpp.h"
#include "3dbpp_solver_settings.h"
#include "3dbpp_test.h"


itype W, H, D;
bpp_solver_settings settings;


int main(int argc, char *argv[])
{
  int N;
  box tab[MAXBOXES];
  int w[MAXBOXES], h[MAXBOXES], d[MAXBOXES];
  int x[MAXBOXES], y[MAXBOXES], z[MAXBOXES], bno[MAXBOXES];
  int wt[MAXBOXES], id[MAXBOXES];

  settings.nodelimit   = 0;
  settings.iterlimit   = 1000;
  settings.timelimit   = 50;
  settings.packingtype = 1;
  settings.bdim = 0;
  settings.type = 0;
  settings.tests = 2;

  // Try problem
  W = 4;
  H = 4;
  D = 4;

  N = 8;
  for (int i = 0; i < 8; i++) {
	  w[i] = 2;
	  h[i] = 2;
	  d[i] = 2;
	  x[i] = 0;
	  y[i] = 0;
	  z[i] = 0;
	  wt[i] = 50;
	  bno[i] = 0;
	  id[i] = i+100;
  }

  settings.show_start();
  binpack3d(N, W, H, D, w, h, d, x, y, z, bno,
		  &settings.lb, &settings.ub, settings.nodelimit, settings.iterlimit, settings.timelimit,
		  &settings.nodeused, &settings.iterused, &settings.timeused, settings.packingtype);
  settings.show_end();
  printboxes(N, W, H, D, w, h, d, x, y, z, wt, id, bno);

  return 0;
}
