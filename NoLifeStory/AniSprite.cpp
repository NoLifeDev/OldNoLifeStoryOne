////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::AniSprite::AniSprite() {
	frame = 0;
	delay = 0;
	repeat = true;
}

void NLS::AniSprite::Set(Node n) {
	frame = 0;
	delay = 0;
	this->n = n;
	f = n[0];
}

void NLS::AniSprite::Draw(int x, int y, bool flipped, float alpha, float rotation) {
	if (n) {
		Sprite s = f;
		s.Draw(x, y, flipped, alpha, rotation);
	}
}

void NLS::AniSprite::Step() {
	delay += Time.delta;
	int d = f["delay"];
	if (d == 0) {
		d = 100;
	}
	if (delay > d) {
		delay -= d;
		frame++;
		if (!n[frame]) {
			if (repeat) {
				frame = 0;
			} else {
				frame--;
			}
		}
		f = n[frame];
	}
}