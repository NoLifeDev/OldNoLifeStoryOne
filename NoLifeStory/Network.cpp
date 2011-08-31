////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

sf::TcpSocket NLS::Network::Socket;

void NLS::Network::Init() {
	if (Socket.Connect("141.157.161.118", 8484) != sf::Socket::Done) {
		cerr << "WARNING: Failed to connect to server" << endl;
	} else {
		cout << "INFO: Connected to LoginServer at 141.157.161.118:8484" << endl;
	}
}