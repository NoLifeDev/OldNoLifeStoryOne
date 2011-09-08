////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	class SpriteData {
	public:
		GLuint texture;
		GLuint width, height;
		GLfloat twidth, theight;
		GLuint originx, originy;
		bool loaded;
		class WZ::PNGProperty* png;
	};
	class Sprite {
	public:
		void Draw(int x, int y, bool flipped = false, float alpha = 1, float rotation = 0);
	private:
		void GetTexture();
		SpriteData* data;
	};
};