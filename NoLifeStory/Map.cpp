////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::Node NLS::Map::node;
string NLS::Map::nextmap;
string NLS::Map::nextportal;
NLS::Map::Layer NLS::Map::Layers[5];

void NLS::Map::Load(const string& id, const string& portal) {
	nextmap = id;
	nextportal = portal;
}

void NLS::Map::Load() {
	C("INFO") << "Loading map " << nextmap << endl;
	char zone;
	if (nextmap == "login") {
		//Uh....
		throw(273);
	} else {
		nextmap.insert(0, 9-nextmap.size(), '0');
		zone = nextmap[0];
		node = WZ::Top["Map"]["Map"][string("Map")+zone][nextmap];
	}
	string bgm = node["info"]["bgm"];
	C("INFO") << "Background music: " << bgm << endl;
	Foothold::Load(node);
}

void NLS::Map::Draw() {
	for (uint8_t i = 0; i < 8; i++) {
		Layers[i].Draw();
	}
}

void NLS::Map::Layer::Draw() {
	for (auto it = Tiles.begin(); it != Tiles.end(); it++) {

	}
}