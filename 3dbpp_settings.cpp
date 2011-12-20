#include <stdio.h>
#include "3dbpp_settings.h"

bpp_settings::bpp_settings() {
	tests = 10;
	sumnode = sumiter = sumtime = sumdev = sumub = sumlb = gap = solved = 0;
	packingtype = 0;
}

bpp_settings::~bpp_settings() {

}

bpp_settings::bpp_settings (const bpp_settings& s) {
	tests = s.tests;
	bdim = s.bdim;
	type = s.type;
	packingtype = s.packingtype;
	nodelimit = s.nodelimit;
	iterlimit = s.iterlimit;
	timelimit = s.timelimit;
	nodeused = s.nodeused;
	iterused = s.iterused;
	timeused = s.timeused;
	ub = s.ub;
	lb = s.lb;
	solved = s.solved;
	gap = s.gap;
	sumnode = s.sumnode;
	sumiter = s.sumiter;
	time = s.time;
	sumtime = s.sumtime;
	deviation = s.deviation;
	sumub = s.sumub;
	sumlb = s.sumlb;
	sumdev = s.sumdev;
}

void bpp_settings::show_start() {
	printf("3DBPP PROBLEM %d %d %d %d\n", nodelimit, iterlimit, timelimit,
			packingtype);
}

void bpp_settings::show_end() {
	printf("bdim        = %d\n", bdim);
	printf("type        = %d\n", type);
	printf("nodelimit   = %d\n", nodelimit);
	printf("iterlimit   = %d\n", iterlimit);
	printf("timelimit   = %d\n", timelimit);
	printf("packingtype = %d\n", packingtype);
	printf("solved      = %d\n", solved);
	printf("ub          = %.1f\n", sumub / (double) tests);
	printf("lb          = %.1f\n", sumlb / (double) tests);
	printf("gap         = %.1f\n", gap / (double) tests);
	printf("dev         = %.2f\n", sumdev / (double) tests);
	printf("nodes       = %.0f\n", sumnode / (double) tests);
	printf("iterations  = %.0f\n", sumiter / (double) tests);
	printf("time        = %.2f\n", sumtime / (double) tests);
}

void bpp_settings::reset() {
	sumnode = sumiter = sumtime = sumdev = sumub = sumlb = gap = solved = 0;
}
