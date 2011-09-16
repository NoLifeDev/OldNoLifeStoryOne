////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

void NLS::Init(const vector<string>& args) {
	C("INFO") << "Initializing NoLifeStory" << endl;
	Network::Init();
	Time.Reset();
	WZ::Init(args.size()>1?args[1]:"");
	Time.Step();
	Graphics::Init();
	Map::Load("10000", "");
	Map::Load();
}

bool NLS::Loop() {
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
	if (!Map::nextmap.empty()) {
		Map::Load();
	}
	return true;
}

void NLS::Unload() {
}