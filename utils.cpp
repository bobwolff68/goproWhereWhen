//
// Set of utility functions around file/directory and pruning for bulk/batch work
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//

#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <vector>

#include <sys/stat.h>
#include <dirent.h>

const char* addToPath(const char* start, const char* add) {
	std::string path(start);

	if (path.back() != '/')
		path += '/';

	path += add;

	return path.c_str();
}

void getAllFilesFromPath(const char* inPath, bool bRecurse, std::vector<std::string>& files) {
  	std::vector<std::string> dirs;
	DIR *dir;
	struct dirent *dirItem;

	files.clear();
	dirs.clear();
  	dirs.push_back(inPath);

  	while (!dirs.empty()) {
//  		cout << "TOP: dirs.size()=" << dirs.size() << endl;
  		std::string curPath = dirs.back();
  		dirs.pop_back();
//  		std::cout << "Processing dir: " << curPath << std::endl;

  		dir = opendir(curPath.c_str());
  		// dir stream is now open and ready for business.
  		if (!dir) {
  			std::cerr << "ERROR: Directory could not be opened: " << curPath << std::endl;
  			exit(-1);
  		}

  		dirItem = readdir(dir);
  		while (dirItem) {
  			// Will receive all file and dir entries including . and ..

  			if (!(!strcmp(dirItem->d_name, ".") || !strcmp(dirItem->d_name, "..")))
  			{
	  			if (dirItem->d_type == DT_REG) {
//		  			std::cout << "FILE: " << dirItem->d_name << std::endl;
		  			files.push_back(addToPath(curPath.c_str(), dirItem->d_name));
	  			}
	  			// Only add directories in if we're in recursive mode.
	  			else if (bRecurse && dirItem->d_type == DT_DIR) {
//		  			std::cout << " DIR: " << dirItem->d_name << std::endl;
		  			dirs.push_back(addToPath(curPath.c_str(), dirItem->d_name));
	  			}
	  		}

	  		// Next entry please.
  			dirItem = readdir(dir);
  		}

  		// Done with this directory. Close it.
		closedir(dir);
//  		cout << "BOTTOM: dirs.size()=" << dirs.size() << endl;

  	}
}

void tailLowerCase(std::string source, unsigned int take, std::string &dest) {
	std::string mine(source);

	// tolower the whole source string.
	std::transform(mine.begin(), mine.end(), mine.begin(),
    [](unsigned char c){ return std::tolower(c); });

	if (take < mine.size()) {
		dest = mine.substr(mine.size() - take);
	}
	else 
		dest = mine;
}

//
// Validation rules:
// - Comma separated
// - No * allowed
// - No . allowed
// - No spaces (shouldn't be possible due to getopt parsing)
// - extension should not exceed 5 characters in length (3 or 4 or 5 is reasonable)
// - destList should contain at least one entry on exit
//
// Output is all lowercase
// Output is a vector of strings for iterating through
//
bool validateFileExts(const char* endingList, std::vector<std::string> &destList) {
	std::string input(endingList);

	// tolower the whole source string.
	std::transform(input.begin(), input.end(), input.begin(),
    [](unsigned char c){ return std::tolower(c); });

	destList.clear();

	// Invalid to have . or * in the list
	// List should be comma separated file endings only.
	// For instance: mp4,mpeg,mov,mpx
	if (input.find_first_of("*. ") != std::string::npos)
		return false;

	size_t start=0;
	size_t end=0;
	do {
		end = input.find(",",start);
//		cout << "start=" << start << ", end=" << end << endl;

		if (end==start) {
			// repeating ',' situation ... advance start pointer
			start++;
			continue;
		}

		if (end==std::string::npos) // reached the end
		{
			// Grab the last entry
			destList.push_back(input.substr(start));
			break;
		}
		else
		{
			// Add this 'chunk' from start to end
			destList.push_back(input.substr(start, end-start));
			start = end+1;
			// Special case if someone puts a ',' at the end.
			if (start>=input.size())
				break;
		}

		if (destList.back().length() > 5 || destList.back().length() == 0)
			return false;

	} while(start < input.size());

	// Catch the 'break' cases and check the last entry for length validation.
	// Also check for the case where there's nothing in destList
	// Lastly check for an empty entry where start==end above.
	if (destList.empty() || destList.back().length() > 5 || destList.back().length() == 0)
		return false;

	return true;
}

void unittest_ExtValidation() {
  std::string test[11];
  test[0] = "MP4,MOV,MPEG";
  test[1] = "MP4";
  test[2] = "mov,MP4,";
  test[3] = "abc,,def";
  test[4] = ",,,mpeg";
  // Everything else should fail
  test[5] = "ABC,*.mp3";
  test[6] = "";
  test[7] = ",";
  test[8] = ",,";
  test[9] = "abc,.123";
  test[10] = "abc,longext,not";

  for (auto it: test) {
  	std::vector<std::string> out;
  	std::cout << "Checking: '" << it << "' : ";

  	if (!validateFileExts(it.c_str(), out)) {
  		std::cout << "FAILED." << std::endl;
  	}
  	else {
  		std::cout << "PASSED. Extensions follow: ";
  		for (auto j: out)
  			std::cout << "'" << j << "' ";
  		std::cout << std::endl;
  	}
  }
}

void pruneFilesList(std::vector<std::string> &filesInOut, std::vector<std::string> toKeep) {
	bool bMatch;

	for (std::vector<std::string>::iterator it = filesInOut.begin() ; it != filesInOut.end(); ) {
//  	for (auto it: filesInOut) {
		// Iterate through the files and remove any that dont match
		// must be a match for one of 'toKeep'

		bMatch = false;
//		cout << "Checking on file: " << *it << " -- Against: ";
		for (auto ext: toKeep) {
			std::string toMatch, currentEnding;
			toMatch = "." + ext;

//			cout << "'" << toMatch << "' ";

			if (it->length() >= toMatch.length()) {
				// Good to go for comparing
				tailLowerCase(*it, toMatch.length(), currentEnding);

				// Now the check
				if (toMatch == currentEnding) {
					bMatch = true;
//					cout << "MATCH";
					break;		// Not critical as the flag dictates further logic.
				}
			}

		}

		if (!bMatch) {
//			cout << "No Match. PRUNE OUT.";
			filesInOut.erase(it);
			// Now do NOT iterate because we've deleted this entry.
		}
		else
			it++;	// Iterate as we didn't modify the list

		bMatch = false;
//		cout << endl;
	}
}

void unittest_PruneFiles() {
	std::vector<std::string> files;
	std::vector<std::string> extsList;

	extsList.push_back("mp4");
	extsList.push_back("mov");
	extsList.push_back("tmp");

	files.push_back("1keep.mp4");
	files.push_back("2keep.MP4");
	files.push_back("3keep.MOV");
	files.push_back("4prune.xls");
	files.push_back("5keep.mp4");
	files.push_back("6prune");
	files.push_back(".7prunealso");
	files.push_back("8pr");
	files.push_back(".9p");

	std::cout << "BEFORE: ";
	for (auto f: files)
		std::cout << "'" << f << "' ";
	std::cout << std::endl;
	pruneFilesList(files, extsList);
	std::cout << "AFTER: ";
	for (auto f: files)
		std::cout << "'" << f << "' ";
	std::cout << std::endl;
}

