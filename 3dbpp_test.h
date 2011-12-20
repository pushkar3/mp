#ifndef TEST3DBPP_H_
#define TEST3DBPP_H_

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

#include "3dbpp.h"

#define srand(x)     srand48x(x)
#define randm(x)     (lrand48x() % (long) (x))
#define rint(a,b)    (randm((b)-(a)+1) + (a))

#define TRUE  1           /* logical variables */
#define FALSE 0

#define VOL(i)                 ((i)->w * (stype) (i)->h * (i)->d)
#define DIF(i,j)               ((int) ((j) - (i) + 1))

/* to ensure that the same instances are generated on all machines */
/* here follows C-versions of SRAND48, and LRAND48.  */
void srand48x(long s);
int lrand48x(void);

void specialbin(box *f, box *l, int W, int H, int D);
void randomtype(box *i, int W, int H, int D, int type);
boolean allgood(stype totvol, box *f, box *l);
void maketest(box *f, box *l, itype *W, itype *H, itype *D,
	      stype bdim, int type);
int readtest(box *tab, itype *W, itype *H, itype *D, char *file);
void printboxes(int n, int W, int H, int D, int *w, int *h, int *d,
                int *x, int *y, int *z, int *wt, int *id, int *bno);
void exchange(int *a, int i, int j);
void exchange_double(double *a, int i, int j);
void sort_range(int i_start, int i_end, int *a, int *b, int *c, int *d, int *e, int *f, int *g, int *h, int *k);
void printpacklistxml(const char* out, int n, int W, int H, int D, int *w, int *h, int *d, int *x, int *y, int *z, int *wt, int *id, int *bno);
void prepareboxes(box *f, box *l, int *w, int *h, int *d, int *wt, int *id);
#endif /* TEST3DBPP_H_ */
