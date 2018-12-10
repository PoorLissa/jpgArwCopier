#pragma once

#include "stdafx.h"

using namespace std;

// ================================================================================================

inline ostream& green	(ostream &);
inline ostream& white	(ostream &);
inline ostream& White	(ostream &);
inline ostream& yellow	(ostream &);
inline ostream& red		(ostream &);

// ================================================================================================

#define myApp_Typedefs \
	typedef LARGE_INTEGER l_int;

// ================================================================================================

// Cout helper structure
struct ok_fail
{
	ok_fail(bool val) : data(val) {};
	bool data;
};
// ================================================================================================

// Main application class
class myApp {

	myApp_Typedefs;

	public:

		myApp();
	   ~myApp();

		string	getFromDir	()	{ return _dirFrom;	}
		string	getToDir	()	{ return _dirTo;	}
		string	getError	()	{ return _error;	}
		bool	AppRunning	()	{ return isRunning;	}
static	HANDLE	getConsole	()	{ return console;	}

		void initDebugMode			();
		void setError				(const char *);
		void setConsoleHandler		();
		void parse_args				(int, _TCHAR*[]);
		void Copy					();
		bool check_disk_and_files	();
		bool check_arw_jpg_Dirs		();																	// Check if arw and jpg dirs exist / create them if they are not
		bool isEnterPressed			();
		int	 consoleCursorVisible	(bool);

	private:

static	BOOL WINAPI		consoleHandler		(DWORD);
static	void			returnCaret			(const int);
static	DWORD CALLBACK	CopyProgressRoutine	(l_int, l_int, l_int, l_int, DWORD, DWORD, HANDLE, HANDLE, LPVOID);
		void			CLS					();
		void			setDirs				(wstring, wstring);
		ULONGLONG		getFiles			();															// get 2 vectors with file names and the total size of all files
		ULONGLONG		getFreeSpace		();
		string			copy				(const char *, const string &, const vector<string> &);
		wstring			getWStr				(string);
		string			getStr				(wstring);
		void			doWait				();
		bool			dirExists			(string);
		bool			fileExists			(wstring);
		ULONGLONG		findFiles			(string, vector<string> &, char *);
		string			getNewName			(const string &) const;

	private:

		char	ch;
		bool	isDebug;
static	bool	isRunning;
static	BOOL	cancelCopy;
static	HANDLE	console;

		vector<string> vecArw;
		vector<string> vecJpg;

		string _dirFrom, _dirTo, _jpgDir, _arwDir, _error;

		unsigned int maxLen, createDirsLen;
};
// ================================================================================================
