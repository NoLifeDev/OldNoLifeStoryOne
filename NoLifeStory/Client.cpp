////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Init(const vector<string>& args) {
	C("INFO") << "Initializing NoLifeStory" << endl;
	Network::Init();
	Time.Reset();
	WZ::Init("C:/Nexon/MapleStory/");
	Time.Step();
	C("WZ") << "Directories initialized in " << floor(Time.tdelta) << " ms" << endl;
	Time.Reset();
	Graphics::Init();
	Map::Load("10000", "");
	Map::Load();
}

bool NLS::Loop() {
	Time.Step();
	static double fps(0);
	fps = fps*0.99 + (1/max(Time.delta, 1))*1000*0.01;
	window->SetTitle("NoLifeStory::FrameRate = "+tostring((int)fps));
	sf::Sleep(max(20-1000/fps, 0));
	sf::Event e;
	while (window->PollEvent(e)) {
		switch (e.Type) {
		case sf::Event::KeyPressed:
			switch (e.Key.Code) {
			case sf::Keyboard::Tilde:
				console->Toggle();
				break;
			case sf::Keyboard::Escape:
				return false;
			}
			break;
		case sf::Event::Closed:
			return false;
			break;
		}
	}
	Graphics::Draw();
	return true;
}

void NLS::Unload() {
}