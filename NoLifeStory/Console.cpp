////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::Console::Console() {
	window = new sf::RenderWindow(sf::VideoMode(400,400,32), "NoLifeStory::FancyConsole", sf::Style::Resize, sf::ContextSettings(0, 0, 0, 2, 0));
	font = new sf::Font();
	font->LoadFromFile("font.ttf");
	pos = 0;
}

void NLS::Console::Loop() {
	window->SetActive(true);
	sf::Event e;
	while (window->GetEvent(e)) {
		switch (e.Type) {
		case sf::Event::KeyPressed:
			switch (e.Key.Code) {
			case sf::Key::Back:
				if (pos != 0) {
					str.erase(--pos, 1);
				}
				break;
			case sf::Key::Delete:
				if (pos != str.size()) {
					str.erase(pos, 1);
				}
				break;
			case sf::Key::Left:
				pos = max<int32_t>(0, pos-1);
				break;
			case sf::Key::Right:
				pos = min<int32_t>(str.size(), pos+1);
				break;
			case sf::Key::Return:
				HandleCommand(str);
				strs.push_back(str);
				str.clear();
				break;
			}
			break;
		case sf::Event::TextEntered:
			if (e.Text.Unicode != 13 and e.Text.Unicode != 8) {
				str.insert(pos++, 1, e.Text.Unicode);
			}
			break;
		}
	}
	window->Clear();
	sf::Text t(sf::String('>')+str, *font, 16);
	t.SetColor(sf::Color::White);
	t.SetPosition(0, window->GetHeight()-20);
	window->Draw(t);
	for (int i = strs.size()-1; i >= 0; i--) {

	}
	window->Display();
	window->SetActive(false);
}

void NLS::Console::HandleCommand(string str) {
	istringstream iss(str);
	vector<string> command;
	copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string>>(words));
	if (command.size() == 0) {
		return;
	}
	//Do stuff :D
}

NLS::Console::~Console() {
	delete font;
	delete window;
}