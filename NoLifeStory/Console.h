////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	class Console {
	public:
		Console();
		~Console();
		void Loop();
		void HandleCommand(string command);
	private:
		sf::RenderWindow* window;
		string str;
		int32_t pos;
		vector <string> strs;
		sf::Font* font;
		bool shutdown;
		sf::Thread* t;
	};
};