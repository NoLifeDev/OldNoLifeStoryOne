////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::Sprite::Sprite() {
	data = 0;
}

void NLS::Sprite::Draw(int x, int y, bool flipped, float alpha, float rotation) {
	if (!data) {
		return;
	}
	if (View.relative) {
		if (flipped) {
			if (x+data->width+data->originx < View.x ||
				y+data->height-data->originy < View.y ||
				x+data->originx > View.x+800 ||
				y-data->originy > View.y+600) {
				return;
			}
		} else {
			if (x+data->width-data->originx < View.x ||
				y+data->height-data->originy < View.y ||
				x-data->originx > View.x+800 ||
				y-data->originy > View.y+600) {
				return;
			}
		}
	}
	glPushMatrix();
	if (flipped) {
		glTranslatef(data->originx, -data->originy, 0);
	} else {
		glTranslatef(-data->originx, -data->originy, 0);
	}
	glRotatef(rotation, 0, 0, 1);
	glTranslatef(x, y, 0);
	glColor4f(1, 1, 1, alpha);
	GetTexture();
	glBegin(GL_QUADS);
	if (flipped) {
		glTexCoord2f(1, 0);
		glVertex2i(0, 0);
		glTexCoord2f(0, 0);
		glVertex2i(data->width, 0);
		glTexCoord2f(0, 1);
		glVertex2i(data->width, data->height);
		glTexCoord2f(1, 1);
		glVertex2i(0, data->height);
	} else {
		glTexCoord2f(0, 0);
		glVertex2i(0, 0);
		glTexCoord2f(1, 0);
		glVertex2i(data->width, 0);
		glTexCoord2f(1, 1);
		glVertex2i(data->width, data->height);
		glTexCoord2f(0, 1);
		glVertex2i(0, data->height);
	}
    glEnd();
	glPopMatrix();
}

void NLS::Sprite::GetTexture() {
	if (!data) {
		glBindTexture(GL_TEXTURE_2D, NULL);
		return;
	}
	if (!data->loaded) {
		if (!data->png) {
			glBindTexture(GL_TEXTURE_2D, NULL);
			return;
		}
		((WZ::PNGProperty*)data->png)->Parse();
		data->loaded = true;
	} else {
		glBindTexture(GL_TEXTURE_2D, data->texture);
	}
}