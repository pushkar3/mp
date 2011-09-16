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

void printboxes(int n, int W, int H, int D, int *w, int *h, int *d,
                int *x, int *y, int *z, int *wt, int *id, int *bno);

void prepareboxes(box *f, box *l, int *w, int *h, int *d, int *wt, int *id);

void binpack3d(int n, int W, int H, int D,
               int *w, int *h, int *d,
               int *x, int *y, int *z, int *bno,
               int *lb, int *ub,
               int nodelimit, int iterlimit, int timelimit,
               int *nodeused, int *iterused, int *timeused,
               int packingtype);

/* ======================================================================
                 bpp_solver_settings
   ====================================================================== */


class bpp_solver_settings {
public:
    int tests;
    int bdim, type, packingtype;
    int nodelimit, iterlimit, timelimit, nodeused, iterused, timeused;
    int ub, lb, solved, gap, sumnode, sumiter;
    double time, sumtime, deviation, sumub, sumlb, sumdev;

    bpp_solver_settings() {
        tests = 10;
        sumnode = sumiter = sumtime = sumdev = sumub = sumlb = gap = solved = 0;
    }

    ~bpp_solver_settings() {

    }

    void show_start() {
        printf("3DBPP PROBLEM %d %d %d %d\n", nodelimit, iterlimit, timelimit, packingtype);
    }

    void show_end() {
        printf("bdim        = %d\n", bdim);
        printf("type        = %d\n", type);
        printf("nodelimit   = %d\n", nodelimit);
        printf("iterlimit   = %d\n", iterlimit);
        printf("timelimit   = %d\n", timelimit);
        printf("packingtype = %d\n", packingtype);
        printf("solved      = %d\n", solved);
        printf("ub          = %.1f\n",   sumub / (double) tests);
        printf("lb          = %.1f\n",   sumlb / (double) tests);
        printf("gap         = %.1f\n",     gap / (double) tests);
        printf("dev         = %.2f\n",  sumdev / (double) tests);
        printf("nodes       = %.0f\n", sumnode / (double) tests);
        printf("iterations  = %.0f\n", sumiter / (double) tests);
        printf("time        = %.2f\n", sumtime / (double) tests);
    }

    void reset() {
        sumnode = sumiter = sumtime = sumdev = sumub = sumlb = gap = solved = 0;
    }
};

/* ======================================================================
                 bpp_layer
   ====================================================================== */


class bpp_layer {
public:
    int n;
    int W, H, D, max_D;
    double C;
    box tab[MAXBOXES_LAYER];

    bpp_layer() {
        n = 0;
        C = 0.0f;
    }

    ~bpp_layer() {

    }

    void set(int _W, int _H, int _D) {
        W = _W;
        H = _H;
        D = _D;
    }

    void set(box _tab, int num) {
        for(int i = n; i < n+num; i++) {
            if(n + num > MAXBOXES_LAYER)
                printf("MAXBOXES_LAYER Exceeded\n");
            tab[i] = _tab;
        }
        n = n + num;

        if(max_D < _tab.d) max_D = _tab.d;
    }

    void solve(bpp_solver_settings settings, boolean solve_one_layer) {
        int w[MAXBOXES_LAYER], h[MAXBOXES_LAYER], d[MAXBOXES_LAYER];
        int x[MAXBOXES_LAYER], y[MAXBOXES_LAYER], z[MAXBOXES_LAYER], bno[MAXBOXES_LAYER];
        int wt[MAXBOXES_LAYER], id[MAXBOXES_LAYER];

        settings.show_start();
        settings.reset();
        for (int v = 1; v <= settings.tests; v++) {
            srand(v + n); // initialize random generator
            prepareboxes(tab, tab + n - 1, w, h, d, wt, id);
            binpack3d(n, W, H, D, w, h, d, x, y, z, bno, &settings.lb, &settings.ub,
                    settings.nodelimit, settings.iterlimit, settings.timelimit, &settings.nodeused,
                    &settings.iterused, &settings.timeused, settings.packingtype);
            settings.time = settings.timeused * 0.001;
            printf("%2d : lb %2d z %2d node %9d iter %9d time %6.2f\n", v, settings.lb,
                    settings.ub, settings.nodeused, settings.iterused, settings.time);
            settings.sumnode += settings.nodeused;
            settings.sumiter += settings.iterused;
            settings.sumtime += settings.time;
            settings.gap += settings.ub - settings.lb;
            settings.deviation = (settings.ub - settings.lb) / (double) settings.lb;
            settings.sumdev += settings.deviation;
            settings.sumub += settings.ub;
            settings.sumlb += settings.lb;
            if (settings.lb == settings.ub) settings.solved++;
        }

        settings.show_end();

        C = 0.0f;
        if(solve_one_layer == true) {
            int c_new = 0;

            for (int i = 0; i < n; i++) {
                if(d[i]+z[i] <= max_D && bno[i] == 1) {
                    tab[c_new].x = x[i];
                    tab[c_new].y = y[i];
                    tab[c_new].z = z[i];
                    tab[c_new].w = w[i];
                    tab[c_new].h = h[i];
                    tab[c_new].d = d[i];
                    tab[c_new].wt = wt[i];
                    tab[c_new].id = id[i];
                    tab[c_new].bno = bno[i];
                    tab[c_new].vol = w[i]*h[i]*d[i];
                    C += tab[c_new].vol;
                    c_new++;
                }
            }

            n = c_new;
            C /= (W*H*max_D);
        }
    }

    void show() {
        printf("\n\n%d (%d,%d,%d) -> C=%lf\n", n, W, H, max_D, C);
        for (int i = 0; i < n; i++) {
          printf("%2d (%2d %2d %2d) : \tBin %2d (%2d, %2d, %2d) \t %d %d\n",
                 i, tab[i].w,tab[i].h,tab[i].d,tab[i].bno,tab[i].x,tab[i].y,tab[i].z,tab[i].wt,tab[i].id);
        }
    }

};



#endif
