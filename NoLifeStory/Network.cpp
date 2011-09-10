////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

sf::TcpSocket NLS::Network::Socket;

void NLS::Network::Init() {
	if (Socket.Connect("localhost", 8484, 3) != sf::Socket::Done) {
		C("ERROR") << "Failed to connect to server at 141.157.161.118:8484" << endl;
	} else {
		C("INFO") << "Connected to LoginServer at 141.157.161.118:8484" << endl;
	}
}