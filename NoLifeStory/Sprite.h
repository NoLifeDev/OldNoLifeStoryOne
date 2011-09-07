////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	class SpriteData {
	public:
		GLuint texture;
	};
	class Sprite {
	public:
		void Draw(int x, int y);
		void Draw(int x, int y, double rotation);
	private:
		SpriteData* data;
	};
};