////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
namespace NLS {
	class View_ {
	public:
		int x, y;
		double vx, vy;
		double tx, ty;
		int xmin, xmax, ymin, ymax;
		void Step();
		View_();
	} extern View;
};