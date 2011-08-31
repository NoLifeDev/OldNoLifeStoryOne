////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Init(vector<string> args) {
	Network::Init();
	Time.Reset();
	WZ::Init("C:\\Nexon\\MapleStory\\");
	Time.Step();
	cout << "Initializing WZ data took: " << Time.tdelta << " seconds!" << endl;
	Time.Reset();
	Graphics::Init();
	Map::Load("100000000", "");
	Map::Load();
}

bool NLS::Loop() {
	Time.Step();
	sf::Event e;
	while (window->GetEvent(e)) {
		//Wheeeeeeeeeee
	}
	Graphics::Draw();
	return true;
}

void NLS::Unload() {

}