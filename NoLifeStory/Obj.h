////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	class Obj {
	public:
		void Draw();
		static void Load(Node n);
		int x, y, z;
		bool repeat;
		int movetype;
		double movew, moveh, movep, mover;
		int flow;
		double rx, ry;
		bool f;
		AniSprite spr;
		Node n;
		static set <Obj*> Objs;
	};
};