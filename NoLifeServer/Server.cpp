///////////////////////////////////////////////
// This file is part of NoLifeStory.         //
// Please see Global.h for more information. //
///////////////////////////////////////////////
#include "Global.h"

NLS::LoginServer* loginServer;

void NLS::Init(vector<string> args) {
	loginServer = new LoginServer();
	//for each world
	//do the same with a WorldServer
	//plus for each channel in that world
	//do the same with a ChannelServer
}

void NLS::Loop() {
	auto GetCommand = []()->vector<string>{
		string command;
		char cstr[1024];
		cin.getline(cstr,1024);
		command = cstr;
		istringstream iss(command);
		vector<string> words;
		copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string>>(words));
		return words;
	};
	while (true) {
		auto command = GetCommand();
		if (command.size() == 0) {
			continue;
		}
		if (command[0] == "help" or command[0] == "?") {
			if (command.size() > 1) {
				if (command[1] == "shutdown") {
					cout << "HELP: Shuts down NoLifeServer gracefully, preserving all data" << endl;
				}
				if (command[1] == "forceshutdown") {
					cout << "HELP: Forces NoLifeServer to shutdown immediately, losing any temporary data" << endl;
				}
				if (command[1] == "help" or command[1] == "?") {
					cout << "HELP: Displays information on commands available and what each command does" << endl;
				}
			} else {
				cout << "HELP: Available commands: shutdown, forceshutdown" << endl;
				cout << "HELP: Enter \"help command\" for help with a particular command" << endl;
			}
		}
		if (command[0] == "shutdown") {
			cout << "INFO: Initiating NoLifeServer shutdown" << endl;
			break;
		}
		if (command[0] == "forceshutdown") {
			cout << "INFO: Forcing NoLifeServer shutdown" << endl;
			system("pause");
			exit(0);
		}
	}
	return;
}

void NLS::Unload() {
	loginServer->done = true;
	loginServer->thread->Wait();
	delete loginServer;
	cout << "INFO: NoLifeServer shutdown complete" << endl;
	system("pause");
}