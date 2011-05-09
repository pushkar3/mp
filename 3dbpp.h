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
