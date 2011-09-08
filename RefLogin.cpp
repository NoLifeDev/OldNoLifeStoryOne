#include "LoginDialogs.h"
#include "Game.h"
#include "MapleClient.h"
#include "World.h"
#include "MapleClient/Player.h"
#include "PacketCreator.h"
#include "SkillConstants.h"

#include <map>

std::map<int,CWorld*> Worlds;
std::vector<CLoginChar*> Players;

void CLoginDialogs::addWorld(int i,CWorld* w) {
	Worlds[i] = w;
}

void doLogin(CCtrl* ctrl) {
	CCtrlEdit* e = (CCtrlEdit*)ctrl->hWnd->getCtrl(EDIT_USERNAME);
	if(e->szStr.length() < 4) {
		return;
	}
	char* szUser = (char*)e->szStr.c_str();

	e = (CCtrlEdit*)ctrl->hWnd->getCtrl(EDIT_PASSWORD);
	if(e->szStr.length() < 4) {
		return;
	}
	char* szPass = (char*)e->szStr.c_str();

	MapleClient->Login(szUser,szPass);
}

void selectWorld(CCtrl* ctrl) {
	CWorld* w = (CWorld*)ctrl->lParam;
	CLoginDialogs::ShowWorld(w);
}

void showCharInfo(CCtrl* ctrl) {
	CLoginChar* c = (CLoginChar*)ctrl->lParam;

	if(Game->Input->getCtrl() == c)
		return;

	CWnd* w = (CWnd*)Game->getCtrl(D_PLAYER_SELECT);
	Game->removeCtrl(D_PLAYER_SELECT_SCROLL);

	c->p->state = "walk1";
	CCtrlStatic* s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("CharSelect")->get("effect")->get("0"),c->x,c->y-340,TRUE);
	s->repeat = 1;
	w->addCtrl(PLAYER_SELECT_EFFECT_1,s);
	s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("CharSelect")->get("effect")->get("1"),c->x,c->y-340,TRUE);
	w->addCtrl(PLAYER_SELECT_EFFECT_2,s);
	
	w = new CWnd(WZ->get("UI")->get("Login")->get("CharSelect")->get("scroll")->get(c->bRank ? "2" : "0"),c->x+100,c->y-270,TRUE);
	w->z[1] = 2;

	s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("CharSelect")->get(c->bRank ? "charInfo1" : "charInfo")->data->spriteValue,-160,80,TRUE);
	s->z[1] = 3;
	w->addCtrl(SCROLL_CHAR_INFO,s);
	
	UINT jP = JOB_PREFIX(c->p->job % 1000);
	if(jP == 5)
		jP = 4;
	CNode* n = WZ->get("UI")->get("Login")->get("CharSelect")->get("icon")->get("job")->get(jP);
	if(n)
		w->addCtrl(SCROLL_JOB,new CCtrlStatic(n->data->spriteValue,-150,143,TRUE));
	
	w->addText(Jobs::JobIds::getJobName(c->p->job),-160,27);
	char szStr[MAX_PATH];
	sprintf(szStr,"%i",c->p->level);
	w->addText(szStr,-160,45);
	sprintf(szStr,"%i",c->p->pop);
	w->addText(szStr,-74,45);
	sprintf(szStr,"%i",c->p->str);
	w->addText(szStr,-160,63);
	sprintf(szStr,"%i",c->p->dex);
	w->addText(szStr,-74,63);
	sprintf(szStr,"%i",c->p->_int);
	w->addText(szStr,-160,81);
	sprintf(szStr,"%i",c->p->luk);
	w->addText(szStr,-74,81);
	if(c->bRank) {
		sprintf(szStr,"Ranked at %i",c->r1);
		w->addText(szStr,-170,123);
		sprintf(szStr,"Ranked at %i",c->r3);
		w->addText(szStr,-170,160);
	}

	Game->addCtrl(D_PLAYER_SELECT_SCROLL,w);
	
}

void selectChar(CCtrl* ctrl) {
	CLoginChar* c = NULL;
	for(UINT i = 0; i < Players.size();i++) {
		if(Players[i]->bFocus) 
			c = Players[i];
	}
	if(c == NULL)
		return;
	MapleClient->sendPacket(PacketCreator::selectCharacter("000000",c->p->UID,FALSE));
}

void selectCH(CCtrl* ctrl) {
	UINT w = (UINT)ctrl->lParam /100;
	UINT ch = (UINT)ctrl->lParam % 100;
	/*CLoginDialogs::cleanWorlds();
	Game->Map->ChangeLoginStep(PLAYER_SELECT);
	*/MapleClient->RequestCharList(w,ch);
}

void doQuit(CCtrl* ctrl) {
	Game->Quit();
}

void doBack(CCtrl* ctrl) {
	MapleClient->bReconnect = TRUE;
	Game->removeCtrl(D_CH_SELECT);
	CLoginDialogs::cleanWorlds();
	Game->Map->ChangeLoginStep(LOGIN_SCREEN);
}

void CLoginDialogs::ShowWorld(CWorld* w) {
	Game->removeCtrl(D_CH_SELECT);
	CWnd* wnd = new CWnd(WZ->get("UI")->get("Login")->get("WorldSelect")->get("chBackgrn")->data->spriteValue,-159,-718,TRUE);
	wnd->z[0] = layer;
	wnd->z = 1;
	wnd->z = obj;
	wnd->z = 8;
	wnd->z = 8;
	wnd->addCtrl(STATIC_WORLD_NAME,new CCtrlStatic(WZ->get("UI")->get("Login")->get("WorldSelect")->get("world")->get(w->ID)->data->spriteValue,9,33,TRUE));
	wnd->addCtrl(BTN_MOVE_TO_CH,new CCtrlBtn(WZ->get("UI")->get("Login")->get("WorldSelect")->get("BtGoworld"),238,48,TRUE));

	for(UINT i = 0; i < 20;i++) {
		UINT x = i % 5;
		UINT y = i / 5;
		CCtrlBtn* b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("WorldSelect")->get("channel")->get(i),24+(x*66),93+(y*29),TRUE); //View Recomended
		if(i+1 > w->maxCH) {
			b->Disable();
			b->alpha = 0.3;
		} else {
			b->clickHandler = selectCH;
			b->lParam = (LPVOID)((w->ID * 100) + i);
			if(i == 13)
				b->setFocus();
		}
		wnd->addCtrl(BTN_CH_1+i,b); 
	}

	Game->addCtrl(D_CH_SELECT,wnd);
}

void CLoginDialogs::createPic(ControlHandler* h) {
	CCtrl* w = Game->getCtrl(D_PIC);
	if(w) {
		
	} else {

	}

}

void CLoginDialogs::Update(char step) {
	if(step == LOGIN_SCREEN) {
		Game->removeCtrl(D_LOGIN_BTNS);

		CCtrl* c = Game->getCtrl(D_LOGIN);
		if(c) {
			c->bDraw = TRUE;
			return;
		}
		CCtrlStatic* s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("Common")->get("frame")->data->spriteValue,400,300,FALSE);
		s->z[1] = 0;
		
		Game->addCtrl(D_LOGIN_FRAME,s);

		CWnd* loginWnd = new CWnd(WZ->get("UI")->get("Login")->get("Title")->get("signboard")->data->spriteValue,22,17,TRUE);

		CCtrlEdit* e = new CCtrlEdit(WZ->get("UI")->get("Login")->get("Title")->get("ID")->data->spriteValue,-100,-51,TRUE);
		e->clickHandler = doLogin;
		e->setText("Username");
		loginWnd->addCtrl(EDIT_USERNAME,e);
		e->setFocus();
		e->bHide = TRUE;
		e = new CCtrlEdit(WZ->get("UI")->get("Login")->get("Title")->get("PW")->data->spriteValue,-100,-26,TRUE);
		e->bPassword = TRUE;
		e->clickHandler = doLogin;
		e->setText("Password");
		e->bHide = TRUE;
		loginWnd->addCtrl(EDIT_PASSWORD,e);

		loginWnd->addCtrl(CB_REMEMBER,new CCtrlCB(WZ->get("UI")->get("Login")->get("Title")->get("check"),-100,4,TRUE,FALSE));

		CCtrlBtn* b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtLoginIDSave"),-90,0,TRUE);
		loginWnd->addCtrl(BTN_ID_SAVE,b);
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtLoginIDLost"),-14,0,TRUE);
		loginWnd->addCtrl(BTN_ID_LOST,b);
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtPasswdLost"),55,2,TRUE);
		loginWnd->addCtrl(BTN_PASS_LOST,b);

		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtNew"),-100,20,TRUE);
		loginWnd->addCtrl(BTN_NEW,b);
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtHomePage"),-27,20,TRUE);
		loginWnd->addCtrl(BTN_HOME,b);
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtQuit"),45,20,TRUE);
		b->clickHandler = doQuit;
		loginWnd->addCtrl(BTN_QUIT,b);

		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Title")->get("BtLogin"),65,-51,TRUE);
		b->clickHandler = doLogin;
		loginWnd->addCtrl(BTN_LOGIN,b);

		Game->addCtrl(D_LOGIN,loginWnd);
	} else if(step == WORLD_SELECT) {
		Game->getCtrl(D_LOGIN)->bDraw = FALSE;

		Game->removeCtrl(D_LOGIN_BTNS);
		CWnd* wsWnd = new CWnd();

		CCtrlBtn* b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("WorldSelect")->get("BtViewChoice"),0,52,FALSE); //View Recomended
		wsWnd->addCtrl(BTN_VIEW_RECOMENDED,b);

		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("ViewAllChar")->get("BtVAC"),0,245,FALSE); //View All Char
		wsWnd->addCtrl(BTN_VAC,b);
			
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Common")->get("BtStart"),0,545,FALSE); //Back
		b->clickHandler = doBack;
		wsWnd->addCtrl(BTN_BACK,b);
			
		Game->addCtrl(D_LOGIN_BTNS,wsWnd);

		//-------------------------------//
		Game->removeCtrl(D_WORLD_SELECT);
		
		wsWnd = new CWnd(122-372,46-908);

		UINT j = Worlds.size()-1;
		for(UINT i = 0; i < 36;j--,i++) {
			CWorld* w = Worlds[j];
			if(w) {
				UINT x = i % 6;
				UINT y = i / 6;
				CCtrlBtn* b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("WorldSelect")->get("BtWorld")->get(w->ID),(x*96),(y*26),TRUE); //View Recomended
				b->lParam = (LPVOID)w;
				b->clickHandler = selectWorld;
				wsWnd->addCtrl(BTN_WORLD_1+i,b);
				if(w->bFocus) {
					
				}
				if(w->e > 0) {
					CCtrlStatic* s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("WorldNotice")->get(w->e),(x*96)+86,(y*26)-2,TRUE);
					s->z[1] = 2;
					wsWnd->addCtrl(BTN_WORLD_EVENT_1+i,s);
				}
			} else {
				UINT x = i % 6;
				UINT y = i / 6;
				CCtrlBtn* b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("WorldSelect")->get("BtWorld")->get("e"),(x*96),(y*26),TRUE); //View Recomended
				b->Disable();
				wsWnd->addCtrl(BTN_WORLD_1+i,b);
			}
		}

		Game->addCtrl(D_WORLD_SELECT,wsWnd);
	} else if(step == PLAYER_SELECT) {
		CLoginDialogs::cleanWorlds();

		Game->removeCtrl(D_LOGIN_BTNS);
		CWnd* wsWnd = new CWnd();

		CCtrlBtn* b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("ViewAllChar")->get("BtVAC"),0,245,FALSE); //View All Char
		b->Disable();
		wsWnd->addCtrl(BTN_VAC,b);

		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("Common")->get("BtStart"),0,545,FALSE); //Back
		b->clickHandler = doBack;
		wsWnd->addCtrl(BTN_BACK,b);

		Game->addCtrl(D_LOGIN_BTNS,wsWnd);

		wsWnd = new CWnd();

		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("CharSelect")->get("BtSelect"),-372+632,-1508+148,TRUE);
		b->clickHandler = selectChar;
		b->setFocus();
		wsWnd->addCtrl(BTN_SELECT,b);
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("CharSelect")->get("BtNew"),-372+632,-1508+185,TRUE);
		wsWnd->addCtrl(BTN_P_NEW,b);
		b = new CCtrlBtn(WZ->get("UI")->get("Login")->get("CharSelect")->get("BtDelete"),-372+632,-1508+234,TRUE);
		wsWnd->addCtrl(BTN_DELETE,b);

		CCtrlStatic* s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("CharSelect")->get("pageL")->get("0")->get("0")->data->spriteValue,-372+155,-1508+331,TRUE);
		wsWnd->addCtrl(PAGE_L,s);
		s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("CharSelect")->get("pageR")->get("0")->get("0")->data->spriteValue,-372+605,-1508+331,TRUE);
		wsWnd->addCtrl(PAGE_R,s);

		Game->addCtrl(D_PLAYER_SELECT,wsWnd);

		for(UINT i = 0; i < Players.size();i++) {
			CLoginChar* c = Players[i];
			CPlayer* p = c->p;
			c->lParam = c;
			c->bRelative = TRUE;
			c->clickHandler = showCharInfo;
			c->dblClickHandler = selectChar;
			
			p->x = c->x = -372+250 + (i * 130);
			p->y = c->y = -1508+370;
			
			char* szType = "adventure";
			if(IS_CIGNUS(p->job))
				szType = "knight";
			else if(IS_ARAN(p->job))
				szType = "aran";
			else if(IS_EVAN(p->job))
				szType = "evan";
			else if(IS_RESISTANCE(p->job))
				szType = "resistance";
			s = new CCtrlStatic(WZ->get("UI")->get("Login")->get("CharSelect")->get(szType),p->x-30,p->y,TRUE);
			wsWnd->addCtrl(PLAYER_FLAG_1+i,s);

			wsWnd->addCtrl(PLAYER_1+i,c);
			if(i == 2)
				showCharInfo(c);
		}
	}
}

void CLoginDialogs::cleanWorlds() {
	for(auto i = Worlds.begin();i != Worlds.end();i++) {
		delete i->second;
	}
	Worlds.clear();
}

void CLoginDialogs::addPlayer(CLoginChar* p) {
	p->z[1] = 2;
	Players.push_back(p);
}

void CLoginDialogs::Clear() {
	Game->removeCtrl(D_LOGIN_FRAME,FALSE);
	Game->removeCtrl(D_LOGIN,FALSE);
	Game->removeCtrl(D_LOGIN_BTNS,FALSE);
	Game->removeCtrl(D_WORLD_SELECT,FALSE);
	Game->removeCtrl(D_CH_SELECT,FALSE);
	Game->removeCtrl(D_PLAYER_SELECT,FALSE);
	Game->removeCtrl(D_PLAYER_SELECT_SCROLL,FALSE);
	cleanWorlds();
	Game->Input->setCtrl(NULL);
	for(UINT i = 0; i < Players.size();i++) {
		delete Players[i];
	}
	Players.clear();
}