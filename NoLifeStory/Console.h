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
		void HandleCommand(const string& command);
		void Push(const string& str);
		void Toggle();
	private:
		sf::RenderWindow* window;
		string str;
		int32_t pos;
		vector <string> strs;
		sf::Font* font;
		bool shutdown;
		sf::Thread* t;
		sf::Mutex m;
		bool toggle, show;
	};
	extern Console* console;
	class Endl_ {} extern Endl;
	class Stream {
	public:
		template <class T>
		Stream& operator << (T v) {
			line << v;
			return *this;
		}
		template <>
		Stream& operator << <Endl_> (Endl_) {
			string s = line.str();
			if (!s.empty()) {
				console->Push(s);
				line.str("");
			}
			return *this;
		}
	private:
		stringstream line;
	};
	inline Stream& C(const string& type) {
		static Stream s;
		s << Endl;
		s << type << ": ";
		return s;
	}
};