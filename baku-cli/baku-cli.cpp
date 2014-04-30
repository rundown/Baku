// baku-cli.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <ios>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "sha_file.h"
#include "dir_list.h"
#include "support.h"
#include "defines.h"
#include "sqlite/sqlite3.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	/* First get all variables
	   There must be more than 3 variables,
		idx[1] -> first is the source directory
		idx[2] -> second is the destination
		idx[3....x] -> all the following variables are filetypes
	 */

	// check if there are atleast 3 variables
	if (argc < 4)
	{
		std::cout << "Baku" << endl << "No nonsense backup toolkit" << endl << "By <gaurav.joseph@linuxmail.org>" << endl
			<< "Powered by SQLite database <sqlite.org>" << endl
			<< "Uses components from OpenSSL Project <openssl.org>" << endl
			<< "Uses 256 bit Secured Hashing Algorithm" << endl << endl
			<< "Usage:" << std::endl
			<< argv[0] << " <sourcedir> <destdir> <extension1> [<extension2> [<extension3>[...]]";
		return BAKU_EXIT_ARGUMENT_ERROR;
	}

	unsigned char hash[SHA256_DIGEST_LENGTH];

	std::string source_dir = argv[1];
	std::string dest_dir = argv[2];

	// check if source and destinations exist
	std::cout << "Checking directories ..... ";
	if (!(path_exists(source_dir) && path_exists(dest_dir))) return BAKU_EXIT_PATH_ERROR;

	std::cout << "OK" << endl << "Opening Database ..... ";

	sqlite3 *litedb;
	char *db_error;

	std::string queried_hash;

	// open the database file in the dest directory or create new if necessary
	int result = sqlite3_open((dest_dir + "\\baku.db3").c_str(), &litedb);
	if (result)
	{
		// error
		sqlite3_close(litedb);
		return BAKU_EXIT_DB_ERROR;
	}
	std::cout << "OK" << endl << "Creating DB conditional ..... ";
	// attempt to create table
	result = sqlite3_exec(litedb, SQL_CREATE_TABLE.c_str(), NULL, NULL, &db_error);
	if (result != SQLITE_OK)
	{
		std::cerr << sqlite3_errmsg(litedb);
		sqlite3_free(db_error);
		sqlite3_close(litedb);
		return BAKU_EXIT_DB_ERROR;
	}
	std::cout << "OK" << endl << "Starting to recurse the source directory ..." << endl;
	// get all the files recursing through the extensions
	for (int i = 3; i < argc; i++){
		// list all the files and their hashes
		std::list<std::string> single_listing;
		std::cout << "Recursing for extension " << std::string(argv[i]) << endl;
		GetFileListing(single_listing, source_dir, "*." + std::string(argv[i]));
		for (std::list<std::string>::iterator it = single_listing.begin(); it != single_listing.end(); ++it)
		{
			// std::cout << (*it).c_str() << std::endl;
			// complete_listing.push_back(*it);
			if (sha1_file((*it).c_str(), hash)){
				std::stringstream hash_s;
				for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
				{
					hash_s << hex << setw(2) << setfill('0') << (int)hash[i];
				}
				// get the folder struct minus the root
				std::string unrooted_path = (*it).substr(source_dir.length() + 1);
				std::cout << unrooted_path << " -> " << hash_s.str() << endl;
				// search for file in the database
				std::string query = "SELECT _hash FROM files WHERE _idef='";
				query += get_unique_filename(unrooted_path) + "'";
				char **results = NULL;
				int rows, columns;
				result = sqlite3_get_table(litedb, query.c_str(), &results, &rows, &columns, &db_error);
				if (result != SQLITE_OK)
				{
					std::cerr << sqlite3_errmsg(litedb);
					sqlite3_free(db_error);
					sqlite3_close(litedb);
					return BAKU_EXIT_DB_ERROR;
				}
				std::string current_dest_filepath = dest_dir + "\\" + unrooted_path;
				std::string current_source_filepath = source_dir + "\\" + unrooted_path;
				if (rows == 0)
				{
					sqlite3_free_table(results);
					// the file doesn't exist just copy the file and insert the hash
					// the folder may not exist, recursively generate the folder
					cout << "Delta > 0 (new entry)" << endl;
					std::string path_to_generate = current_dest_filepath.substr(0, current_dest_filepath.find_last_of("\\"));
					std::cout << "Generate dirs: " << path_to_generate << endl;
					createdirs_recursive(path_to_generate);
					if (!CopyFile(current_source_filepath.c_str(), current_dest_filepath.c_str(), TRUE))
						continue;
					query = "INSERT INTO files VALUES ('" + get_unique_filename(unrooted_path) + "', '" + hash_s.str() + "')";
					result = sqlite3_exec(litedb, query.c_str(), NULL, NULL, &db_error);
					if (result != SQLITE_OK)
					{
						std::cerr << sqlite3_errmsg(litedb);
						sqlite3_free(db_error);
						sqlite3_close(litedb);
						return BAKU_EXIT_DB_ERROR;
					}
				}
				else
				{
					queried_hash = results[1];
					sqlite3_free_table(results);
					// check if hash is the same
					if (queried_hash != hash_s.str())
					{
						std::cout << "Delta has changed (" << queried_hash << ") -> (" << hash_s.str() << ")" << endl;
						// if file exists remove the old file and copy over the new one and replace the hash
						if (path_exists(current_dest_filepath, false))
						{
							if (!DeleteFile(current_dest_filepath.c_str()))
								continue;
						}
						if (!CopyFile(current_source_filepath.c_str(), current_dest_filepath.c_str(), TRUE))
							continue;
						query = "UPDATE files SET _hash='" + hash_s.str() + "' WHERE _idef='" + get_unique_filename(unrooted_path) + "'";
						result = sqlite3_exec(litedb, SQL_CREATE_TABLE.c_str(), NULL, NULL, &db_error);
						if (result != SQLITE_OK)
						{
							std::cerr << sqlite3_errmsg(litedb);
							sqlite3_free(db_error);
							sqlite3_close(litedb);
							return BAKU_EXIT_DB_ERROR;
						}
					}
				}
			}
		}
	}
	std::cout << "Backup Complete ..." << endl;
	return BAKU_EXIT_SUCCESS;
}


/*
unsigned char hash[SHA256_DIGEST_LENGTH];
sha1_file("C:\\Users\\Peregrin Took\\Downloads\\md5-fast-64.S", hash);
std::stringstream ss;
for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
{
ss << hex << setw(2) << setfill('0') << (int)hash[i];
}
std::cout << ss.str();
*/