
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
#define RESET   "\033[0m"
#define RED     "\033[1m\033[31m"
#define GREEN   "\033[1m\033[32m"
#define RANDOMTESTS    10     /* Number of test to run for each type */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include <xml_parser.h>
#include <packlist.h>
#include <response.h>

#include <pthread.h>

#include "3dbpp.h"

#include <Eigen/Eigen>

using namespace Eigen;
using namespace std;

/* ======================================================================
				   macros
   ====================================================================== */

#define srand(x)     srand48x(x)
#define randm(x)     (lrand48x() % (long) (x))
#define rint(a,b)    (randm((b)-(a)+1) + (a))

#define TRUE  1           /* logical variables */
#define FALSE 0

#define VOL(i)                 ((i)->w * (stype) (i)->h * (i)->d)
#define DIF(i,j)               ((int) ((j) - (i) + 1))



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


/* ======================================================================
                actual binpacking
   ====================================================================== */

void binpack(bpp_layer layer, bpp_solver_settings settings) {

}


/* ======================================================================
                Global vars to be used with mutex
   ====================================================================== */


itype W, H, D;
bpp_solver_settings settings;
bpp_layer l[1000];
int bpp_layer_count;

/* ======================================================================
				Create columns of A
   ====================================================================== */

void *create_layer_thread(void*) {
    printf(RED "[INFO] Starting create_layer_thread\n" RESET);

    bpp_layer_count = 0;
    box t[4];
    int tcap[4];
    t[0].w = 43;
    t[0].h = 132;
    t[0].d = 41;
    t[0].wt = 500;
    t[0].id = 1;

    t[1].w = 43;
    t[1].h = 86;
    t[1].d = 41;
    t[1].wt = 500;
    t[1].id = 2;

    t[2].w = 67;
    t[2].h = 90;
    t[2].d = 82;
    t[2].wt = 500;
    t[2].id = 3;

    t[3].w = 66;
    t[3].h = 132;
    t[3].d = 41;
    t[3].wt = 500;
    t[3].id = 4;

    for(int i = 0; i < 1000; i++) {
        l[i].set(W, H, D);
        l[i].set(settings);
    }

    l[0].set(t[0], 40);
    l[1].set(t[1], 40);
    l[2].set(t[2], 40);
    l[3].set(t[3], 40);
    tcap[0] = l[0].solve(true);
    tcap[1] = l[1].solve(true);
    tcap[2] = l[2].solve(true);
    tcap[3] = l[3].solve(true);

    int i[4];
    bpp_layer_count = 4;
    for (i[0] = 0; i[0] < tcap[0]; i[0]++) {
        for (i[1] = 0; i[1] < tcap[1]; i[1]++) {
            for (i[2] = 0; i[2] < tcap[2]; i[2]++) {
                for (i[3] = 0; i[3] < tcap[3]; i[3]++) {
                    l[bpp_layer_count].set(t[0], i[0]);
                    l[bpp_layer_count].set(t[1], i[1]);
                    l[bpp_layer_count].set(t[2], i[2]);
                    l[bpp_layer_count].set(t[3], i[3]);
                    printf(GREEN "Solved %d [%d %d %d %d]: %d\n" RESET, bpp_layer_count,
                            i[0],i[1],i[2],i[3],l[bpp_layer_count].solve(true));
                    l[bpp_layer_count].show();
                    bpp_layer_count++;
                }
            }
        }
    }

    printf(RED "[INFO] Terminating create_layer_thread\n" RESET);
    pthread_exit(NULL);
}


/* ======================================================================
                main
   ====================================================================== */


int main(int argc, char *argv[])
{

  pthread_t thread_layer, thread_lp;

  int N;
  box tab[MAXBOXES];
  int w[MAXBOXES], h[MAXBOXES], d[MAXBOXES];
  int x[MAXBOXES], y[MAXBOXES], z[MAXBOXES], bno[MAXBOXES];
  int wt[MAXBOXES], id[MAXBOXES];
  char file[1000];
  char file_packlist[100] = "out.xml";
  char problem[10] = "test.p";

  if (argc == 3) {
      OrderXML oxml;
      oxml.convertToProblem(argv[1], problem);
      strcpy(file, problem);
      strcpy(file_packlist, argv[2]);
      settings.nodelimit   = 0;
      settings.iterlimit   = 1000;
      settings.timelimit   = 50;
      settings.packingtype = 1;
      settings.bdim = 0;
      settings.type = 0;
      settings.tests = 2;
      N = readtest(tab, &W, &H, &D, file);
  }

  if ((argc != 3)) {
    printf("3DBPP PROBLEM\n");
    printf("n = ");
    scanf("%d", &N);
    printf("bindim = ");
    scanf("%d", &settings.bdim);
    printf("type = ");
    scanf("%d", &settings.type);
    printf("nodelimit = ");
    scanf("%d", &settings.nodelimit);
    printf("iterlimit = ");
    scanf("%d", &settings.iterlimit);
    printf("timelimit = ");
    scanf("%d", &settings.timelimit);
    printf("packingtype = ");
    scanf("%d", &settings.packingtype);
    settings.tests = RANDOMTESTS;
  }

  int rc = pthread_create(&thread_layer, NULL, create_layer_thread, NULL);

  int m = 3;
  int n = 3;
  MatrixXf A(m, n);
  MatrixXf basis(m, m);
  MatrixXf basis_index(n, 1);
  MatrixXf c(n, 1);
  MatrixXf b(m, 1);
  MatrixXf xb(n, 1);

  A.setIdentity();
  b << 10, 10, 10;
  xb = A.inverse()*b;

//  if (settings.type == 0) printpacklistxml(file_packlist, n, W, H, D, w, h, d, x, y, z, wt, id, bno);

  pthread_exit(NULL);
  return 0; // correct termination
}



