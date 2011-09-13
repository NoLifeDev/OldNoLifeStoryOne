#pragma once

class CRelPos {
public:
	double pos, v;

	CRelPos(CRelPos* pos){
		this->pos = pos->pos;
		v = pos->v;
	}

	CRelPos() {

	}
};

#pragma once

#include "RelPos.h"
#include "MapleClient/FootHold.h"

class CAbsPos {
public:
	double x, y, vx, vy;

	CAbsPos() {

	}

	void SetFromRelPos(CRelPos rp, CFootHold* pfh) {
			x = pfh->uvx * rp.pos + pfh->x1;
			y = pfh->uvy * rp.pos + pfh->y1;
			vx = pfh->uvx * rp.v;
			vy = pfh->uvy * rp.v;
	}
};