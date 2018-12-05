// jpgArwCopier.cpp : Defines the entry point for the console application

#include "stdafx.h"
#include "myApp.h"

// --------------------------------------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
	myApp app;


	app.setConsoleHandler();


	if( app.getError().empty() )
	{
		app.parse_args(argc, argv);
	}


	if( app.getError().empty() )
	{
		if( app.check_disk_and_files() )
		{
			if( app.check_arw_jpg_Dirs() )
			{
				cout << endl;
				cout << green << " Ready to Proceed!" << white << endl;
				cout << "   Press '" << White << "Ctrl+C" << white << "' anytime you want to " << White << "Stop" << white << endl;
				cout << "   Hit '" << White << "Enter" << white << "' to " << White << "Begin" << white << " OR input any other text to " << White << "Exit" << white << ": " << flush;

				app.Copy();
			}
		}
	}


	if( !app.getError().empty() )
	{
		cout << red << "\n Error: " << app.getError() << white << endl;
	}
	else
	{
		if( app.isEnterPressed() && app.AppRunning() )
			cout << green << " Done. Press '" << White << "Enter" << green << "' to Exit. " << white;
		else
			cout << "\n Exiting...\n";
	}

	if( app.AppRunning() )
		getchar();

	return 0;
}
