#pragma once

class CAttrShoe {
public:
	double mass, walkAcc, walkSpeed, walkDrag, walkSlant, walkJump, swimAcc, swimSpeedH, swimSpeedV, flyAcc, flySpeed;
	void set() {
        mass = 100.0;
        walkAcc = 1.0;
        walkSpeed = 1.4;
        walkDrag = 1.0;
        walkSlant = 0.9;
        walkJump = 1.2;
        swimAcc = 1.0;
        swimSpeedH = 1.0;
        swimSpeedV = 1.0;
        flyAcc = 0.0;
        flySpeed = 0.0;
    }
};