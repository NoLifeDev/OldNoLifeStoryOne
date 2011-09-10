////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	namespace Map {
		void Load(const string& id, const string& portal);
		void Load();
		void Draw();
		extern Node node;
		extern string nextmap;
		extern string nextportal;
		class Layer {
		public:
			void Draw();
			vector<Obj> Objs;
			vector<Tile> Tiles;
		};
		extern Layer Layers[5];
	};
};