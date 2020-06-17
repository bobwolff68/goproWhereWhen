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
#include <vector>

#include "opts.h"

using namespace std;

opts options;

// Utils
extern	void getAllFilesFromPath(const char* inPath, bool bRecurse, vector<string>& files);
extern	bool validateFileExts(const char* endingList, vector<string> &destList);
extern	void pruneFilesList(vector<string> &filesInOut, vector<string> toKeep);

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

