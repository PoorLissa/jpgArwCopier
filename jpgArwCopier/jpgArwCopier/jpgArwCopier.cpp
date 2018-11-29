// jpgArwCopier.cpp : Defines the entry point for the console application

#include "stdafx.h"
#include "myApp.h"

// --------------------------------------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
	cout.imbue( std::locale("en") );

	char  ch = 0;
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
				cout << "   Press 'Ctrl+C' anytime you want to Stop (Incomplete files will be deleted)" << endl;
				cout << "   Hit 'Enter' to Begin OR input any other text to Exit: " << flush;

				ch = getchar();

				if( int(ch) == 10 )
				{
					app.Copy();
				}
			}
		}
	}


	if( !app.getError().empty() )
	{
		cout << red << "\n Error: " << app.getError() << white << endl;
	}
	else
	{
		if( int(ch) == 10 )
			cout << green << " Done. Press 'Enter' to Exit. " << white;
		else
			cout << "\n Exiting...\n";
	}

	if( app.AppRunning() )
		getchar();

	return 0;
}
