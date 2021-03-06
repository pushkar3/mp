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
#include <vector>
#include <map>

#include "3dbpp.h"


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
				 type declarations
   ====================================================================== */

typedef int           boolean; /* logical variable      */
typedef int           ntype;   /* number of states,bins */
typedef short         itype;   /* can hold up to W,H,D  */
typedef int           stype;   /* can hold up to W*H*D  */

typedef int (*funcptr) (const void *, const void *);

/* box record */
typedef struct irec {
  ntype    no;           /* box number */
  itype    w;            /* box x-size */
  itype    h;            /* box y-size */
  itype    d;            /* box z-size */
  itype    x;            /* optimal x-position */
  itype    y;            /* optimal y-position */
  itype    z;            /* optimal z-position */
  ntype    bno;          /* bin number */
  stype    vol;          /* volume of box */
  itype	   wt;
  itype    id;
} box;


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
    printf("%2d (%2d %2d %2d) : Bin %2d (%2d, %2d, %2d) \t %d %d\n",
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
	  pallet[0].insertPackage(package, w[i], h[i], d[i], wt[i], id[i]);
	  // TODO: For multiple pallets it should be bno[i]-1
	  // if(bno[i] > max_bins) max_bins = bno[i];
	  max_bins = bno[i] = 1;
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
				main
   ====================================================================== */

int main(int argc, char *argv[])
{
  int v, n;
  itype W, H, D;
  int bdim, type, packingtype;
  int nodelimit, iterlimit, timelimit, nodeused, iterused, timeused;
  box tab[MAXBOXES];
  int w[MAXBOXES], h[MAXBOXES], d[MAXBOXES];
  int x[MAXBOXES], y[MAXBOXES], z[MAXBOXES], bno[MAXBOXES];
  int wt[MAXBOXES], id[MAXBOXES];
  int ub, lb, solved, gap, sumnode, sumiter;
  double time, sumtime, deviation, sumub, sumlb, sumdev;
  char file[1000];
  char file_packlist[100] = "out.xml";
  char problem[10] = "test.p";

  if (argc == 3) {
	  OrderXML oxml;
	  oxml.convertToProblem(argv[1], problem);
	  strcpy(file, problem);
	  strcpy(file_packlist, argv[2]);
	  nodelimit   = 0;
	  iterlimit   = 0;
	  timelimit   = 0;
	  packingtype = 0;
	  printf("3DBPP PROBLEM %s %d %d %d %d\n",
			 file, nodelimit, iterlimit, timelimit, packingtype);
	  n = readtest(tab, &W, &H, &D, file);
	  bdim = 0;
	  type = 0;
	  TESTS = 2;
  }

  if (argc == 6) {
    strcpy(file, argv[1]);
    nodelimit   = atoi(argv[2]);
    iterlimit   = atoi(argv[3]);
    timelimit   = atoi(argv[4]);
    packingtype = atoi(argv[5]);
    printf("3DBPP PROBLEM %s %d %d %d %d\n", 
           file, nodelimit, iterlimit, timelimit, packingtype);
    n = readtest(tab, &W, &H, &D, file);
    bdim = 0;
    type = 0;
    TESTS = 1;
  } 
  if (argc == 8) {
    n           = atoi(argv[1]);
    bdim        = atoi(argv[2]);
    type        = atoi(argv[3]);
    nodelimit   = atoi(argv[4]);
    iterlimit   = atoi(argv[5]);
    timelimit   = atoi(argv[6]);
    packingtype = atoi(argv[7]);
    TESTS = RANDOMTESTS;
  }
  if ((argc != 3) && (argc != 6) && (argc != 8)) {
    printf("3DBPP PROBLEM\n");
    printf("n = ");
    scanf("%d", &n);
    printf("bindim = ");
    scanf("%d", &bdim);
    printf("type = ");
    scanf("%d", &type);
    printf("nodelimit = ");
    scanf("%d", &nodelimit);
    printf("iterlimit = ");
    scanf("%d", &iterlimit);
    printf("timelimit = ");
    scanf("%d", &timelimit);
    printf("packingtype = ");
    scanf("%d", &packingtype);
    TESTS = RANDOMTESTS;
  }

  printf("3DBPP PROBLEM %d %d %d %d %d %d %d\n", 
          n, bdim, type, nodelimit, iterlimit, timelimit, packingtype);
  sumnode = sumiter = sumtime = sumdev = sumub = sumlb = gap = solved = 0;
  for (v = 1; v <= TESTS; v++) {
    srand(v+n); /* initialize random generator */
    if (type != 0) maketest(tab, tab+n-1, &W, &H, &D, bdim, type);
    prepareboxes(tab, tab+n-1, w, h, d, wt, id);
    sort_range(0, n, d, w, h, x, y, z, wt, id, bno);

    int nt = binpack3d_layer(n, W, H, D, w, h, d, x, y, z, wt, id, bno, &lb, &ub,
              nodelimit, iterlimit, timelimit, 
              &nodeused, &iterused, &timeused, 
              packingtype);

    if (type == 0) printpacklistxml(file_packlist, nt, W, H, D, w, h, d, x, y, z, wt, id, bno);
    //if (type == 0) printboxes(nt, W, H, D, w, h, d, x, y, z, wt, id, bno);

    printf("\n\n");
    return 0;

    time = timeused * 0.001;
    printf("%2d : lb %2d z %2d node %9d iter %9d time %6.2f\n", 
            v, lb, ub, nodeused, iterused, time); 
    sumnode += nodeused;
    sumiter += iterused;
    sumtime += time; 
    gap += ub - lb;
    deviation = (ub - lb) / (double) lb;
    sumdev += deviation;
    sumub += ub;
    sumlb += lb;
    if (lb == ub) solved++;
    if (type == 0) printpacklistxml(file_packlist, n, W, H, D, w, h, d, x, y, z, wt, id, bno);
    if (type == 0) printboxes(n, W, H, D, w, h, d, x, y, z, wt, id, bno);
  }
  printf("n           = %d\n", n);
  printf("bdim        = %d\n", bdim);
  printf("type        = %d\n", type);
  printf("nodelimit   = %d\n", nodelimit);
  printf("iterlimit   = %d\n", iterlimit);
  printf("timelimit   = %d\n", timelimit);
  printf("packingtype = %d\n", packingtype);
  printf("solved      = %d\n", solved);
  printf("ub          = %.1f\n",   sumub / (double) TESTS);
  printf("lb          = %.1f\n",   sumlb / (double) TESTS);
  printf("gap         = %.1f\n",     gap / (double) TESTS);
  printf("dev         = %.2f\n",  sumdev / (double) TESTS);
  printf("nodes       = %.0f\n", sumnode / (double) TESTS);
  printf("iterations  = %.0f\n", sumiter / (double) TESTS);
  printf("time        = %.2f\n", sumtime / (double) TESTS);

  return 0; /* correct termination */
}

