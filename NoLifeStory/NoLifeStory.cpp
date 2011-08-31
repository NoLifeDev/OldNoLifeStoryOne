////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

int main(int argc, char **argv) {//Not much to do here
#if defined(NLS_WINDOWS) && defined(_CONSOLE)
	SetConsoleTitleA("NoLifeStory::Console");
	HANDLE hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_NOLIFESTORY_ICON));
	::SendMessageW(GetConsoleWindow(), WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
#endif
	cout << "INFO: Loading NoLifeStory" << endl;
	vector<string> strs(argv, argv+argc);
	//Actually run NLS here
	NLS::Init(strs);
	while (NLS::Loop()) {}
	NLS::Unload();
	return 0;
}