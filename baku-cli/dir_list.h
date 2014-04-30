#include <windows.h>
#include <string>
#include <iostream>
#include <list>

using namespace std;

// This recursively gets all the filenames that match the filter, and adds
// them to the std list passed in
void GetFileListing(std::list<std::string>& listing, std::string directory, std::string fileFilter, bool recursively = true)
{
	// If we are going to recurse over all the subdirectories, first of all
	// get all the files that are in this directory that match the filter
	if (recursively)
		GetFileListing(listing, directory, fileFilter, false);

	directory += "\\";

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	// Setup the filter according to whether we are getting the directories
	// or just the files
	std::string filter = directory + (recursively ? "*" : fileFilter);

	// Find the first file in the directory.
	hFind = FindFirstFile(filter.c_str(), &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		if (dwError != ERROR_FILE_NOT_FOUND)
		{
			//nothing
		}
	}
	else
	{
		// Add the first file found to the list
		if (!recursively)
		{
			listing.push_back(directory + std::string(FindFileData.cFileName));
		}

		// List all the other files in the directory.
		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			if (!recursively)
			{
				listing.push_back(directory + std::string(FindFileData.cFileName));
			}
			else
			{
				// If we found a directory then recurse into it
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)>0 && FindFileData.cFileName[0] != '.')
				{
					GetFileListing(listing, directory + std::string(FindFileData.cFileName), fileFilter);
				}
			}
		}

		DWORD dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES)
		{
			std::cout << "FindNextFile error. Error is " << dwError << std::endl;
		}
	}
}