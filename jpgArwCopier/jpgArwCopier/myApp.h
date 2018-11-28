#pragma once

#include "stdafx.h"

using namespace std;

// ================================================================================================

inline ostream& aaa1	(ostream &);
inline ostream& green	(ostream &);
inline ostream& white	(ostream &);
inline ostream& yellow	(ostream &);
inline ostream& red		(ostream &);
inline ostream& white2	(ostream &);

// ================================================================================================

struct ok_fail 
{
	ok_fail(bool val) : data(val) {};
	bool data;
};

class myApp {

	public:

		myApp();
	   ~myApp();

		string	getFromDir	()	{ return _dirFrom;	}
		string	getToDir	()	{ return _dirTo;	}
		string	getError	()	{ return _error;	}
		bool	AppRunning	()	{ return isRunning;	}
static	HANDLE	getConsole	()	{ return console;	}

		void setError				(const char *);
		void setConsoleHandler		();
		void parse_args				(int, _TCHAR*[]);
		void Copy					();
		bool check_disk_and_files	();
		bool check_arw_jpg_Dirs		();																				// Check if arw and jpg dirs exist / create them if they are not

	private:

typedef LARGE_INTEGER l_int;

static	BOOL WINAPI		consoleHandler		(DWORD);
static	void			returnCaret			(const int);
static	DWORD CALLBACK	CopyProgressRoutine	(l_int, l_int, l_int, l_int, DWORD, DWORD, HANDLE, HANDLE, LPVOID);
		void			setDirs				(wstring, wstring);
		ULONGLONG		getFiles			();																		// get 2 vectors with file names and the total size of all files
		ULONGLONG		getFreeSpace		();
		string			copy				(const char *, const string &, const vector<string> &);
		wstring			getWStr				(string);
		string			getStr				(wstring);
		bool			dirExists			(string);
		bool			fileExists			(wstring);
		ULONGLONG		findFiles			(string, vector<string> &, char *);
		string			getNewName			(const string &) const;

	private:

static	HANDLE console;

		bool isDebug;
static	bool isRunning;
static	BOOL cancelCopy;

		vector<string> vecArw;
		vector<string> vecJpg;

		string _dirFrom, _dirTo, _jpgDir, _arwDir, _error;

		unsigned int maxLen;
};
