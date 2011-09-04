////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::Console* NLS::console;

NLS::Endl_ NLS::Endl;

NLS::Console::Console() {
	shutdown = false;
	toggle = false;
	show = true;
	t = new sf::Thread([&](){this->Loop();});
	t->Launch();
}

void NLS::Console::Loop() {
	window = new sf::RenderWindow(sf::VideoMode(400,400,32), "NoLifeStory::FancyConsole", sf::Style::Resize|sf::Style::Close, sf::ContextSettings(0, 0, 0, 2, 0));
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
	while (!shutdown) {
		if (toggle) {
			window->Show(show);
			toggle = false;
		}
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
					m.Lock();
					strs.push_back(str);
					m.Unlock();
					str.clear();
					pos = 0;
					break;
				case sf::Key::Tilde:
					Toggle();
					break;
				}
				break;
			case sf::Event::TextEntered:
				if (e.Text.Unicode != 13 and e.Text.Unicode != 8) {
					str.insert(pos++, 1, e.Text.Unicode);
				}
				break;
			case sf::Event::Closed:
				show = false;
				toggle = true;
				break;
			case sf::Event::Resized:
				break;
			}
		}
		window->Clear();
		sf::Text t(str, *font, 8);
		t.SetPosition(0, window->GetHeight()-10);
		window->Draw(t);
		m.Lock();
		for (int i = 0; i < strs.size(); i++) {
			sf::Text t(strs[strs.size()-i-1], *font, 8);
			t.SetPosition(0, window->GetHeight()-12*i-24);
			window->Draw(t);
		}
		m.Unlock();
		sf::Shape s = sf::Shape::Line(0, window->GetHeight()-12, window->GetWidth(), window->GetHeight()-12, 1, sf::Color::White);
		window->Draw(s);
		window->Display();
		sf::Sleep(max(0.01-window->GetFrameTime(), 0.));
	}
	delete font;
	delete window;
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

void NLS::Console::Push(string str) {
	m.Lock();
	strs.push_back(str);
	m.Unlock();
}

void NLS::Console::Toggle() {
	show != show;
	toggle = true;
}

NLS::Console::~Console() {
	shutdown = true;
	t->Wait();
	delete t;
}