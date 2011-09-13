////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::View_ NLS::View;

NLS::View_::View_() {
	x = 0;
	y = 0;
	vx = 0;
	vy = 0;
	tx = 0;
	ty = 0;
	xmin = 0;
	xmax = 0;
	ymin = 0;
	ymax = 0;
}

void NLS::View_::Step() {
	if (sf::Keyboard::IsKeyPressed(sf::Keyboard::Up)) {
		ty -= 10;
	}
	if (sf::Keyboard::IsKeyPressed(sf::Keyboard::Down)) {
		ty += 10;
	}
	if (sf::Keyboard::IsKeyPressed(sf::Keyboard::Left)) {
		tx -= 10;
	}
	if (sf::Keyboard::IsKeyPressed(sf::Keyboard::Right)) {
		tx += 10;
	}
	double dx = tx-vx;
	double dy = ty-vy;
	dx = max(abs(dx)-20, 0.0)*sign(dx);
	dy = max(abs(dy)-20, 0.0)*sign(dy);
	vx += Time.delta*dx/200;
	vy += Time.delta*dy/200;
	x = vx;
	y = vy;
	glLoadIdentity();
	glTranslatef(-x, -y, 0);
}