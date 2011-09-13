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
		GLint originx, originy;
		bool loaded;
		void* png;
	};
	class Sprite {
	public:
		Sprite();
		void Draw(int x, int y, bool flipped = false, float alpha = 1, float rotation = 0);
		void GetTexture();
		SpriteData* data;
	};
};