#include "MaplePyhsics.h"
#include "Game.h"

const double walkForce = 140000;
const double walkSpeed = 125;
const double walkDrag = 80000;
const double slipForce = 60000;
const double slipSpeed = 120;
const double floatDrag1 = 100000;
const double floatDrag2 = 10000;
const double floatCoefficient = 0.01;
const double swimForce = 120000;
const double swimSpeed = 140;
const double flyForce = 120000;
const double flySpeed = 200;
const double gravityAcc = 2000;
const double fallSpeed = 670;
const double jumpSpeed = 555;
const double maxFriction = 2;
const double minFriction = 0.05;
const double swimSpeedDec = 0.9;
const double flyJumpDec = 0.35;
const double emotime = 4000;

#define A5(x) (x >= 0.0 ? x + 0.5 : x - 0.5)

CPyhsics::CPyhsics(int x, int y) {
	m_pCurAttrShoe.set();
	setXY(x,y);
	m_pfh = NULL;
	m_pfhLast = NULL;
	m_pLadderOrRope = NULL;
	dwLast = 0;
	delay = 0;
	m_nMaxFreeFallTickCount = 0;
	m_bImmovable = FALSE;
	m_bJumpNext = FALSE;
	m_bJumpDownNext = FALSE;
	m_bLadderOrRopeNext = FALSE;
	m_nInputX = 0;
	m_nInputY = 0;

}

void CPyhsics::setXY(int x, int y) {
	this->x = x;
	this->y = y;
	m_ap.x = (double)x;
	m_ap.x = (double)y;
	m_ap.vx = 0;
	m_ap.vy = 0;
	OnAttachedObjectChanged(NULL, NULL, 0);
}

void CPyhsics::UpdateActive() {
	if(GetTickCount() < dwLast+delay )
		return;
	int r = BeginUpdateActive();
	if (r > 0)
	{
		if (WorkUpdateActive(r))
			EndUpdateActive();
	}
	dwLast = GetTickCount();
	delay = r;
}

void CPyhsics::EndUpdateActive() {
	x = (int)m_ap.x;
	y = (int)m_ap.y;
}

int CPyhsics::BeginUpdateActive() {
	m_apl = m_ap;
	if (m_apl.vy < fallSpeed)
		m_nMaxFreeFallTickCount = 0;
	else
		m_nMaxFreeFallTickCount++;
	return 30;
}



bool CPyhsics::WorkUpdateActive(int tElapse) {
	bool left, right, up, down, jump;
	left = Game->Input->bKey[SDLK_LEFT];
	right = Game->Input->bKey[SDLK_RIGHT];
	up = Game->Input->bKey[SDLK_UP];
	down = Game->Input->bKey[SDLK_DOWN];
	jump = Game->Input->bKey[SDLK_LALT] || Game->Input->bKey[SDLK_RALT];

	m_nInputX = 0;
	if (right) m_nInputX++;
	if (left) m_nInputX--;
	m_nInputY = 0;
	if (up) m_nInputY--;
	if (down) m_nInputY++;
	if (jump && (m_pLadderOrRope != NULL || m_pfh != NULL || Game->Map->fly < 0))
	{
		if (down)
		{
			if(m_pfh != NULL && Game->Map->IsFootholdBelow(m_ap.x, m_ap.y, 30 * 10, m_pfh)) m_bJumpDownNext = true;
		}
		else if(m_pLadderOrRope == NULL || m_nInputX != 0) m_bJumpNext = true;
		if(m_pLadderOrRope == NULL) m_bImmovable = true;
	}
	else
	{
		m_bImmovable = false;
		if(m_pLadderOrRope == NULL)
			m_bLadderOrRopeNext = true;
	}
	//getnextinput
	if(m_bImmovable) {
		WorkUpdateActive2(tElapse);
		return true;
	}
	if (m_pLadderOrRope != NULL)
	{
		WorkUpdateActiveLadderOrRope();
		return true;
	}
	WorkUpdateActive2(tElapse);

	int a5x = A5(m_ap.x);
	int a5y = A5(m_ap.y);
	CLadder* lor = Game->Map->GetLadderOrRope(A5(m_apl.x), A5(m_apl.y), a5x, a5y);

	if (m_pfh == NULL && m_ap.vy > 0.0 && m_pLadderOrRope == NULL && m_nInputY < 0 && IsAbleToClimbLadderOrRope() && lor != NULL)
	{
		m_ap.vy = 0.0;
		m_ap.vx = 0.0;
		m_ap.x = lor->x;
		OnAttachedObjectChanged(NULL, lor, 0);
	}
	else
	{
		lor = Game->Map->GetLadderOrRope(a5x, a5y - 30 + 10, a5x, a5y);
		if (m_pfh != NULL && m_pLadderOrRope == NULL && m_nInputY < 0 && IsAbleToClimbLadderOrRope() && lor != NULL)
		{
			m_ap.vy = 0.0;
			m_ap.vx = 0.0;
			m_ap.x = lor->x;
			if (lor->y2 < m_ap.y)
				m_ap.y = lor->y2;
			OnAttachedObjectChanged(NULL, lor, 0);
		}
		else
		{
			lor = Game->Map->GetLadderOrRope(a5x, a5y, a5x, a5y + 30 / 3);
			if (m_pfh != NULL && m_pLadderOrRope == NULL && m_nInputY > 0 && IsAbleToClimbLadderOrRope() && lor != NULL && a5y <= lor->y1 && a5y >= lor->y1 - 30 / 3)
			{
				m_ap.vy = 0.0;
				m_ap.vx = 0.0;
				m_ap.x = lor->x;
				m_ap.y = lor->y1;
				OnAttachedObjectChanged(NULL, lor, 0);
			}
		}
	}
	m_bLadderOrRopeNext = false;
	return true;
}

bool CPyhsics::WorkUpdateActive2(int tElapse)
{
	CAbsPos apl;
	CRelPos rpl;

	if (m_bJumpNext)
		Jump();
	if (m_bJumpDownNext)
		JumpDown();
	//if ( m_impactNext.bValid )
	//	Impact();
	if (m_pfh != NULL)
	{
		rpl = m_rp;
		CalcWalk(tElapse);
		
		if (CollisionDetectWalk(rpl, tElapse))
		{
			if (tElapse > 0)
			{
				if (m_pfh == NULL)
				{
					apl = m_ap;
					m_ap.x += apl.vx * tElapse * 0.001;
					m_ap.y += apl.vy * tElapse * 0.001;
					CollisionDetectFloat(apl, tElapse, false);
				}
				else
				{
					rpl.pos = m_rp.pos;
					rpl.v = m_rp.v;
					m_rp.pos += rpl.v * tElapse * 0.001;
					CollisionDetectWalk(rpl, tElapse);
				}
			}
		}
	}
	else
	{
		apl = m_ap;
		CalcFloat(tElapse);
		if (CollisionDetectFloat(apl, tElapse, true))
		{
			if (tElapse > 0)
			{
				if (m_pfh == NULL)
				{
					CollisionDetectFloat(apl, tElapse, false);
				}
				else
				{
					rpl = m_rp;
					m_rp.pos += tElapse * 0.001 * rpl.v;
					CollisionDetectWalk(rpl, tElapse);
				}
			}
		}
	}
	MakeContinuousMovePath(tElapse);
	return true;
}

void CPyhsics::CalcFloat(int tElapse)
{
	double mass = m_pCurAttrShoe.mass;
	double vx = m_ap.vx;
	double vy = m_ap.vy;
	double g = floatDrag1;
	double vMax = floatDrag2;
	double tSec = tElapse * 0.001;
	double vMid;

	if (!IsFreeFalling())
	{
		double fly = abs(Game->Map->fly);

		if (Game->Map->fly >= 0)
		{
			vMid = m_pCurAttrShoe.flyAcc;
			vMax = m_pCurAttrShoe.flySpeed * fly * flySpeed;
		}
		else
		{
			vMid = m_pCurAttrShoe.swimAcc;
			vMax = m_pCurAttrShoe.swimSpeedH * fly * swimSpeed;
		}

		if (vMax >= 0.0)
		{
			if (vx <= vMax)
			{
				if (vx < -vMax)
				{
					vx = g / mass * tSec + vx;
					if (vx > -vMax)
						vx = -vMax;
				}
			}
			else
			{
				vx -= g / mass * tSec;
				if (vx < vMax)
					vx = vMax;
			}
		}
		if (m_nInputX != 0)
		{
			AccSpeed(vx, (double)m_nInputX * vMid * fly * flyForce, mass, vMax, tSec);
		}
		else
		{
			if (vx < 0.0)
			{
				vx += g / mass * tSec;
				if (vx > 0)
				{
					vx = 0.0;
				}
			}
			else if (vx > 0.0)
			{
				vx -= g / mass * tSec;
				if (vx < 0.0)
				{
					vx = 0.0;
				}
			}
		}

		if (vMax >= 0.0)
		{
			if (vy <= vMax)
			{
				double fx = -vMax;
				if (vy < fx)
				{
					vy += g / mass * tSec;
					if (vy > fx) vy = fx;
				}
			}
			else
			{
				vy -= g / mass * tSec;
				if (vy < vMax)
				{
					vy = vMax;
				}
			}
		}

		g = flyForce / mass / fly * vMid;

		if (m_nInputY < 0)
		{
			if (vy < vMax * 0.3)
			{
				vy += g * tSec * 0.5; ;
				if (vy > vMax * 0.3)
					vy = vMax * 0.3;
			}
			else if (vy > vMax * 0.3)
			{
				vy -= g * tSec;
				if (vy < vMax * 0.3)
					vy = vMax * 0.3;
			}
		}
		else if (m_nInputY > 0)
		{
			if (vy < vMax * 1.5)
			{
				vy += g * tSec;
				if (vy > vMax * 1.5)
					vy = vMax * 1.5;
			}
			else if (vy > vMax * 1.5)
			{
				vy -= g * tSec * 0.5;
				if (vy < vMax * 1.5)
					vy = vMax * 1.5;
			}
		}

		else
		{
			AccSpeed(vy, g * mass, mass, vMax, tSec);
		}
	}
	else
	{
		vMid = fallSpeed;
		if (vy > 0.0)
		{
			if (vMid >= 0.0)
			{
				if (vy <= vMid)
				{
					if (vy < -vMid)
					{
						vy += vMax / mass * tSec;
						if (vy > -vMid)
							vy = -vMid;
					}
				}
				else
				{
					vy -= vMax / mass * tSec;
					if (vy < vMid)
						vy = vMid;
				}
			}
		}
		AccSpeed(vy, mass * gravityAcc, mass, vMid, tSec);
		if (m_nInputX != 0)
		{
			AccSpeed(vx, m_nInputX * vMax * 2, mass, vMax * 0.0008928571428571428, tSec);
		}
		else
		{
			if (vy < vMid)
			{
				vMid = vMax * floatCoefficient;
				if (vx > 0.0)
				{
					vx -= vMid / mass * tSec;
					if (vx < 0.0)
						vx = 0.0;
				}
				else if (vx < 0.0)
				{
					vx += vMid / mass * tSec;
					if (vx > 0.0)
						vx = 0.0;
				}
			}
			else
			{
				if (vx > 0.0)
				{
					vx -= -vMax / mass * tSec;
					if (vx < 0.0)
						vx = 0.0;
				}
				else if (vx < 0.0)
				{
					vx += vMax / mass * tSec;
					if (vx > 0.0)
						vx = 0.0;
				}
			}
		}
	}
	m_ap.x = (vx + m_ap.vx) * tSec * 0.5 + m_ap.x;
	m_ap.y = (vy + m_ap.vy) * tSec * 0.5 + m_ap.y;
	m_ap.vx = vx;
	m_ap.vy = vy;
}

bool CPyhsics::IsFreeFalling()
{
	return m_pfh == NULL && Game->Map->fly >= 0.0 && m_pCurAttrShoe.flyAcc <= 0.0;
}

bool CPyhsics::CollisionDetectFloat(CAbsPos p1, int &tElapse, bool bCalcFloat)
{
	int tElapseMax = BoundPosMapRange(p1, tElapse);

	int x0 = A5(p1.x);
	int y0 = A5(p1.y);
	int x = A5(m_ap.x);
	int y = A5(m_ap.y);

	if (x == x0 && y == y0)
	{
		tElapse -= tElapseMax;
		MakeContinuousMovePath(tElapseMax);
		return false;
	}

	CFootHold* _fh = NULL;
	CFootHold* _fh1 = NULL;

	std::vector<CFootHold*> lRes = Game->Map->GetFootholds(x0, y0, x, y);

	int o1 = 1;
	int o2 = 2;
	double tx = 0, ty = 0;

	if (lRes.size() == 0)
	{
		tElapse -= tElapseMax;
		MakeContinuousMovePath(tElapseMax);
		return false;
	}
	for(UINT i = 0; i < lRes.size();i++) {
		CFootHold* fh = lRes[i];

		if (fh->uvx > 0.0 /*|| fh->group == Game->Map->m_nBaseZMass ||*/ /*fh->group == m_lZMass*/){
			int line0 = (int)((fh->x2 - fh->x1) * (y0 - fh->y1) - (fh->y2 - fh->y1) * (x0 - fh->x1));
			int line = (int)((fh->x2 - fh->x1) * (y - fh->y1) - (fh->y2 - fh->y1) * (x - fh->x1));
			if (line0 > 0 || line < 0 || (line0 == 0 && line == 0)) continue;
			int line1 = (int)((x - x0) * (fh->y1 - y0) - (y - y0) * (fh->x1 - x0));
			int line2 = (int)((x - x0) * (fh->y2 - y0) - (y - y0) * (fh->x2 - x0));
			if (line1 < 0 && line2 < 0) continue;
			if (line1 > 0 && line2 > 0) continue;
			CFootHold* fh1;
			CFootHold* fh2;
			if (line1 == 0)
			{
				if (fh->prev == NULL || !is_blocked_area(fh->prev, fh, x, y)) continue;
				fh2 = fh;
				fh1 = fh->prev;
			}
			else if (line2 == 0)
			{
				if (fh->next == NULL || !is_blocked_area(fh, fh->next, x, y)) continue;
				fh2 = fh->next;
				fh1 = fh;
			}
			else
			{
				fh1 = fh;
				fh2 = fh;
			}
			int n1 = (int)abs((x - x0) * (fh->y2 - fh->y1) - (y - y0) * (fh->x2 - fh->x1)); // AREA of [fh->p1, p0, fh->p2, p]
			int n2 = (int)abs(fh->x1 * fh->y2 - fh->y1 * fh->x2 + y0 * (fh->x2 - fh->x1) - x0 * (fh->y2 - fh->y1)); // AREA of [fh->p1, p0, fh->p2]
			if (n2 == 0 && fh->uvx != 0)
			{
				continue;
			}
			if (o2 * n1 > o1 * n2)
			{
				o1 = n1;
				o2 = n2;
				_fh = fh1;
				_fh1 = fh2;
				tx = (int)(x0 + (((y0 - fh->y1) * (fh->x2 - fh->x1) - (x0 - fh->x1) * (fh->y2 - fh->y1)) / ((x - x0) * (fh->y2 - fh->y1) - (y - y0) * (fh->x2 - fh->x1))) * (x - x0));
				ty = (x == x0) ? ((tx - fh->x1) * (fh->y2 - fh->y1) / (double)(fh->x2 - fh->x1) + fh->y1) : ((tx - x0) * (y - y0) / (double)(x - x0) + y0);
			}
			else
			{
				if ((fh2->y1 - _fh->y1) * (_fh->x2 - _fh->x1) - (_fh->y2 - _fh->y1) * (fh2->x1 - _fh->x1) < 0) //m(fh2->p1, _fh->p1) < m(_fh->p2, _fh->p1)
					_fh = fh2;
				if ((fh1->y2 - _fh1->y1) * (_fh1->x2 - _fh1->x1) - (_fh1->y2 - _fh1->y1) * (fh1->x2 - _fh1->x1) < 0) //m(fh2->p1, _fh->p1) < m(_fh->p2, _fh->p1)
					_fh1 = fh1;
			}
		}
	}
	if (_fh == NULL || _fh1 == NULL)
	{
		tElapse -= tElapseMax;
		MakeContinuousMovePath(tElapseMax);
		return false;
	}
	double prop = o2 / (double)o1;
	m_rp.v = _fh->uvy * ((m_ap.vy - p1.vy) * prop + p1.vy) + _fh->uvx * ((m_ap.vx - p1.vx) * prop + p1.vx);
	if (_fh != _fh1)
	{
		double _fh1v = _fh1->uvy * ((m_ap.vy - p1.vy) * prop + p1.vy) + _fh1->uvx * ((m_ap.vx - p1.vx) * prop + p1.vx);
		if (m_rp.v <= 0.001 || _fh1v <= 0.001)
		{
			if (m_rp.v >= -0.001 || _fh1v >= -0.001)
			{
				if (_fh->x1 >= _fh->x2)
					_fh = _fh1;
				m_rp.v = 0.0;
			}
			else
			{
				m_rp.v = _fh1v;
				_fh = _fh1;
			}
		}
	}
	tElapse -= (int)(tElapseMax * prop);
	m_rp.pos = (abs(_fh->uvx) <= 0.5) ? ((ty - _fh->y1) / _fh->uvy) : ((tx - _fh->x1) / _fh->uvx);
	if (m_rp.pos > _fh->len)
		m_rp.pos = _fh->len;
	else if (m_rp.pos < 0)
		m_rp.pos = 0.0;
	if (_fh->uvx > 0.0)
	{
		m_rp.v = (m_nInputX * m_rp.v < 0) ? 0.0 : m_rp.v * 0.5;
		m_ap.SetFromRelPos(m_rp, _fh);
		OnAttachedObjectChanged(_fh, NULL, (int)(tElapseMax * prop));
	}
	else // | platform
	{
		if (bCalcFloat)
		{
			m_ap.vx = _fh->uvx * (_fh->uvy * m_ap.vy + _fh->uvx * m_ap.vx);
			m_ap.vy = _fh->uvy * (_fh->uvy * m_ap.vy + _fh->uvx * m_ap.vx);
			m_ap.x = (_fh->uvx * m_rp.v + m_ap.vx) * tElapse * 0.001 * 0.5 + _fh->uvx * m_rp.pos + _fh->x1;
			m_ap.y = (_fh->uvy * m_rp.v + m_ap.vy) * tElapse * 0.001 * 0.5 + _fh->uvy * m_rp.pos + _fh->y1;
		}
		else
		{
			m_ap.SetFromRelPos(m_rp, _fh);
		}
		MakeContinuousMovePath((int)(tElapseMax * prop));
	}
	return true;
}

bool CPyhsics::is_blocked_area(CFootHold* pfh1, CFootHold* pfh2, int x, int y)
{
	if ((pfh1->x2 - pfh1->x1) * (pfh2->y2 - pfh1->y1) - (pfh2->x2 - pfh1->x1) * (pfh1->y2 - pfh1->y1) <= 0)// m(p1,p3) <= m(p1,p2)
	{
		if ((y - pfh1->y1) * (pfh1->x2 - pfh1->x1) - (x - pfh1->x1) * (pfh1->y2 - pfh1->y1) > 0) // m(p1, p) > m(p1, p2)
			return true;
	}
	else
	{
		if ((y - pfh1->y1) * (pfh1->x2 - pfh1->x1) - (x - pfh1->x1) * (pfh1->y2 - pfh1->y1) <= 0) // m(p1, p) <= m(p1, p2)
			return false;
	}
	if ((y - pfh2->y1) * (pfh2->x2 - pfh2->x1) - (x - pfh2->x1) * (pfh2->y2 - pfh2->y1) <= 0) // m(p2, p) <= m(p2, p3)
		return false;
	return true;
}

bool CPyhsics::CollisionDetectWalk(CRelPos p1, int &tElapse)
{
	int tElapseMax = BoundPosMapRange(p1, tElapse);
	if (p1.pos > 0.0 || m_rp.pos <= 0.0)
	{
		if (m_pfh->len <= p1.pos && m_pfh->len > m_rp.pos && m_pfh->next != NULL && m_pfh->next->prev != NULL && m_pfh->next->prev != m_pfh)
		{
			if (m_pfh->next->prev->uvx <= 0.0)
			{
				m_rp.pos = m_pfh->len;
				m_rp.v = 0.0;
			}
			else
			{
				m_rp.pos = m_pfh->next->prev->len;
				m_ap.SetFromRelPos(m_rp, m_pfh->next->prev);
				tElapseMax = 0;
				OnAttachedObjectChanged(m_pfh->next->prev, NULL, 0);
			}
		}
	}
	else if (m_pfh->prev != NULL && m_pfh->prev->next != NULL && m_pfh->prev->next != m_pfh)
	{
		if (m_pfh->prev->next->uvx <= 0.0)
		{
			m_rp.pos = 0.0;
			m_rp.v = 0.0;
		}
		else
		{
			m_rp.pos = 0.0;
			m_ap.SetFromRelPos(m_rp, m_pfh->prev->next);
			tElapseMax = 0;
			OnAttachedObjectChanged(m_pfh->prev->next, NULL, 0);
		}
	}

	if (m_rp.pos >= 0.0 && m_rp.pos <= m_pfh->len) // still on platform
	{
		m_ap.SetFromRelPos(m_rp, m_pfh);
		tElapse -= tElapseMax;
		MakeContinuousMovePath(tElapseMax);
		return false;
	}

	CRelPos p2 = m_rp;

	CFootHold* pfhNew;

	bool bBackward;
	double t;
	if (m_rp.pos >= 0.0)
	{
		bBackward = false;
		pfhNew = m_pfh->next;
		t = (m_pfh->len - p1.pos) / (m_rp.pos - p1.pos);
	}
	else
	{
		pfhNew = m_pfh->prev;
		bBackward = true;
		t = p1.pos / (p1.pos - m_rp.pos);
	}

	tElapse -= (int)(tElapseMax * t);
	int tCollide = (int)(tElapseMax * t);

	if (pfhNew == NULL) // there is no next, fall off
	{
		m_rp.pos = bBackward ? 0.0 : m_pfh->len;
		m_rp.v = (p2.v - p1.v) * t + p1.v;
		m_ap.SetFromRelPos(m_rp, m_pfh);
		OnAttachedObjectChanged(NULL, NULL, tCollide);
		return true;
	}

	if (pfhNew->uvx > 0.0) // not | foothold
	{
		m_rp.pos = bBackward ? pfhNew->len : 0.0;
		m_rp.v = (p2.v - p1.v) * t + p1.v;

		if (bBackward)
		{
			if (m_rp.v > 0.0)
				m_rp.v = 0.0;
		}
		else
		{
			if (m_rp.v < 0.0)
				m_rp.v = 0.0;
		}
		m_ap.SetFromRelPos(m_rp, pfhNew);
		OnAttachedObjectChanged(pfhNew, NULL, tCollide);
		return true;
	}

	if ((bBackward && pfhNew->uvy < 0.0) || (!bBackward && pfhNew->uvy > 0.0)) // fall off platform
	{
		m_rp.pos = bBackward ? 0.0 : m_pfh->len;
		m_rp.v = (p2.v - p1.v) * t + p1.v;
		m_ap.SetFromRelPos(m_rp, m_pfh);
		OnAttachedObjectChanged(NULL, NULL, tCollide);
		return true;
	}
	// move to next fh
	m_rp.pos = bBackward ? 0.0 : m_pfh->len;
	m_rp.v = 0.0;
	m_ap.SetFromRelPos(m_rp, m_pfh);
	MakeContinuousMovePath(tCollide);
	return true;

}


int CPyhsics::BoundPosMapRange(CAbsPos p, int &tElapse)
{
	int ret = tElapse;
	int oldx = (int)(m_ap.x - p.x);
	int oldy = (int)(m_ap.y - p.y);
	int x;
	if ( m_ap.x <  (x = Game->View->xmin) || m_ap.x > (x = Game->View->xmax) )
	{
		m_ap.x = x;
		m_ap.vx = 0.0;
		if ( oldx != 0.0 )
			ret = (int)((m_ap.x - p.x) * tElapse / oldx);
		if ( ret > tElapse ) ret = tElapse;
		else if (ret < 0) ret = 0;
	}

	int y;
	if ( m_ap.y <  (x = Game->View->ymin) || m_ap.y > (y = Game->View->ymax) )
	{
		m_ap.y = y;
		m_ap.vy = 0.0;
		if ( oldy != 0.0 )
		{
			int mret = (int)((m_ap.y - p.y) * tElapse / oldy);
			if ( mret <= 0 ) mret = 0;
			if ( mret < ret )
				ret = mret;
		}
	}
	return ret;
}

int CPyhsics::BoundPosMapRange(CRelPos p, int &tElapse)
{
	double pos = m_pfh->uvx * m_rp.pos + m_pfh->x1;

	if (pos >= Game->View->xmin && pos <= Game->View->xmax)
		return tElapse;

	m_rp.pos = ((pos >= Game->View->xmin) ? Game->View->xmax - m_pfh->x1 : Game->View->xmin - m_pfh->x1) / m_pfh->uvx;
	m_rp.v = 0.0;

	int ret = tElapse;
	double oldpos = m_rp.pos - p.pos;

	if (oldpos != 0.0)
		ret = (int)((m_rp.pos - p.pos) * tElapse / oldpos);
	if (ret > tElapse) ret = tElapse;
	else if (ret < 0) ret = 0;

	return ret;
}

bool CPyhsics::IsAbleToClimbLadderOrRope()
{
	return m_bLadderOrRopeNext;
}

void CPyhsics::WorkUpdateActiveLadderOrRope()
{
	if (m_bJumpNext)
		Jump();
	//if ( m_impactNext.bValid )
	//	Impact();
	if (m_pLadderOrRope != NULL)
	{
		m_ap.y += m_nInputY * (30 / 10);

		if (m_nInputY < 0)
		{
			if (m_ap.y < m_pLadderOrRope->y1)
			{
				if (m_pLadderOrRope->uf)
				{
					m_ap.y = m_pLadderOrRope->y1 - 0.5 * 10;
					OnAttachedObjectChanged(NULL, NULL, 0);
				}
				else
				{
					m_ap.y = m_pLadderOrRope->y1;
				}
			}
		}
		else if (m_nInputY > 0)
		{
			if (m_ap.y > m_pLadderOrRope->y2)
			{
				m_ap.y = m_pLadderOrRope->y2 + 1;
				OnAttachedObjectChanged(NULL, NULL, 0);
			}
		}
	}
	MakeContinuousMovePath(30);
}


void CPyhsics::JumpDown()
{
	if (m_pLadderOrRope == NULL)
	{
		OnAttachedObjectChanged(NULL, NULL, 0);
	}
	else
	{
		if (m_pfh == NULL) return;
		DetachFromFoothold();
	}
	m_bJumpDownNext = false;
	m_ap.vy = jumpSpeed * 0.35355339;
	m_ap.vx = 0;
}

void CPyhsics::Jump()
{
	bool bFloat = Game->Map->fly < 0.0 || m_pCurAttrShoe.flyAcc > 0.0;

	m_bJumpNext = false;

	if (m_pLadderOrRope == NULL)
	{
		CFootHold* fh = m_pfh;
		if (fh != NULL)
		{
			DetachFromFoothold();
			m_ap.vy = m_pCurAttrShoe.walkJump * -jumpSpeed;
			if (bFloat) m_ap.vy *= 0.7;
			if (m_nInputX != 0)
			{
				double maxSpeed = max_walk_speed(fh, m_pCurAttrShoe, (fh->uvy * m_nInputX >= 0.0) ? 1 : -1);

				if (maxSpeed * 0.8 > m_nInputX * m_ap.vx)
					m_ap.vx = m_nInputX * maxSpeed * 0.8 + m_ap.vx;
				if (m_nInputX * m_ap.vx > maxSpeed)
					m_ap.vx = m_nInputX * maxSpeed;
			}
		}
		else if (bFloat)
		{
			m_ap.vy = -((Game->Map->fly < 0) ? m_pCurAttrShoe.swimSpeedV * abs(Game->Map->fly) * 0.7 * 1000 : m_pCurAttrShoe.flySpeed * abs(Game->Map->fly) * flySpeed);
		}
	}
	else if (m_nInputX != 0)
	{
		OnAttachedObjectChanged(NULL, NULL, 0);
		m_ap.vy = m_pCurAttrShoe.walkJump * -jumpSpeed / (bFloat ? 0.3 : 0.5);
		m_ap.vx = m_nInputX * m_pCurAttrShoe.walkSpeed * 162.5;
	}
}

void CPyhsics::CalcWalk(int tElapse)
{
	double mass = m_pCurAttrShoe.mass;

	double vSlip = abs(m_pfh->uvy);
	double vMaxL = vSlip * vSlip;

	int hd = (m_pfh->uvy < 0.0) ? 1 : -1;

	double v = m_rp.v;

	bool isb = /*(m_pfh->drag == 1.0) && */(m_pfh->force == 0.0);

	double vMaxH = m_pfh->bWalk * (isb ? m_pCurAttrShoe.walkAcc : 1.0)  * walkForce;

	double f = m_nInputX * vMaxH;

	if ( Game->Map->fly < 0.0 ) {
		f *= swimSpeedDec;
	}

	double footholdforce = m_pfh->force;

	if ( footholdforce == 0.0 )
	{
		if ( m_nInputX * f <= 0.0 )
			f = 0.0;
	}
	else
	{
		if ( m_nInputX != 0 )
			f *= ((m_nInputX * footholdforce > 0.0) ? 2 * abs(footholdforce) : 0.2 / abs(footholdforce));
		else
			f = footholdforce * vMaxH;
	}

	f *= ((hd > 0) ?  (1.0 - vMaxL) : (1.0 + vMaxL));

	double vMax = m_pfh->bWalk * m_pCurAttrShoe.walkSpeed * walkSpeed;

	
	if ( Game->Map->fly < 0.0 )
		vMax *= swimSpeedDec;

	if ( footholdforce != 0.0 )
	{
		vMax *= ((m_nInputX != 0) ? ((m_nInputX * footholdforce < 0.0) ? 0.2 / abs(footholdforce) : 2 * abs(footholdforce)) : abs(footholdforce));
	}

	double drag = (isb ? m_pCurAttrShoe.walkDrag : 1.0) /* m_pfh->drag*/;
	if (drag > maxFriction)
		drag = maxFriction;
	if (drag < minFriction)
		drag = minFriction;
	if ( drag < 1.0 )
		drag *= 0.5;

	double fSlip = drag * walkDrag;
	double drag2 = (drag == 0.0) ? walkDrag / 5 : fSlip;

	double tSec = tElapse * 0.001;
	if ( vSlip == 0.0 )
	{
		if ( vMax >= 0.0 )
		{
			if ( v < -vMax)
			{
				v += drag2 / mass * tSec;
				if(v > -vMax)
					v = -vMax;
			}

			else if(v > vMax)
			{
				v -= drag2 / mass * tSec;
				if ( v < vMax )
					v = vMax;
			}
		}
		if ( m_nInputX == 0 && footholdforce == 0.0 )
		{
			if ( v < 0.0 )
			{
				v += fSlip / mass * tSec;
				if ( v > 0.0 )
					v = 0.0;
			}
			else if (v > 0.0)
			{
				v -= fSlip / mass * tSec;
				if ( v < 0.0 )
					v = 0.0;
			}
		}
		else
		{
			AccSpeed( v, f, mass, vMax, tSec); 
		}
	}
	else
	{
		vMaxH = vMax;
		if ( vMax < 0.0 )
			vMaxH = 0.0;

		vMaxL = (vMaxL + 1.0) * vMax;

		if ( vMaxL < 0.0 )
			vMaxL = 0.0;

		vMax = (double)hd;

		if ( vMax * v > 0.0 )
			vMaxL = vMaxH;

		if ( vMaxL >= 0.0 )
		{
			if ( v > vMaxL )
			{
				v -= drag2 / mass * tSec;
				if ( v < vMaxL )
					v = vMaxL;
			}
			else if( v < -vMaxL )
			{
				v += drag2 / mass * tSec;
				if ( v > -vMaxL )
				{
					v = -vMaxL;
				}
			}
		}

		if ( m_pCurAttrShoe.walkSlant < vSlip )
		{
			fSlip = vSlip * slipForce * -hd;
			vSlip = vSlip * slipSpeed;
			if ( hd * m_nInputX <= 0 )
			{
				if ( m_nInputX != 0.0 || footholdforce != 0.0 )
				{
					fSlip += f;
					vSlip += vMaxL;
				}
			}
			else
			{
				fSlip *= 0.5;
				vSlip *= 0.5;
			}
			if ( vMax * v > 0.0 )
			{
				if ( v < 0.0 )
				{
					v += drag2 / mass * tSec;
					if(v > 0.0)
						v = 0.0;
				}
				else if ( v > 0.0)
				{
					v -= drag2 / mass * tSec;
					if ( v < 0.0 )
						v = 0.0;
				}
			}
			AccSpeed( v, fSlip, mass, vSlip, tSec); 
		}
		else
		{
			if ( drag == 0.0 )
				f -= vSlip * slipForce * vMax;
			if ( m_nInputX != 0 || drag == 0.0 || footholdforce != 0.0 )
			{
				AccSpeed( v, f, mass, (vMax * f > 0.0) ? vMaxH : vMaxL , tSec);
			}
			else
			{
				if ( v < 0.0 )
				{
					v += fSlip / mass * tSec;
					if ( v > 0.0 )
						v = 0.0;
				}
				else if (v > 0.0)
				{
					v -= fSlip / mass * tSec;
					if ( v < 0.0 )
						v = 0.0;
				}
			}
		}
	}

	m_rp.pos = (v + m_rp.v) * tSec * 0.5 + m_rp.pos;
	m_rp.v = v;
}

void CPyhsics::AccSpeed( double &v, double f, double m, double vMax, double tSec)
{
	if (vMax >= 0.0)
	{
		if (f <= 0.0)
		{
			if (v > -vMax)
			{
				v += f / m * tSec;
				if (v < -vMax)
					v = -vMax;
			}
		}
		else
		{
			if (v < vMax)
			{
				v += f / m * tSec;
				if (v > vMax)
				{
					v = vMax;
				}
			}
		}
	}
}


double CPyhsics::max_walk_speed(CFootHold* pfh, CAttrShoe pas, int dir) {
	double result;

	result = pas.walkSpeed * walkSpeed;
	if (dir != 0)
		result = (pfh->uvy * pfh->uvy * (double)dir + 1.0) * result;
	return result;
}

void CPyhsics::DetachFromFoothold()
{
	if (m_pfh != NULL)
	{
		if (m_rp.pos > m_pfh->len)
			m_rp.pos = m_pfh->len;
		else if (m_rp.pos < 0.0)
			m_rp.pos = 0.0;

		m_ap.SetFromRelPos(m_rp, m_pfh);

		if (m_ap.x < m_pfh->x1)
			m_ap.x = m_pfh->x1;
		else if (m_ap.x > m_pfh->x2)
			m_ap.x = m_pfh->x2;

		m_ap.y = floor(m_ap.y - 1.0);

		OnAttachedObjectChanged(NULL, NULL, 0);
	}
}

void CPyhsics::OnAttachedObjectChanged(CFootHold* pfhNew, CLadder* pLadderOrRopeNew, int tElapse)
{
	MakeContinuousMovePath(tElapse);
	int newPage = m_lPage, newMass = m_lZMass;
	if (pfhNew != NULL)
	{
		newPage = pfhNew->layer;
		newMass = pfhNew->group;
		m_pfhLast = pfhNew;
	}
	else if (pLadderOrRopeNew != NULL)
	{
		newPage = pLadderOrRopeNew->page;
		newMass = 0;
	}
	if (newPage != m_lPage || newMass != m_lZMass)
	{
		m_lPage = newPage;
		m_lZMass = newMass;
		m_lZMass_Last = newMass;
	}
	m_pLadderOrRope = pLadderOrRopeNew;
	m_pfh = pfhNew;
}

void CPyhsics::MakeContinuousMovePath(int tElapse)
{

}