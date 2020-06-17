//
// System for extracting GoPro GPMETA data from MP4 files
//
// Target is GPS lat, lon, and elevation as well as GPSU UTC time information
//
// System allows for a variety of sampling rates as well as batch file processing
// flexibility.
//
// Export of data can be had as CSV file(s) or GPX files to be imported elsewhere
//
// Initial focus in 2020 was post-processing video taken on motorcycle adventure
// trips to know where I was when certain video files were taken to enable looking
// back at a bit of video and being able to pinpoint a location "post-ride"
//
// Author: Robert M. Wolff - bob dawt wolff 68 aht gmail.com
//

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <set>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <dirent.h>

#include "getopt/getopt.hpp"
#include "opts.h"

using namespace std;

opts options;

const char* addToPath(const char* start, const char* add) {
	string path(start);

	if (path.back() != '/')
		path += '/';

	path += add;

	return path.c_str();
}

void getAllFilesFromPath(const char* inPath, bool bRecurse, vector<string>& files) {
  	vector<string> dirs;
	DIR *dir;
	struct dirent *dirItem;

	files.clear();
	dirs.clear();
  	dirs.push_back(inPath);

  	while (!dirs.empty()) {
//  		cout << "TOP: dirs.size()=" << dirs.size() << endl;
  		string curPath = dirs.back();
  		dirs.pop_back();
  		cout << "Processing dir: " << curPath << endl;

  		dir = opendir(curPath.c_str());
  		// dir stream is now open and ready for business.
  		if (!dir) {
  			cout << "ERROR: Directory could not be opened: " << curPath << endl;
  			exit(-1);
  		}

  		dirItem = readdir(dir);
  		while (dirItem) {
  			// Will receive all file and dir entries including . and ..

  			if (!(!strcmp(dirItem->d_name, ".") || !strcmp(dirItem->d_name, "..")))
  			{
	  			if (dirItem->d_type == DT_REG) {
		  			cout << "FILE: " << dirItem->d_name << endl;
		  			files.push_back(addToPath(curPath.c_str(), dirItem->d_name));
	  			}
	  			// Only add directories in if we're in recursive mode.
	  			else if (bRecurse && dirItem->d_type == DT_DIR) {
		  			cout << " DIR: " << dirItem->d_name << endl;
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

void tailLowerCase(string source, unsigned int take, string &dest) {
	string mine(source);

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
bool validateFileExts(const char* endingList, vector<string> &destList) {
	string input(endingList);

	// tolower the whole source string.
	std::transform(input.begin(), input.end(), input.begin(),
    [](unsigned char c){ return std::tolower(c); });

	destList.clear();

	// Invalid to have . or * in the list
	// List should be comma separated file endings only.
	// For instance: mp4,mpeg,mov,mpx
	if (input.find_first_of("*. ") != string::npos)
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

		if (end==string::npos) // reached the end
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
  string test[11];
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
  	vector<string> out;
  	cout << "Checking: '" << it << "' : ";

  	if (!validateFileExts(it.c_str(), out)) {
  		cout << "FAILED." << endl;
  	}
  	else {
  		cout << "PASSED. Extensions follow: ";
  		for (auto j: out)
  			cout << "'" << j << "' ";
  		cout << endl;
  	}
  }
}

void pruneFilesList(vector<string> &filesInOut, vector<string> toKeep) {
	bool bMatch;

	for (std::vector<string>::iterator it = filesInOut.begin() ; it != filesInOut.end(); ) {
//  	for (auto it: filesInOut) {
		// Iterate through the files and remove any that dont match
		// must be a match for one of 'toKeep'

		bMatch = false;
//		cout << "Checking on file: " << *it << " -- Against: ";
		for (auto ext: toKeep) {
			string toMatch, currentEnding;
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
	vector<string> files;
	vector<string> extsList;

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

	cout << "BEFORE: ";
	for (auto f: files)
		cout << "'" << f << "' ";
	cout << endl;
	pruneFilesList(files, extsList);
	cout << "AFTER: ";
	for (auto f: files)
		cout << "'" << f << "' ";
	cout << endl;
}

int main(int argc, const char** argv)
{
  vector<string> files;
  options.processOpts(argc, argv);

// unittest_ExtValidation();
// unittest_PruneFiles();
//exit(1);

  if (options.fileExtRaw != "") {
  	// Need to validate the input and place it into the options vector
  	if (!validateFileExts(options.fileExtRaw.c_str(), options.fileExtList)) {
  		// Invalid.
  		cout << "ERROR: Option '--fileext' expects to get a comma separated list of" << endl;
  		cout << "  file extensions. No '*' or '.' - just command separated extensions." << endl;
  		cout << "  Upper/Lower case does not matter. Extension comparison will be insensitive." << endl;
  		cout << "  Example: --fileext=MP4,MOV,MPEG" << endl;
  		exit(-2);
  	}

  	// Now we have a good extensions list.
  	// These will be applied to 'sourcedir' when searching.
  }

  if (options.sourceDir != "") {
  	getAllFilesFromPath(options.sourceDir.c_str(), options.sourceDirRecursive, files);

  	// Now let's apply the fileext results to this list to prune it down to our work items.
  	pruneFilesList(files, options.fileExtList);
  }

  return 0;
}

