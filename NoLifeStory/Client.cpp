////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Init(vector<string> args) {
	Network::Init();
	Time.Reset();
	WZ::Init(args[1]);
	Time.Step();
	cout << "INFO: WZ Data initialized in " << floor(Time.tdelta*1000) << " ms" << endl;
	Time.Reset();
	Graphics::Init();
	Map::Load("10000", "");
	Map::Load();
}

bool NLS::Loop() {
	Time.Step();
	static double fps(0);
	fps = fps*0.99 + (1/max(Time.delta, 0.001))*0.01;
	window->SetTitle("NoLifeStory::FrameRate = "+tostring((int)fps));
	if (Time.delta < 0.01) {
		sf::Sleep(0.01-Time.delta);
	}
	sf::Event e;
	while (window->GetEvent(e)) {
		switch (e.Type) {
		case sf::Event::KeyPressed:
			switch (e.Key.Code) {
			case sf::Key::Tilde:
				console->Toggle();
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