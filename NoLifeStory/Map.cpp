////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::Node NLS::Map::node;
string NLS::Map::nextmap;
string NLS::Map::nextportal;
vector<NLS::Back*> NLS::Map::Backgrounds;
NLS::Map::Layer NLS::Map::Layers[8];
vector<NLS::Back*> NLS::Map::Foregrounds;

void NLS::Map::Load(const string& id, const string& portal) {
	nextmap = id;
	nextportal = portal;
}

void NLS::Map::Load() {
	char zone;
	if (nextmap == "login") {
		//Uh....
		throw(273);
	} else {
		if (nextmap.size() < 9) {
			nextmap.insert(0, 9-nextmap.size(), '0');
		}
		zone = nextmap[0];
		node = WZ::Top["Map"]["Map"][string("Map")+zone][nextmap];
	}
	if (!node) {
		C("ERROR") << "Unable to locate map " << nextmap << endl;
		nextmap = "";
		nextportal = "";
		return;
	}
	C("INFO") << "Loading map " << nextmap << endl;
	string bgm = node["info"]["bgm"];
	auto p = bgm.find('/');
	Node s = WZ::Top["Sound"][bgm.substr(0, p)][bgm.substr(p+1)];
	sf::Sound snd(*s.data->sound);
	snd.Play();
	for (uint8_t i = 0; i < 8; i++) {
		Layers[i].Tiles.clear();
		Layers[i].Objs.clear();
	}
	Backgrounds.clear();
	Foregrounds.clear();
	Foothold::Load(node);
	Tile::Load(node);
	Obj::Load(node);
	Back::Load(node);
	View.tx = 0;
	View.ty = 0;
	View.vx = 0;
	View.vy = 0;
	nextmap = "";
	nextportal = "";
}

void NLS::Map::Draw() {
	for (uint32_t i = 0; i < Backgrounds.size(); i++) {
		Backgrounds[i]->Draw();
	}
	for (uint8_t i = 0; i < 8; i++) {
		Layers[i].Draw();
	}
	for (uint32_t i = 0; i < Foregrounds.size(); i++) {
		Foregrounds[i]->Draw();
	}
}

void NLS::Map::Layer::Draw() {
	for (auto it = Objs.begin(); it != Objs.end(); it++) {
		(*it)->Draw();
	}
	for (auto it = Tiles.begin(); it != Tiles.end(); it++) {
		(*it)->Draw();
	}
}