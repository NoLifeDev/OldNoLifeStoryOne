////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Foothold::Load(Node n) {
	n = n["foothold"];
	if (!n) {
		cerr << "ERROR: No foothold node" << endl;
		throw(273);
	}
	for (auto i = n.Begin(); i != n.End(); i++) {
		int fhdepth = toint(i->first);
		for (auto j = i->second.Begin(); j != i->second.End(); j++) {
			int fhgroup = toint(j->first);
			for (auto k = j->second.Begin(); k != j->second.End(); k++) {
				Node& fn = k->second;
				Foothold* fh = new Foothold();
				fh->x1 = fn["x1"];
				fh->y1 = fn["y1"];
				fh->x2 = fn["x2"];
				fh->y2 = fn["y2"];
				fh->nextid = fn["next"];
				fh->previd = fn["prev"];
				fh->force = fn["force"];
				fh->forbid = (int)fn["forbidFallDown"];
				fh->id = toint(k->first);
				fh->layer = fhdepth;
				fh->group = fhgroup;
				//fh->dir = 
			}
		}
	}
}