////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Sprite::Draw(int x, int y) {
	if (!data) {
		return;
	}
	glPushMatrix();

}