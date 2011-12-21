#ifndef _3DBPP_H_
#define _3DBPP_H_

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

#define IUNIT        1000  /* scaling factor of nodes and iterat */
#define MAXBOXES      501  /* max number of boxes (plus one box) */
#define MAXTYPES      25
#define MAXBPP    1000000  /* max numer of iterations in 1-dim bpp */
#define MAXITER      1000  /* max iterations in heuristic onebin_robot */
#define MAXCLOSE       16  /* max level for which try_close is applied */
#define MAXBOXES_LAYER 100
/* ======================================================================
 type declarations
 ====================================================================== */

typedef short boolean; /* logical variable      */
typedef short ntype; /* number of states,bins */
typedef short itype; /* can hold up to W,H,D  */
typedef long stype; /* can hold up to W*H*D  */
typedef long ptype; /* product multiplication */

/* box record */
typedef struct irec {
    ntype no; /* box number                            */
    itype w; /* box width  (x-size)                   */
    itype h; /* box height (y-size)                   */
    itype d; /* box depth  (z-size)                   */
    itype x; /* optimal x-position                    */
    itype y; /* optimal y-position                    */
    itype z; /* optimal z-position                    */
    ntype bno; /* bin number                            */
    boolean k; /* is the box chosen?                    */
    stype vol; /* volume of box                         */
    int id;
    int wt;
    struct irec *ref; /* reference to original box (if necessary) */
} box;

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

#endif
