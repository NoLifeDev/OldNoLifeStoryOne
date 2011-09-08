////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Sprite::Draw(int x, int y, bool flipped, float alpha, float rotation) {
	if (!data) {
		return;
	}
	glPushMatrix();
	if (flipped) {
		glTranslatef(data->originx, -data->originy, 0);
	} else {
		glTranslatef(-data->originx, -data->originy, 0);
	}
	glRotatef(rotation, 0, 0, 1);
	glColor4f(1, 1, 1, alpha);
	GetTexture();
	glBegin(GL_QUADS);
	if (flipped) {
		glTexCoord2f(data->twidth, 0);
		glVertex2i(0, 0);
		glTexCoord2f(0, 0);
		glVertex2i(data->width, 0);
		glTexCoord2f(0, data->theight);
		glVertex2i(data->width, data->height);
		glTexCoord2f(data->twidth, data->theight);
		glVertex2i(0, data->height);
	} else {
		glTexCoord2f(0, 0);
		glVertex2i(0, 0);
		glTexCoord2f(data->twidth, 0);
		glVertex2i(data->width, 0);
		glTexCoord2f(data->twidth, data->theight);
		glVertex2i(data->width, data->height);
		glTexCoord2f(0, data->theight);
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
		//TODO: Actually load the texture
	}
	glBindTexture(GL_TEXTURE_2D, data->texture);
}