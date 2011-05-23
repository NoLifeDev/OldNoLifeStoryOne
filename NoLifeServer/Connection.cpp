///////////////////////////////////////////////
// This file is part of NoLifeStory.         //
// Please see Global.h for more information. //
///////////////////////////////////////////////
#include "Global.h"

vector<NLS::Connection*> NLS::Connections;

NLS::Connection::Connection(sf::TcpSocket* socket) {
	this->socket = socket;
	Connections.push_back(this);
}