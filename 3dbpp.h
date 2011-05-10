#define IUNIT        1000  /* scaling factor of nodes and iterat */
#define MAXBOXES      501  /* max number of boxes (plus one box) */
#define MAXBPP    1000000  /* max numer of iterations in 1-dim bpp */
#define MAXITER      1000  /* max iterations in heuristic onebin_robot */
#define MAXCLOSE       16  /* max level for which try_close is applied */

/* ======================================================================
                                binpack3d
   ====================================================================== */

void binpack3d(int n, int W, int H, int D,
               int *w, int *h, int *d,
               int *x, int *y, int *z, int *bno,
               int *lb, int *ub,
               int nodelimit, int iterlimit, int timelimit,
               int *nodeused, int *iterused, int *timeused,
               int packingtype);

int binpack3d_layer(int n, int W, int H, int D,
               int *w, int *h, int *d,
               int *x, int *y, int *z, int *bno,
               int *lb, int *ub,
               int nodelimit, int iterlimit, int timelimit,
               int *nodeused, int *iterused, int *timeused,
               int packingtype);

void printboxes(int n, int W, int H, int D, int *w, int *h, int *d,
                int *x, int *y, int *z, int *bno);

void exchange(int *a, int i, int j);

void exchange_double(double *a, int i, int j);

void sort_range(int i_start, int i_end, int *a, int *b=NULL, int *c=NULL, int *d=NULL, int *e=NULL, int *f=NULL, int *g=NULL);

void sort_sol_range(int i_start, int i_end, int *a, int *b, double *c, int use_h_to_sort);
