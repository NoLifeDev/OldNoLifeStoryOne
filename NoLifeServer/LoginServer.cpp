////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::LoginServer::LoginServer() {
	done = false;
	listener.SetBlocking(false);
	if (listener.Listen(8484) == sf::Socket::Error) {
		cerr << "ERROR: LoginServer failed to listen on port 8484" << endl;
		throw(273);
	}
	cout << "INFO: LoginServer up and listening on port 8484" << endl;
	//Create world servers
	thread = new sf::Thread([&](){this->Loop();});
	thread->Launch();
}

void NLS::LoginServer::Loop() {
	while (true) {
		static sf::TcpSocket* next(new sf::TcpSocket());
		if (listener.Accept(*next) == sf::Socket::Done) {
			cout << "INFO: Player connected from " << next->GetRemoteAddress() << ":" << next->GetRemotePort() << endl;
			//Do stuff with next
			next = new sf::TcpSocket();
		}
		sf::Sleep(0.1);
		if (done) {
			//Shutdown world serverz
			cout << "INFO: Shutting down LoginServer" << endl;
			//Clean up stuff
			break;
		}
	}
}