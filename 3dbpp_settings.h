#ifndef BPP_SOLVER_SETTINGS_H_
#define BPP_SOLVER_SETTINGS_H_

class p_type {
public:
	int w, h, d, n;

	p_type(): w(0), h(0), d(0), n(0) {}
	p_type(int _w, int _h, int _d, int _n) { w=_w; h=_h; d=_d; n=_n; }
};

class b_type {
public:
	int W, H, D, N;
};

class bpp_settings {
public:
    int tests;
    int bdim, type, packingtype;
    int nodelimit, iterlimit, timelimit, nodeused, iterused, timeused;
    int ub, lb, solved, gap, sumnode, sumiter;
    double time, sumtime, deviation, sumub, sumlb, sumdev;

    bpp_settings();
    ~bpp_settings();

    bpp_settings (const bpp_settings& s);
    void show_start();
    void show_end();
    void reset();
};


#endif /* BPP_SOLVER_SETTINGS_H_ */
