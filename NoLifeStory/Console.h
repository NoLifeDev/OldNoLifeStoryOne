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
		u32string str;
		int32_t pos;
		vector <u32string> strs;
		sf::Font* font;
	};
};