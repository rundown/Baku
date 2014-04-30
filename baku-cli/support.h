#include <Windows.h>
#include <string>
#include <iostream>

using namespace std;

bool createdirs_recursive(const std::string &path)
{
	static const std::string separators("\\");

	// get the postion of the first separator
	std::size_t slashIndex = path.find_first_of(separators);
	// start from the second slash, first slash indicates the root

	do
	{
		std::string path_part;
		slashIndex = path.find_first_of(separators, slashIndex + 1);
		path_part = path.substr(0, slashIndex);
		// If the specified directory name doesn't exist, do our thing
		DWORD fileAttributes = ::GetFileAttributes(path_part.c_str());
		if ((fileAttributes == INVALID_FILE_ATTRIBUTES))
		{
			BOOL result = ::CreateDirectory(path_part.c_str(), nullptr);
			if (result == FALSE) return false;
		}
	} while (slashIndex != std::string::npos);

	return true;
}

bool path_exists(const std::string &path, bool checkForFolder = true)
{
	WIN32_FIND_DATA data;
	HANDLE hFile = FindFirstFile(path.c_str(), &data);

	if (hFile == INVALID_HANDLE_VALUE) // directory doesn't exist
		return false;
	FindClose(hFile);
	if (checkForFolder)
	{
		// is it folder or file?
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return true;
		return false;
	}
	return true;
}

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

std::string get_unique_filename(const std::string path)
{
	std::string replaceable = path;
	ReplaceStringInPlace(replaceable, "\\", "-");
	ReplaceStringInPlace(replaceable, "'", "-");
	ReplaceStringInPlace(replaceable, "\"", "-");
	ReplaceStringInPlace(replaceable, "=", "-");
	ReplaceStringInPlace(replaceable, ";", "-");
	ReplaceStringInPlace(replaceable, "`", "-");
	ReplaceStringInPlace(replaceable, ".", "-");
	ReplaceStringInPlace(replaceable, "%", "-");
	ReplaceStringInPlace(replaceable, "?", "-");
	ReplaceStringInPlace(replaceable, "*", "-");
	return replaceable;
}


/*
std::wstring s2ws(const std::string& s)
{
int len;
int slength = (int)s.length() + 1;
len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
wchar_t* buf = new wchar_t[len];
MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
std::wstring r(buf);
delete[] buf;
return r;
}

std::wstring stemp = s2ws(myString);
LPCWSTR result = stemp.c_str();
*/
