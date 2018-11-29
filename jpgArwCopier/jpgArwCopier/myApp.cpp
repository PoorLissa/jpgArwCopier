#include "stdafx.h"
#include "myApp.h"

// ================================================================================================

// Static members initialization
bool	myApp::isRunning  = false;
BOOL	myApp::cancelCopy = FALSE;
HANDLE	myApp::console	  = nullptr;

// ================================================================================================

extern ostream& green	(ostream &s) { SetConsoleTextAttribute(myApp::getConsole(), FOREGROUND_GREEN|FOREGROUND_INTENSITY);									return s; }
extern ostream& white	(ostream &s) { SetConsoleTextAttribute(myApp::getConsole(), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);						return s; }
extern ostream& yellow	(ostream &s) { SetConsoleTextAttribute(myApp::getConsole(), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);					return s; }
extern ostream& red		(ostream &s) { SetConsoleTextAttribute(myApp::getConsole(), FOREGROUND_RED|FOREGROUND_INTENSITY);									return s; }
extern ostream& white2	(ostream &s) { SetConsoleTextAttribute(myApp::getConsole(), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);	return s; }

template <class _Elem, class _Traits>
basic_ostream<_Elem,_Traits>& 
	operator << (basic_ostream<_Elem,_Traits>& stream, ok_fail& val)
{
    return (val.data ? stream << green << "Ok" : stream << red << "Fail") << white;
}

// ================================================================================================

myApp::myApp()
{
	initDebugMode();

   _dirFrom.clear();
   _dirTo.clear();
   _error.clear();

	console = GetStdHandle(STD_OUTPUT_HANDLE);

	if( console == INVALID_HANDLE_VALUE )
	{
	   _error = "Could not retrieve the CONSOLE Handle";
	}

	isRunning  = true;
	cancelCopy = FALSE;
}
// ------------------------------------------------------------------------------------------------

myApp::~myApp()
{
	isRunning = false;
	console = nullptr;
}
// ------------------------------------------------------------------------------------------------

void myApp::setError(const char *err)
{
	_error = err;
}
// ------------------------------------------------------------------------------------------------

void myApp::setConsoleHandler()
{
	if( !SetConsoleCtrlHandler(&myApp::consoleHandler, TRUE) )
		_error = "Could not set Control Handler";
}
// ------------------------------------------------------------------------------------------------

void myApp::parse_args(int argc, _TCHAR* argv[])
{
	// set paths from the argument
	if( argc < 2 )
	{
		setError("Too few parameters. Press 'Enter' to Exit: ");
	}
	else
	{
		wstring from, to;

		for(int i = 1; i < argc; i++)
		{
			wstring param(argv[i]);

			if( param.substr(0, 6) == L"/from=" )
				from = param;

			if( param.substr(0, 4) == L"/to=" )
				to = param;
		}

		setDirs(from, to);
	}

	return;
}
// ------------------------------------------------------------------------------------------------

void myApp::Copy()
{
	string str("\n");

	str += copy("arw", _arwDir, vecArw);
	str += copy("jpg", _jpgDir, vecJpg);

	cout << str << endl;

	return;
}
// ------------------------------------------------------------------------------------------------

bool myApp::check_disk_and_files()
{
	auto getOffset = [](ULONGLONG val, int offset)
	{
		int cnt = 0;

		while( val > 0 )
		{
			val /= 10;
			offset++;
				cnt++;

			// take thousands separators into account
			if( cnt > 2 )
			{
				cnt = 0;
				offset++;
			}
		}

		return offset;
	};

	auto getSizeGb = [](ULONGLONG val)
	{
		return ((val/1024.0/1024/1024) * 1000) / 1000.0;
	};

	bool diskOk(false), filesOk(false);

	ULONGLONG freeSpace = getFreeSpace();
	ULONGLONG filesSize = getFiles();

	if( freeSpace > 0 )
		diskOk = true;
	else
	   _error = "Zero free space on the disk!";

	if( filesSize > 0 && filesSize < freeSpace )
		filesOk = true;
	else
		if( filesSize > freeSpace )
		   _error = "Not enough free space to copy the files!";

	ULONGLONG max = freeSpace > filesSize ? freeSpace : filesSize;
	ULONGLONG tmp = static_cast<ULONGLONG>(max/1024.0/1024/1024);

	int offset1 = getOffset(max, 0);
	int offset2 = getOffset(tmp, 4);

	cout << " Disk space available : " << setw(offset1) << freeSpace << " bytes (" << fixed << setprecision(3) << setw(offset2) << getSizeGb(freeSpace) << " Gb) - " << ok_fail( diskOk) << endl;
	cout << " Total files size     : " << setw(offset1) << filesSize << " bytes (" << fixed << setprecision(3) << setw(offset2) << getSizeGb(filesSize) << " Gb) - " << ok_fail(filesOk) << endl;

	// Calculate createDirsLen as a sum of 2 offsets and the length 0f the text above
	createDirsLen = 39 + offset1 + offset2;

	return diskOk && filesOk;
}
// ------------------------------------------------------------------------------------------------

// Check if arw and jpg dirs exist / create them if they are not
bool myApp::check_arw_jpg_Dirs()
{
	bool res1(true), res2(true);

	auto createDir = [](const char *name, const string &dir, bool &res, int len)
	{
		cout << " '" << name << "' Dir does not exist. Creating " << flush;
 
		res = CreateDirectoryA(dir.c_str(), NULL);

		Sleep(222);

		for(int i = 0; i < len; i++)
		{
			Sleep(25);
			cout << "." << flush;
		}

		cout << " " << ok_fail(res) << endl;

		Sleep(333);
	};

	if( !isDebug )
	{
		bool newLine = true;

		// reduce createDirsLen by the length of the text above
		createDirsLen -= 37;

		if( !dirExists( _arwDir ) )
		{
			cout << endl;
			createDir("arw", _arwDir, res1, createDirsLen);
			newLine = false;
		}

		if( !dirExists( _jpgDir ) )
		{
			if( newLine )
				cout << endl;

			createDir("jpg", _jpgDir, res2, createDirsLen);
		}
	}

	return res1 && res2;
}
// ------------------------------------------------------------------------------------------------

// Catches 'Ctrl+C' event (breaks the operation)
BOOL WINAPI myApp::consoleHandler(DWORD signal)
{
	if( signal == CTRL_C_EVENT )
	{
		cancelCopy = TRUE;
		isRunning  = false;
	}

	return TRUE;
}
// ------------------------------------------------------------------------------------------------

void myApp::returnCaret(const int num)
{ 
	COORD pos;

	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(console, &info);

	pos.X = info.dwCursorPosition.X - num;
	pos.Y = info.dwCursorPosition.Y;
	SetConsoleCursorPosition(console, pos);

	return;
}
// ------------------------------------------------------------------------------------------------

DWORD CALLBACK myApp::CopyProgressRoutine(l_int TotalFileSize, l_int TotalBytesTransferred, l_int StreamSize, l_int StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
	int progress = static_cast<int>(100 * TotalBytesTransferred.QuadPart / TotalFileSize.QuadPart);

	returnCaret(4);

	cout << std::right << std::setw(3) << progress << "%";

	return PROGRESS_CONTINUE;
}
// ------------------------------------------------------------------------------------------------

void myApp::setDirs(wstring from, wstring to)
{
	auto setPath = [](wstring str, string &realPath)
	{
		string path(""), res("");
		int pos = -1;

		if( str.substr(0, 6) == L"/from=" )
			pos = 6;

		if( str.substr(0, 4) == L"/to=" )
			pos = 4;

		str = str.substr(pos, str.length());

		for(unsigned int i = 0; i < str.length(); i++)
		{
			if( str[i] != '"' )
				path += static_cast<char>(str[i]);
		}

		if( path.back() != '\\' )
			path += '\\';

		realPath = path;
	};

	setPath(from, _dirFrom);
	setPath(to,   _dirTo  );

	_jpgDir = _dirTo + "jpg\\";
	_arwDir = _dirTo + "arw\\";

	cout << endl;
	cout << "   [from] : '" << _dirFrom << "'" << endl;
	cout << "     [to] : '" << _dirTo	<< "'" << endl;
	cout << " [arwDir] : '" << _arwDir  << "'" << endl;
	cout << " [jpgDir] : '" << _jpgDir  << "'" << endl;
	cout << endl;

	if( !dirExists( _dirFrom ) || !dirExists( _dirTo ) )
	{
		_error = "Wrong directory [to]/[from]";
	}

	return;
}
// ------------------------------------------------------------------------------------------------

// get 2 vectors with file names and the total size of all files
ULONGLONG myApp::getFiles()
{
	ULONGLONG size(0);

	maxLen = 0;
			
	size += findFiles(_dirFrom, vecArw, "*.arw");
	size += findFiles(_dirFrom, vecJpg, "*.jpg");

	size_t offset = 0, max = vecArw.size() > vecJpg.size() ? vecArw.size() : vecJpg.size();

	while( max > 0 )
	{
		max /= 10;
		offset++;
	}

	cout << white2;
	cout << " Found " << setw(offset) << vecArw.size() << " .arw files" << endl;
	cout << " Found " << setw(offset) << vecJpg.size() << " .jpg files" << endl;
	cout << white << endl;

	return size;
}
// ------------------------------------------------------------------------------------------------

ULONGLONG myApp::getFreeSpace()
{
	ULONGLONG size = 0;

	ULARGE_INTEGER lrg1, lrg2, lrg3;

	if( GetDiskFreeSpaceExA(_dirTo.c_str(), &lrg1, &lrg2, &lrg3) )
	{
		size = lrg1.QuadPart;
	}

	return size;
}
// ------------------------------------------------------------------------------------------------

string myApp::copy(const char *str, const string &dir, const vector<string> &vec)
{
	string res(""); 

	if( isRunning )
	{
		cancelCopy = FALSE;

		res += " [";
		res += str;

		cout << endl;
		cout << " Copying '" << str << "' files to " << dir << " ..." << endl;

		cout << string(dir.length() + 30, '-') << std::endl;

		size_t cnt(0), offset(1), Size(vec.size()), N(Size);

		while( N > 0 )
		{
			offset++;
			N /= 10;
		}

		for(size_t i = 0; i < Size; i++)
		{
			if( !isRunning )
				break;

			string old_name = vec[i];
			string new_name = getNewName(old_name);

			wstring old_file = getWStr(_dirFrom + old_name);
			wstring new_file = getWStr( dir     + new_name);

			cout << "  [" << right << setw(offset) << i+1 << "/" << Size << " ]: " 
								   << setw(maxLen) << white2 << old_name << white << " ==> " 
								   << setw(maxLen) << white2 << new_name << white << " - [ ..0% ]";

			if( isDebug )
			{
				cout << " - " << ok_fail(true) << " (Debug Mode)" << endl;
				cnt++;
			}
			else
			{
				if( fileExists((new_file)) )
				{
					cout << yellow << " - Skipped" << white;
				}
				else
				{
					DWORD dwCopyFlags = COPY_FILE_FAIL_IF_EXISTS;

					returnCaret(2);

					// If the user presses 'ctrl+C' while copying, cancelCopy becomes 'TRUE', operation is interrupted and currently copied file is deleted
					bool success = CopyFileExW(old_file.c_str(), new_file.c_str(), CopyProgressRoutine, NULL, &cancelCopy, dwCopyFlags);

					cout << " ] - " << ok_fail(success);
					
					if( success )
						cnt++;
					else
						if( cancelCopy )
							cout << yellow << " [Interrupted by User]" << white;
				}

				cout << endl;
			}
		}

		if( isDebug )
			res += "] Found " + to_string(Size) + " files. Files were NOT copied (Program is running in DEBUG mode)\n";
		else
			res += "] Copied " + to_string(cnt) + " / " + to_string(Size) + " files\n";
	}

	return res;
}
// ------------------------------------------------------------------------------------------------

wstring myApp::getWStr(string s)
{
	wstring wS;
			
	for(size_t i = 0; i < s.length(); i++)
		wS += s[i];

	return wS;
}
// ------------------------------------------------------------------------------------------------

string myApp::getStr(wstring ws)
{
	string S;
			
	for(size_t i = 0; i < ws.length(); i++)
		S += static_cast<char>(ws[i]);

	return S;
}
// ------------------------------------------------------------------------------------------------

bool myApp::dirExists(string s)
{
	wstring wS = getWStr(s);

	DWORD dwAttrib = GetFileAttributes(wS.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
};
// ------------------------------------------------------------------------------------------------

bool myApp::fileExists(wstring s)
{
	DWORD dwAttrib = GetFileAttributes(s.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
};
// ------------------------------------------------------------------------------------------------

// Get list of [mask] files from the directory. Return total size of all files found
ULONGLONG myApp::findFiles(string dir, vector<string> &vec, char *mask)
{
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	bool error(false);
	ULONGLONG size(0);
	vec.clear();
	string Dir = dir + mask;
	struct _stat32 buf;

	for(size_t i = 0; i < Dir.length(); i++)
	{
		szDir[i+0] = Dir[i];
		szDir[i+1] = '\0';
	}

	hFind = FindFirstFile(szDir, &ffd);

	if( hFind != INVALID_HANDLE_VALUE )
	{
		do {

			if( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				size_t i = 0;
				string name(""), fullName(dir);

				while( ffd.cFileName[i] != '\0' )
				{
					char ch = static_cast<char>(ffd.cFileName[i]);

					name.push_back(ch);
					fullName.push_back(ch);
					i++;
				}


				// Remember the longest file name length within this bunch
				maxLen = (i > maxLen) ? i : maxLen;

				// Remember the name
				vec.push_back(name);


				// Get data associated with "crt_stat.c": 
				if( _stat32(fullName.c_str(), &buf ) )
				{
					error = true;
				}
				else
				{
					size += buf.st_size;
				}
			}
		}
		while( FindNextFile(hFind, &ffd) );

		FindClose(hFind);
	}

	if( !error )
		sort(vec.begin(), vec.end());
	else
		size = 0;

	return size;
}
// ------------------------------------------------------------------------------------------------

string myApp::getNewName(const string &oldName) const
{
	string	name("");
	char	border(0);
	int		diff('Z' - 'z');

	for(size_t i = 0; i < oldName.length(); i++)
	{
		char ch = oldName[i];

		if( ch >= 'A' && ch <= 'Z' )
			ch = ch - diff;

		if( border == 0 && i > 0 )
		{
			if( ch >= '0' && ch <= '9' )
			{
				char prev = oldName[i-1];

				if( prev < '0' || prev > '9' )
				{
					if( prev != '_' )
					{
						border = 1;
					}
				}
			}
		}

		if( border == 1 )
		{
			name.push_back('_');
			border = 2;
		}

		if( !( i == 0 && ch == '_') )
			name.push_back(ch);
	}
	
	return name;
}
// ------------------------------------------------------------------------------------------------

void myApp::initDebugMode()
{
	#ifndef _DEBUG
		isDebug = false;
	#else
		isDebug = true;
	#endif

	return;
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
