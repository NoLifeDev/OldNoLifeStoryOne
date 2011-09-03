////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::Console::Console() {
	sf::Thread t([&](){this->Loop();});
	t.Launch();
}

void NLS::Console::Loop() {
	window = new sf::RenderWindow(sf::VideoMode(400,400,32), "NoLifeStory::FancyConsole", sf::Style::Resize, sf::ContextSettings(0, 0, 0, 2, 0));
#ifdef NLS_WINDOWS
	HANDLE hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_NOLIFESTORY_ICON));
	HWND hWnd = window->GetSystemHandle();
	if (hWnd) {
		::SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
	}
#endif
	font = new sf::Font();
	font->LoadFromFile("font.ttf");
	pos = 0;
	while (true) {
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
					pos = 0;
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
		sf::Text t(string("USER: ")+str, *font, 8);
		t.SetPosition(0, window->GetHeight()-10);
		window->Draw(t);
		for (int i = 0; i < strs.size(); i++) {
			sf::Text t(string("USER: ")+strs[strs.size()-i-1], *font, 8);
			t.SetPosition(0, window->GetHeight()-12*i-24);
			window->Draw(t);
		}
		sf::Shape s = sf::Shape::Line(0, window->GetHeight()-12, window->GetWidth(), window->GetHeight()-12, 1, sf::Color::White);
		window->Draw(s);
		window->Display();
		sf::Sleep(max(0.01-window->GetFrameTime(), 0.));
	}
}

void NLS::Console::HandleCommand(string str) {
	istringstream iss(str);
	vector<string> command;
	copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string>>(command));
	if (command.size() == 0) {
		return;
	}
	//Do stuff :D
}

NLS::Console::~Console() {
	delete font;
	delete window;
}