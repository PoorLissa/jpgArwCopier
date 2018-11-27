// jpgArwCopier.cpp : Defines the entry point for the console application

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <iomanip>


using namespace std;


class myApp {

	public:

		myApp() 
		{
			_dirFrom.clear();
			_dirTo.clear();
			_error.clear();
		}

		void setDirs(wstring from, wstring to)
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

		// get 2 vectors with file names and the total size of all files
		ULONGLONG getFiles()
		{
			ULONGLONG size(0);
			
			size += findFiles(_dirFrom, vecArw, "*.arw");
			size += findFiles(_dirFrom, vecJpg, "*.jpg");

			return size;
		}

		void Copy()
		{
			string str("");

			str += copy("arw", _arwDir, vecArw);
			str += copy("jpg", _jpgDir, vecJpg);

			cout << str << endl;

			return;
		}

		ULONGLONG getFreeSpace()
		{
			ULONGLONG size = 0;

			ULARGE_INTEGER lrg1, lrg2, lrg3;

			if( GetDiskFreeSpaceExA(_dirTo.c_str(), &lrg1, &lrg2, &lrg3) )
			{
				size = lrg1.QuadPart;
			}

			return size;
		}

		// Check if arw and jpg dirs exist / create them if they are not
		bool check_arw_jpg_Dirs()
		{
			bool res1(true), res2(true);
#ifndef _DEBUG
			if( !dirExists( _arwDir ) )
			{
				cout << " arw Dir does not exist. Creating - ";
				res1 = CreateDirectoryA(_arwDir.c_str(), NULL);
				cout << (res1 ? "OK" : "Fail") << endl;
			}

			if( !dirExists( _jpgDir ) )
			{
				cout << " jpg Dir does not exist. Creating - ";
				res2 = CreateDirectoryA(_jpgDir.c_str(), NULL);
				cout << (res2 ? "OK" : "Fail") << endl;
			}
#endif
			return res1 && res2;
		}

		string getFromDir()	{ return _dirFrom;	}
		string getToDir()	{ return _dirTo;	}
		string getError()	{ return _error;	}

	private:

		static void returnCaret(const int num)
		{ 
			COORD pos;
			static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_SCREEN_BUFFER_INFO info;
			GetConsoleScreenBufferInfo(console, &info);

			pos.X = info.dwCursorPosition.X - num;
			pos.Y = info.dwCursorPosition.Y;
			SetConsoleCursorPosition(console, pos);
		}

		static DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
		{
			int progress = 100 * TotalBytesTransferred.QuadPart / TotalFileSize.QuadPart;

			returnCaret(4);

			cout << std::right << std::setw(3) << progress << "%";

			return PROGRESS_CONTINUE;
		}

		string copy(const char *str, const string &dir, const vector<string> &vec)
		{
			string res(" ["); 
				   res += str;

			cout << endl;
			cout << " Copying '" << str << "' files to " << dir << " ..." << endl;

			int cnt(0), offset(1), Size(vec.size()), N(Size);

			while( N > 0 )
			{
				offset++;
				N /= 10;
			}

			for(size_t i = 0; i < Size; i++)
			{

				string old_name = vec[i];
				string new_name = old_name;

				wstring old_file = getWStr(_dirFrom + old_name);
				wstring new_file = getWStr( dir     + new_name);

				cout << "  [" << std::right << std::setw(offset) << i << "/" << Size << " ]: '" << old_name << "' ==> '" << new_name << "' - [ ..0% ]";

				#ifndef _DEBUG

					if( fileExists((new_file)) )
					{
						cout << " - Fail (File exists already, skipping)";
					}
					else
					{
						DWORD dwCopyFlags = COPY_FILE_FAIL_IF_EXISTS;

						returnCaret(2);

						bool success = success = CopyFileExW(old_file.c_str(), new_file.c_str(), CopyProgressRoutine, NULL, false, dwCopyFlags);

						cout << " ] - " << (success ? "OK" : "Fail");

						if( success )
							cnt++;
					}

					cout << endl;

				#else

					cout << " - OK (Debug)" << endl;
					cnt++;

				#endif
			}

			cout << endl;

			#ifndef _DEBUG
				res += "] Copied " + to_string(cnt) + " / " + to_string(Size) + " files\n";
			#else
				res += "] Found " + to_string(Size) + " files. Files were NOT copied (Program is running in DEBUG mode)\n";
			#endif

			return res;
		}

		wstring getWStr(string s)
		{
			wstring wS;
			
			for(size_t i = 0; i < s.length(); i++)
				wS += s[i];

			return wS;
		}

		string getStr(wstring ws)
		{
			string S;
			
			for(size_t i = 0; i < ws.length(); i++)
				S += static_cast<char>(ws[i]);

			return S;
		}

		bool dirExists(string s)
		{
			wstring wS = getWStr(s);

			DWORD dwAttrib = GetFileAttributes(wS.c_str());

			return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		};

		bool fileExists(wstring s)
		{
			DWORD dwAttrib = GetFileAttributes(s.c_str());

			return (dwAttrib != INVALID_FILE_ATTRIBUTES);
		};

		ULONGLONG findFiles(string dir, vector<string> &vec, char *mask)
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
						int i = 0;
						string name(""), fullName(dir);

						while( ffd.cFileName[i] != '\0' )
						{
							char ch = static_cast<char>(ffd.cFileName[i]);

							name.push_back(ch);
							fullName.push_back(ch);
							i++;
						}

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

	private:

		vector<string> vecArw;
		vector<string> vecJpg;

		string _dirFrom, _dirTo, _jpgDir, _arwDir, _error;
};

// ------------------------------------------------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
	myApp app;


	// set paths from the argument
	if( argc < 2 )
	{
		cout << "\n  Too few parameters. Press 'Enter' to Exit: ";
		getchar();
		return 1;
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

		app.setDirs(from, to);
	}

	char ch;

	if( app.getError().empty() )
	{
		bool diskOk(false), filesOk(false);

		ULONGLONG freeSpace = app.getFreeSpace();
		ULONGLONG filesSize = app.getFiles();

		if( freeSpace > 0 )
			diskOk = true;

		if( filesSize > 0 && filesSize < freeSpace )
			filesOk = true;

		freeSpace *= 10;
		filesSize *= 1;

		ULONGLONG max = freeSpace > filesSize ? freeSpace : filesSize;

		int offset1 = 0, offset2 = 4;
		unsigned int tmp = max/1024.0/1024/1024;

		while( max > 0 )
		{
			max /= 10;
			offset1++;
		}

		while( tmp > 0 )
		{
			tmp /= 10;
			offset2++;
		}


		tmp = (freeSpace/1024.0/1024/1024) * 1000; cout << " Disk space available : " << std::setw(offset1) << freeSpace << " bytes (" << std::fixed << std::setprecision(3) << std::setw(offset2) << tmp/1000.0 << " Gb) - " << (diskOk  ? "OK" : "Fail" ) << endl;
		tmp = (filesSize/1024.0/1024/1024) * 1000; cout << " Total files size     : " << std::setw(offset1) << filesSize << " bytes (" << std::fixed << std::setprecision(3) << std::setw(offset2) << tmp/1000.0 << " Gb) - " << (filesOk ? "OK" : "Fail" ) << endl;


		if( diskOk && filesOk )
		{
			if( app.check_arw_jpg_Dirs() )
			{
				cout << "\n Ready to proceed. Hit 'Enter' to begin OR input any other text to Exit: ";

				ch = getchar();

				if( int(ch) == 10 )
				{
					app.Copy();
				}
			}

			cout << ((int(ch) == 10) ? " Done. Press 'Enter' to Exit. " : "\n Exiting...\n");
		}
	}
	else
	{
		cout << "Error: " << app.getError() << endl;
	}

	getchar();

	return 0;
}
