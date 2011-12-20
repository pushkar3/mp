
/* ======================================================================
      3D BIN PACKING, Silvano Martello, David Pisinger, Daniele Vigo 
                             1998, 2003, 2006
   ====================================================================== */

/* This code generates instances for the three-dimensional bin-packing 
 * problem and solves them using the 3DBPP algorithm by Martello, Pisinger,
 * Vigo. 
 *
 * A description of the test instances is found in the following papers:
 *   S. Martello, D. Pisinger, D. Vigo, E. den Boef, J. Korst (2003)
 *   "Algorithms for General and Robot-packable Variants of the
 *    Three-Dimensional Bin Packing Problem"
 *   submitted TOMS.
 *
 *   S.Martello, D.Pisinger, D.Vigo (2000)
 *   "The three-dimensional bin packing problem"
 *   Operations Research, 48, 256-267
 *
 * The algorithm can either read an instance from a file or randomly
 * generate 10 instances.
 *
 * If the instance is read from a file, five arguments should be given:
 *   filename  A filename in which the test instance is found. The format 
 *             of the file is:
 *                n W H D
 *                w_1 h_1 d_1
 *                :
 *                w_n h_n d_n
 *             where 
 *                n is the number of items, 
 *                W,H,D is the size of the bin, and 
 *                w_j,h_j,d_j is the size of box j.
 *   nodelimit maximum number of decision nodes to be explored in the
 *             main branching tree. If set to zero, the algorithm will
 *             run until an optimal solution is found (or timelimit or
 *             iterlimit is reached). Measured in thousands (see IUNIT).
 *   iterlimit maximum number of iterations in the ONEBIN algorithm
 *             which packs a single bin. If set to zero, the algorithm will
 *             run until an optimal solution is found (or timelimit or
 *             nodelimit is reached). Measured in thousands (see IUNIT).
 *   timelimit Time limit for solving the problem expressed in seconds.
 *             If set to zero, the algorithm will run until an optimal
 *             solution is found; otherwise it terminates after timelimit
 *             seconds with a heuristic solution. 
 *   packingtype 
 *             Desired packing type. If set to zero, the algorithm will
 *             search for an optimal general packing; if set to one, it
 *             will search for a robot packing.
 *             will search for a robot packing.
 * If the code should randomly generate 10 instances, seven arguments 
 * should be given:
 *   n         The size of the instance, i.e., number of boxes.
 *   bindim    The size of the bin, typically 40-100.
 *   type      An integer saying which randomly generated instance should
 *             be generated. A value between 1-9 selects one of the instance 
 *             types described in the above papers. Value 10-11 generates
 *             1D and 2D instances.
 *   nodelimit as above
 *   iterlimit as above
 *   timelimit as above
 *   packingtype
 *             as above
 *
 * Results are written to standard output
 * 
 * (c) Copyright 1998, 2003, 2006
 *
 *   David Pisinger                        Silvano Martello, Daniele Vigo
 *   DIKU, University of Copenhagen        DEIS, University of Bologna
 *   Universitetsparken 1                  Viale Risorgimento 2
 *   Copenhagen, Denmark                   Bologna, Italy
 * 
 * This code can be used free of charge for research and academic purposes 
 * only. 
 */

#include "3dbpp_test.h"
using namespace std;

/* ======================================================================
				global variables
   ====================================================================== */

int TESTS;


/* =======================================================================
                                random
   ======================================================================= */

/* to ensure that the same instances are generated on all machines */
/* here follows C-versions of SRAND48, and LRAND48.  */

unsigned int _h48, _l48;

void srand48x(long s)
{
  _h48 = s;
  _l48 = 0x330E;
}

int lrand48x(void)
{
  _h48 = (_h48 * 0xDEECE66D) + (_l48 * 0x5DEEC);
  _l48 = _l48 * 0xE66D + 0xB;
  _h48 = _h48 + (_l48 >> 16);
  _l48 = _l48 & 0xFFFF;
  return (_h48 >> 1);
}


/* ======================================================================
				specialbin
   ====================================================================== */

void specialbin(box *f, box *l, int W, int H, int D)
{
  box *m, *i, *j, *k;
  int w, h, d;

  if (f == l) { f->w = W; f->h = H; f->d = D; return; }
  if (DIF(f,l) == 5) {
    w = W/3; h = H/3; i = f+1; j = f+2; k = f+3; 
    i->w =   W-w; i->h =     h; i->d = D;
    j->w =     w; j->h =   H-h; j->d = D;
    k->w =   W-w; k->h =     h; k->d = D; 
    l->w =     w; l->h =   H-h; l->d = D;
    f->w = W-2*w; f->h = H-2*h; f->d = D;
    return;
  }

  m = f + (l-f) / 2;
  for (;;) {
    switch (rint(1,3)) {
      case 1: if (W < 2) break;
              w = rint(1,W-1); 
  	      specialbin(f, m, w, H, D);
	      specialbin(m+1, l, W-w, H, D);
	      return;
      case 2: if (H < 2) break;
	      h = rint(1,H-1); 
	      specialbin(f, m, W, h, D); 
	      specialbin(m+1, l, W, H-h, D);
	      return;
      case 3: if (D < 2) break;
	      d = rint(1,D-1);
	      specialbin(f, m, W, H, d); 
	      specialbin(m+1, l, W, H, D-d);
	      return;
    }
  }
}


/* ======================================================================
				randomtype
   ====================================================================== */

void randomtype(box *i, int W, int H, int D, int type)
{
  itype w, h, d, t;

  if (type <= 5) { /* Martello, Vigo */
    t = rint(1,10); if (t <= 5) type = t;
  }

  w = h = d = 0;
  switch (type) {
    /* Martello, Vigo */
    case  1: w = rint(1,W/2);   h = rint(2*H/3,H); d = rint(2*D/3,D); break;
    case  2: w = rint(2*W/3,W); h = rint(1,H/2);   d = rint(2*D/3,D); break;
    case  3: w = rint(2*W/3,W); h = rint(2*H/3,H); d = rint(1,D/2);   break;
    case  4: w = rint(W/2,H);   h = rint(H/2,H);   d = rint(D/2,D);   break;
    case  5: w = rint(1,W/2);   h = rint(1,H/2);   d = rint(1,D/2);   break;

    /* Berkey, Wang */
    case 6: w = rint(1,10);     h = rint(1,10);    d = rint(1,10);    break;
    case 7: w = rint(1,35);     h = rint(1,35);    d = rint(1,35);    break;
    case 8: w = rint(1,100);    h = rint(1,100);   d = rint(1,100);   break;

    /* 2D and 1D cases */
    case 10: w = rint(1,W);     h = rint(1,H);     d = D;             break;
    case 11: w = rint(1,W/2);   h = H;             d = D;             break;
  }
  i->w = w; i->h = h; i->d = d;
  i->x = 0; i->y = 0; i->z = 0;
}


/* ======================================================================
				allgood
   ====================================================================== */

boolean allgood(stype totvol, box *f, box *l)
{
  box *j, *m;
  stype vol;

  for (vol = 0, j = f, m = l+1; j != m; j++) {
    if ((j->w < 1) || (j->h < 1) || (j->d < 1)) return FALSE;
    vol += VOL(j);
  }
  return (vol == totvol);
}


/* ======================================================================
				maketest
   ====================================================================== */

void maketest(box *f, box *l, itype *W, itype *H, itype *D,
	      stype bdim, int type)
{
  register box *i, *k, *m;
  int no;

  /* set bin dimensions */
  *W = bdim; *H = bdim; *D = bdim;

  /* make maxtypes box types */
  for (i = f, m = l+1, no = 1; i != m; i++, no++) {
    randomtype(i, *W, *H, *D, type);
    i->no = no;
  }

  /* guillotine cut three bins */
  if (type == 9) {
    no = DIF(f,l)/3;
    k = f + no; m = k + no;
    for (;;) {
      specialbin(f  , k, *W, *H, *D); 
      specialbin(k+1, m, *W, *H, *D); 
      specialbin(m+1, l, *W, *H, *D); 
      if (allgood(3*bdim*bdim*bdim, f, l)) break;
    }
  }
}


/* ======================================================================
				readtest
   ====================================================================== */

int readtest(box *tab, itype *W, itype *H, itype *D, char *file)
{
  FILE *in;
  box *i;
  int n, w, h, d, wt, id;

  in = fopen(file, "r");
  if (in == NULL) { printf("wrong filename"); exit(-1); }

  fscanf(in,"%d %d %d %d", &n, &w, &h, &d);
  *W = w; *H = h; *D = d;
  for (i = tab; i < tab+n; i++) {
    fscanf(in,"%d %d %d %d %d", &w, &h, &d, &wt, &id);
    i->w = w; i->h = h; i->d = d;
    i->wt = wt; i->id = id;
  }
  fclose(in);
  return n;
}


/* ======================================================================
                                printboxes
   ====================================================================== */

void printboxes(int n, int W, int H, int D, int *w, int *h, int *d, 
                int *x, int *y, int *z, int *wt, int *id, int *bno)
{
  int i;
  printf("%d (%d,%d,%d)\n", n, W, H, D);
  for (i = 0; i < n; i++) {
    printf("%2d (%2d %2d %2d) : \tBin %2d (%2d, %2d, %2d) \t %d %d\n",
           i,w[i],h[i],d[i],bno[i],x[i],y[i],z[i], wt[i], id[i]);
  }
}

void exchange(int *a, int i, int j)
{
	if(a == NULL) return;
	int temp = a[i];
	a[i] = a[j];
	a[j] = temp;
}

void exchange_double(double *a, int i, int j)
{
	if(a == NULL) return;
	double temp = a[i];
	a[i] = a[j];
	a[j] = temp;
}


void sort_range(int i_start, int i_end, int *a, int *b, int *c, int *d, int *e, int *f, int *g, int *h, int *k)
{
	if (a == NULL) {
		printf("Error in %d %s\n", __LINE__, __func__);
		return;
	}

	for (int i = i_start; i < i_end; i++) {
		for (int j = i; j < i_end; j++) {
			if(a[i] > a[j]) {
				exchange(a, i, j);
				if(b!=NULL) exchange(b, i, j);
				if(c!=NULL) exchange(c, i, j);
				if(d!=NULL) exchange(d, i, j);
				if(e!=NULL) exchange(e, i, j);
				if(f!=NULL) exchange(f, i, j);
				if(g!=NULL) exchange(g, i, j);
				if(h!=NULL) exchange(h, i, j);
				if(k!=NULL) exchange(k, i, j);
			}
		}
	}
}

void printpacklistxml(const char* out, int n, int W, int H, int D, int *w, int *h, int *d, int *x, int *y, int *z, int *wt, int *id, int *bno)
{
  int i = 0;
  int max_bins = 0;

  PackList list;
  list.order_id = 1;

  PackPallet pallet[10];

  sort_range(0, n, z, x, y, w, h, d, wt, id, bno);

  for (i = 0; i < n; i++) {
	  Package package;
	  int pos_x = x[i] + w[i]/2.0;
	  int pos_y = y[i] + h[i]/2.0;
	  int pos_z = z[i] + d[i];
	  package.place_position.set(pos_x, pos_y, pos_z);
	  package.article.description = itoa(id[i]);
	  pallet[bno[i] - 1].insertPackage(package, w[i], h[i], d[i], wt[i], id[i]);
	  if(max_bins < bno[i]) max_bins = bno[i];
  }

  for (i = 0; i < max_bins; i++)
	  list.insertPallet(pallet[i], W, H, D, 0);


  write_response(list, out, 0);
}

/* ======================================================================
                                prepareboxes
   ====================================================================== */

void prepareboxes(box *f, box *l, int *w, int *h, int *d, int *wt, int *id)
{
  box *i;
  int k;

  for (i = f, k = 0; i != l+1; i++, k++) {
    w[k] = i->w; h[k] = i->h; d[k] = i->d;
    wt[k] = i->wt; id[k] = i->id;
  }
}




